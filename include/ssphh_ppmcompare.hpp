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
#ifndef SSPHH_CORONA_HPP
#define SSPHH_CORONA_HPP

#include <fluxions.hpp>
#include <fluxions_gte.hpp>
#include <fluxions_gte_matrix_math.hpp>
#include <fluxions_stdcxx.hpp>

namespace SSPHH
{
	using namespace Fluxions;

	struct IntensityStat
	{
		Fluxions::Image3f image;

		double sumI = 0.0;
		double minI = 1e6;
		double maxI = -1e6;

		double varI = 0.0;
		double xbarI = 0.0;
		double rbarI = 0.0;
		double lclI = 0.0;
		double uclI = 0.0;

		Color3d sumColor;
		Color3d minColor = Color3d(1e6);
		Color3d maxColor = Color3d(-1e6);
		Color3d lastColor;

		Color3d xbarColor;
		Color3d rbarColor;
		Color3d lclColor;
		Color3d uclColor;

		int count = 0;

		IntensityStat &operator+=(const Color3f &color);
		void Finalize();

		//inline Color3f min(const Color3f& v1, const Color3f& v2)
		//{
		//    return Color3f::min(v1, v2);
		//}

		//inline Color3f max(const Color3f& v1, const Color3f& v2)
		//{
		//    return Color3f::max(v1, v2);
		//}
	};

	class PPMCompare
	{
	public:
		PPMCompare();
		~PPMCompare();

		enum class ColorSpaceType
		{
			XYZ,
			SRGB,
			P3
		};

		void Init(bool hasSpecular, int maxRayDepth, int passCount, int maxDegree);
		void SetConversion(ColorSpaceType im1type, ColorSpaceType im2type);
		void Compare(Image3f &image1, Image3f &image2);
		void SaveResults(const std::string &statsName, const std::string &pathtracerName, bool genDiffs = false, bool ignoreCache = false);

		// main product variables
		IntensityStat image1stat;
		IntensityStat image2stat;
		IntensityStat diffstat;
		IntensityStat diffbwstat;
		IntensityStat absdiffstat;

		double compareTime = 0.0;

	private:
		// count, avg, min, max, avgI, minI, maxI
		using BlockCountTuple = std::tuple<float, Color3d, Color3f, Color3f, float, float, float>;
		std::vector<BlockCountTuple> blockcounts;
		IntensityStat blockstat;
		IntensityStat blockbwstat;

		//Image3f blockImage;
		//Image3f blockImageBW;
		double bcMaxValue = 0.0;
		double bcMaxValueBW = 0.0;
		int bciMaxValue = 0;
		int bciMaxValueBW = 0;
		int bcwidth = 0;
		int bcheight = 0;

	private:
		bool ks;
		int mrd;
		int pl;
		int md;
		int blockSize = 32;
		ColorSpaceType imageColorSpaces[2] = { ColorSpaceType::SRGB, ColorSpaceType::SRGB };

		inline float reverseGamma(float value)
		{
			const float a = 0.055f;
			return value <= 0.04045f ? (value / 12.92f) : (powf((value + a) / (1.0f + a), 2.4f));
		}

		inline float compandGamma(float value)
		{
			const float a = 0.055f;
			if (value <= 0.0031308f)
				return 12.92f * value;
			else if (value > 0.0031308f)
				return (1 + a) * powf(value, 1.0f / 2.4f) - a;
			return 0.0f;
		}

		inline Color3f SRGBtoXYZ(const Color3f &color)
		{
			Color3f out;
			Vector3f srgb(reverseGamma(color.r), reverseGamma(color.g), reverseGamma(color.b));
			Matrix3f conversionMatrix(
				0.4124f, 0.3576f, 0.1805f,
				0.2126f, 0.7152f, 0.0722f,
				0.0193f, 0.1192f, 0.9505f);
			Vector3f xyz = conversionMatrix * srgb;
			out = xyz;
			return out;
		}

		inline Color3f XYZtoSRGB(const Color3f &color)
		{
			// Step 1: matrix multiplication
			Matrix3f conversionMatrix(
				3.2406f, -1.5372f, -0.4986f,
				-0.9689f, 1.8758f, 0.0415f,
				0.0557f, -0.2040f, 1.0570f);
			Color3f rgb_linear = conversionMatrix * color;
			// Step 2: gamma companding
			Color3f srgb(compandGamma(rgb_linear.r), compandGamma(rgb_linear.g), compandGamma(rgb_linear.b));
			return srgb;
		}

		inline Color3f XYZtoSRGB255(const Color3f &color)
		{
			// Step 1: matrix multiplication
			Matrix3f conversionMatrix(
				3.2406f, -1.5372f, -0.4986f,
				-0.9689f, 1.8758f, 0.0415f,
				0.0557f, -0.2040f, 1.0570f);
			Color3f rgb_linear = conversionMatrix * color;
			// Step 2: gamma companding
			float r = clamp(255.99f * compandGamma(rgb_linear.r), 0.0f, 255.0f);
			float g = clamp(255.99f * compandGamma(rgb_linear.g), 0.0f, 255.0f);
			float b = clamp(255.99f * compandGamma(rgb_linear.b), 0.0f, 255.0f);
			Color3f srgb(r, g, b);
			return srgb;
		}
	};
} // namespace SSPHH
#endif
