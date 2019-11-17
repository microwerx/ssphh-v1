// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
// Copyright (C) 2017 Jonathan Metzgar
// All rights reserved.
//
// This program is free software : you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.If not, see <https://www.gnu.org/licenses/>.
//
// For any other type of licensing, please contact me at jmetzgar@outlook.com
#include "pch.hpp"
#ifdef __unix__
#include <unistd.h>
#endif
#include <ssphh_ppmcompare.hpp>

namespace SSPHH
{

	using namespace Fluxions;

	IntensityStat& IntensityStat::operator+=(const Color3f& color) {
		sumColor += color;
		minColor = Color3d::min(color, minColor);
		maxColor = Color3d::max(color.maxrgb(), maxColor);
		rbarColor += lastColor - color;

		sumI += color.Intensity();
		minI = std::min(minI, minColor.Intensity());
		maxI = std::max(maxI, maxColor.Intensity());
		rbarI += (lastColor - color).Intensity();

		lastColor = color;
		count++;

		return *this;
	}

	void IntensityStat::Finalize() {
		double invNumPixels = 1.0 / count;
		xbarColor = invNumPixels * sumColor;
		xbarI = invNumPixels * sumI;
		rbarColor *= invNumPixels;
		rbarI *= invNumPixels;
		lclI = xbarI - 2.66 * rbarI;
		uclI = xbarI + 2.66 * rbarI;
		lclColor = xbarColor - 2.66 * rbarColor;
		uclColor = xbarColor + 2.66 * rbarColor;
	}

	PPMCompare::PPMCompare() {}

	PPMCompare::~PPMCompare() {}

	void PPMCompare::Init(bool hasSpecular, int maxRayDepth, int passCount, int maxDegree) {
		ks = hasSpecular;
		mrd = maxRayDepth;
		pl = passCount;
		md = maxDegree;
	}

	void PPMCompare::SetConversion(ColorSpaceType im1type, ColorSpaceType im2type) {
		imageColorSpaces[0] = im1type;
		imageColorSpaces[1] = im2type;
	}

	void PPMCompare::Compare(Image3f& image1, Image3f& image2) {
		double t0 = Hf::Log.getSecondsElapsed();

		if (image1.width() <= 0 || image1.height() <= 0 || image1.width() > 8192 || image1.height() > 8192 || image1.width() != image2.width() || image1.height() != image2.height()) {
			Hf::Log.errorfn(__FUNCTION__, "image sizes do not match or exceed the dimensions of 8192x8192");
			return;
		}

		std::vector<Image3f*> images = {
			&image1stat.image,
			&image2stat.image,
			&diffstat.image,
			&diffbwstat.image,
			&absdiffstat.image };

		int width = image1.width();
		int height = image1.height();
		for (auto& image : images) {
			image->resize(width, height);
		}

		bcwidth = width / blockSize;
		bcheight = height / blockSize;
		blockstat.image.resize(bcwidth, bcheight);
		blockbwstat.image.resize(bcwidth, bcheight);
		blockcounts.resize(blockstat.image.width() * blockstat.image.height(),
			BlockCountTuple(0.0f, Color3d(0.0), Color3f(65535.0f), Color3f(-65535.0f), 0.0f, 0.0f, 0.0f));
		const int CountIndex = 0;
		const int AverageIndex = 1;
		const int MinIndex = 2;
		const int MaxIndex = 3;
		const int AvgIIndex = 4;
		const int MinIIndex = 5;
		const int MaxIIndex = 6;

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Color3f pixel1 = image1.getPixel(x, y);
				Color3f pixel2 = image2.getPixel(x, y);
				if (imageColorSpaces[0] == ColorSpaceType::SRGB) {
					pixel1 = SRGBtoXYZ(pixel1);
				}
				if (imageColorSpaces[1] == ColorSpaceType::SRGB) {
					pixel2 = SRGBtoXYZ(pixel2);
				}

				Color3f diff = pixel1 - pixel2;
				Color3f absdiff = Color3f(fabs(diff.r), fabs(diff.g), fabs(diff.b));
				Color3f bwpixel = Color3f((float)absdiff.Intensity());

				image1stat.image.setPixel(x, y, XYZtoSRGB(pixel1));
				image2stat.image.setPixel(x, y, XYZtoSRGB(pixel2));
				diffstat.image.setPixel(x, y, XYZtoSRGB(diff));
				diffbwstat.image.setPixel(x, y, XYZtoSRGB(bwpixel));
				absdiffstat.image.setPixel(x, y, XYZtoSRGB(absdiff));

				image1stat += pixel1;
				image2stat += pixel2;
				diffstat += diff;
				absdiffstat += absdiff;

				int i = x / blockSize;
				int j = y / blockSize;
				size_t addr = blockstat.image.addr(i, j);
				std::get<CountIndex>(blockcounts[addr])++;
				std::get<AverageIndex>(blockcounts[addr]) += absdiff;
				std::get<MinIndex>(blockcounts[addr]) = std::min(std::get<MinIndex>(blockcounts[addr]), absdiff);
				std::get<MaxIndex>(blockcounts[addr]) = std::max(std::get<MaxIndex>(blockcounts[addr]), absdiff);
			}
		}
		image1stat.Finalize();
		image2stat.Finalize();
		diffstat.Finalize();
		diffbwstat.Finalize();
		absdiffstat.Finalize();

		for (int j = 0; j < bcheight; j++) {
			for (int i = 0; i < bcwidth; i++) {
				size_t addr = blockstat.image.addr(i, j);
				float count = std::get<CountIndex>(blockcounts[addr]);
				if (count > 0.0) {
					std::get<AverageIndex>(blockcounts[addr]) /= count;
				}
				else {
					std::get<AverageIndex>(blockcounts[addr]) *= 0.0f;
					std::get<MinIndex>(blockcounts[addr]) *= 0.0f;
					std::get<MaxIndex>(blockcounts[addr]) *= 0.0f;
				}
				Color3f color = std::get<AverageIndex>(blockcounts[addr]);
				Color3f bwcolor((float)color.Intensity());
				blockstat.image.setPixel(i, j, color);
				blockbwstat.image.setPixel(i, j, bwcolor);
				blockstat += color;
				blockbwstat += bwcolor;

				bcMaxValue = std::max<double>(bcMaxValue, color.maxrgb());
				bcMaxValueBW = std::max<double>(bcMaxValue, bwcolor.maxrgb());

				Color3f average = std::get<AverageIndex>(blockcounts[addr]);
				Color3f mindiff = std::get<MinIndex>(blockcounts[addr]);
				Color3f maxdiff = std::get<MaxIndex>(blockcounts[addr]);
				float averageI = std::get<AvgIIndex>(blockcounts[addr]) = (float)average.Intensity();
				float mindiffI = std::get<MinIIndex>(blockcounts[addr]) = (float)mindiff.Intensity();
				float maxdiffI = std::get<MaxIIndex>(blockcounts[addr]) = (float)maxdiff.Intensity();
			}
		}
		blockstat.Finalize();
		blockbwstat.Finalize();
		bciMaxValue = (int)(255.99 * bcMaxValue);
		bciMaxValueBW = (int)(255.99 * bcMaxValueBW);

		compareTime = Hf::Log.getSecondsElapsed() - t0;
	}

	void PPMCompare::SaveResults(const std::string& statsName, const std::string& pathtracerName, bool genDiffs, bool ignoreCache) {
		std::string dtg = Hf::Log.makeDTG();
		{
			std::ofstream fout(statsName + "_blocks.csv", ignoreCache ? std::ios::app : std::ios::out);
			if (ignoreCache)
				fout << "name,dtg,ks,mrd,pl,md,j,i,addr,avgI,minI,maxI" << std::endl;
			const int CountIndex = 0;
			const int AverageIndex = 1;
			const int MinIndex = 2;
			const int MaxIndex = 3;
			const int AvgIIndex = 4;
			const int MinIIndex = 5;
			const int MaxIIndex = 6;
			for (unsigned j = 0; j < blockstat.image.height(); j++) {
				for (unsigned i = 0; i < blockstat.image.width(); i++) {
					unsigned addr = blockstat.image.addr(i, j);
					float averageI = std::get<AvgIIndex>(blockcounts[addr]);
					float mindiffI = std::get<MinIIndex>(blockcounts[addr]);
					float maxdiffI = std::get<MaxIIndex>(blockcounts[addr]);
					fout << statsName << ",";
					fout << dtg << ",";
					fout << (ks ? "Ks" : "None") << ", ";
					fout << mrd << ",";
					fout << pl << ",";
					fout << md << ",";
					fout << j << ",";
					fout << i << ",";
					fout << addr << ",";
					fout << std::setprecision(6) << std::fixed << std::setw(10) << averageI << ",";
					fout << std::setprecision(6) << std::fixed << std::setw(10) << mindiffI << ",";
					fout << std::setprecision(6) << std::fixed << std::setw(10) << maxdiffI << std::endl;
				}
			}
			fout.close();
		}

		if (mrd >= 0 && pl >= 0 && md >= 0) {
			std::ofstream fout(statsName + "_stats.csv", ignoreCache ? std::ios::out : std::ios::app);
			if (ignoreCache)
				fout << "name,type,dtg,ks,mrd,pl,md,sumI,xbarI,minI,maxI,rbarI,lclI,uclI" << std::endl;

			std::vector<std::pair<const char*, IntensityStat*>> pstats = {
				{"IMAGE1", &image1stat},
				{"IMAGE2", &image2stat},
				{"DIFFIM", &absdiffstat} };

			for (auto& pstat : pstats) {
				fout << statsName << ",";
				fout << pstat.first << ",";
				fout << dtg << ",";
				fout << (ks ? "Ks" : "None") << ",";
				fout << mrd << ",";
				fout << pl << ",";
				fout << md << ",";
				fout << std::setprecision(6) << std::fixed << std::setw(10) << pstat.second->sumI << ",";
				fout << std::setprecision(6) << std::fixed << std::setw(10) << pstat.second->xbarI << ",";
				fout << std::setprecision(6) << std::fixed << std::setw(10) << pstat.second->minI << ",";
				fout << std::setprecision(6) << std::fixed << std::setw(10) << pstat.second->maxI << ",";
				fout << std::setprecision(6) << std::fixed << std::setw(10) << pstat.second->rbarI << ",";
				fout << std::setprecision(6) << std::fixed << std::setw(10) << pstat.second->lclI << ",";
				fout << std::setprecision(6) << std::fixed << std::setw(10) << pstat.second->uclI << std::endl;
			}
			fout.close();
		}

		if (!genDiffs)
			return;

		std::ostringstream fn2str;
		fn2str << "_sphlrender"
			<< "_" << std::setw(2) << std::setfill('0') << md;
		std::string fn2 = fn2str.str();

		std::map<std::string, std::string> files;
		files["image1"] = pathtracerName + "_pathtracer.ppm";
		files["image2"] = pathtracerName + fn2 + ".ppm";
		files["diff01"] = pathtracerName + fn2 + "_diff01_rgb.ppm";
		files["diff02"] = pathtracerName + fn2 + "_diff02_bw.ppm";
		files["diff03"] = pathtracerName + fn2 + "_diff03_blocks.ppm";
		files["diff04"] = pathtracerName + fn2 + "_diff04_blocksbw.ppm";

		Image3f blockimage = blockstat.image.ScaleImage(blockstat.image.width() * blockSize, blockstat.image.height() * blockSize);
		Image3f blockbwimage = blockbwstat.image.ScaleImage(blockbwstat.image.width() * blockSize, blockbwstat.image.height() * blockSize);

		image1stat.image.savePPMi(files["image1"], 255.0f, 0, 255);
		image2stat.image.savePPMi(files["image2"], 255.0f, 0, 255);
		diffstat.image.savePPMi(files["diff01"], 255.0f, 0, 255);
		absdiffstat.image.savePPMi(files["diff02"], 255.0f, 0, 255);
		blockimage.savePPMi(files["diff03"], 255.0f, 0, bciMaxValue);
		blockbwimage.savePPMi(files["diff04"], 255.0f, 0, bciMaxValueBW);

		// convert to PNG
		for (auto& f : files) {
			std::string png = f.second;
			png.replace(png.end() - 3, png.end(), "png");

			FilePathInfo fpi(png);
			if (fpi.Exists() && !ignoreCache)
				continue;

			std::string cmdline = std::string("magick ") + f.second + " " + png;
			Hf::Log.infofn(__FUNCTION__, "running ", cmdline.c_str());
			system(cmdline.c_str());
#ifdef _WIN32
			DeleteFileA(f.second.c_str());
#elif __unix__
			unlink(f.second.c_str());
#endif
		}
	}

}