#include <ssphhapp.hpp>
#include <hatchetfish_stopwatch.hpp>

namespace SSPHH
{
	void SSPHH_Application::RegenHosekWilkieSky() {
		ssg.environment.ComputePBSky();
	}

	void SSPHH_Application::SaveHosekWilkieSky() {
		// Save PB Sky PPMs
		HFLOGINFO("Saving pbsky ppm texture maps");

		Hf::StopWatch stopwatch;
		ssg.environment.pbsky.generatedCylMap.savePPMRaw("pbsky_cylmap.ppm");
		ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_0.ppm", 0);
		ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_1.ppm", 1);
		ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_2.ppm", 2);
		ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_3.ppm", 3);
		ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_4.ppm", 4);
		ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_5.ppm", 5);
		ssg.environment.pbsky.generatedCubeMap.saveCubePPM("pbsky_cubemap.ppm");
		stopwatch.Stop();
		HFLOGINFO("Saving pbsky ppm texture maps took %4.2f seconds", stopwatch.GetSecondsElapsed());
	}

	void SSPHH_Application::RegenCoronaSky() {
		Uf::CoronaJob job("ssphh_sky", Uf::CoronaJob::Type::Sky);

		if (Interface.ssphh.enableHQ)
			job.EnableHQ();
		if (Interface.ssphh.enableHDR)
			job.EnableHDR();

		job.Start(coronaScene, ssg);

		vcPbsky->coronaPath = job.GetOutputPath(vcPbsky->coronaLoadEXR);
		LoadCoronaSky(vcPbsky->coronaLoadEXR, vcPbsky->coronaPath);
	}

	void SSPHH_Application::LoadCoronaSky(bool loadEXR, const std::string &path) {
		FilePathInfo fpi(path);
		if (fpi.Exists()) {
			Image4f lightProbe;
			if (loadEXR) {
				lightProbe.loadEXR(fpi.path);
			}
			else {
				lightProbe.loadPPM(fpi.path);
				//lightProbe.scaleColors(1.0f / (2.5f * powf(2.0f, ssg.environment.toneMapExposure)));
				lightProbe.ReverseSRGB().ReverseToneMap(ssg.environment.toneMapExposure);
			}
			lightProbe.convertRectToCubeMap();
			FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, ssg.environment.pbskyColorMapId);
			for (int i = 0; i < 6; i++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, (GLsizei)lightProbe.width(), (GLsizei)lightProbe.height(), 0, GL_RGBA, GL_FLOAT, lightProbe.getImageData(i));
			}
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
		else {
			HFLOGERROR("Could not generate %s", fpi.path.c_str());
		}
	}
}