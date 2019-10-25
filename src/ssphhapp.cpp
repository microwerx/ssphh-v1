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

#include <memory>
#include <ssphhapp.hpp>
#include <hatchetfish_stopwatch.hpp>

namespace Fluxions
{
	extern bool debugging;
} // namespace Fluxions

std::shared_ptr<SSPHH::SSPHH_Application> ssphhPtr;
extern std::vector<std::string> g_args;

#ifdef USE_GLUT
extern void PrintBitmapStringJustified(float x, float y, int justification, void* font, const char* format, ...);
extern void PrintString9x15(float x, float y, int justification, const char* format, ...);
#endif

extern double gt_Fps;
extern double g_distance;

using namespace Fluxions;

void InitSSPHH() {
	ssphhPtr = std::make_shared<SSPHH::SSPHH_Application>();
}

void KillSSPHH() {
	//ssphh->Stop();
	ssphhPtr->Kill();
}

namespace SSPHH
{
	using namespace Fluxions;
	using namespace Vf;

	SSPHH_Application::SSPHH_Application()
		: Widget("ssphhapplication"), PBSkyCubeMap(GL_TEXTURE_CUBE_MAP) {
		sceneFilename = "resources/scenes/test_texture_scene/test_gallery3_scene.scn";
	}

	SSPHH_Application::SSPHH_Application(const std::string& name)
		: Widget(name), PBSkyCubeMap(GL_TEXTURE_CUBE_MAP) {
		sceneFilename = "resources/scenes/test_texture_scene/test_gallery3_scene.scn";
	}

	SSPHH_Application::~SSPHH_Application() {
		if (ssgUserData) {
			delete ssgUserData;
		}
	}

	void SSPHH_Application::ResetScene() {
		ssg.environment.pbsky.SetCivilDateTime(ssg.environment.pbsky_dtg);
		ssg.environment.pbsky.ComputeSunFromLocale();
		pbsky_localtime = ssg.environment.pbsky.GetTime();
		pbsky_timeOffsetInSeconds = 0.0;
		Interface.recomputeSky = true;
		AdvanceSunClock(0.0, true);
		RegenHosekWilkieSky();
	}

	void SSPHH_Application::UseCurrentTime() {
		pbsky_localtime = time(NULL);
		pbsky_timeOffsetInSeconds = 0.0;

		ssg.environment.pbsky.SetTime(time(NULL), 0.0);
		ssg.environment.pbsky.ComputeSunFromLocale();
		Interface.recomputeSky = true;
		RegenHosekWilkieSky();
	}


	void SSPHH_Application::LoadScene() {
		if (Interface.uf.uf_type == UfType::Broker) {
			Hf::Log.info("configured to be a broker, so not loading scene");
			return;
		}

		FilePathInfo fpi(sceneFilename);
		Interface.sceneName = fpi.fname;
		ssg.Load(sceneFilename);
		ssg.BuildBuffers();

		gles30CubeMap.SetSceneGraph(ssg);
	}

	void SSPHH_Application::OptimizeClippingPlanes() {
		//Matrix4f cameraMatrix = defaultRenderConfig.preCameraMatrix * ssg.camera.actualViewMatrix * defaultRenderConfig.postCameraMatrix;
		//cameraMatrix.AsInverse().col4()
		Matrix4f cameraMatrix = ssg.camera.actualViewMatrix.AsInverse();
		const BoundingBoxf& bbox = ssg.GetBoundingBox();
		const Matrix4f& frameOfReference = cameraMatrix;
		const Vector3f& position = frameOfReference.col4().xyz();
		float znear;
		float zfar;

		float distanceToBoxCenter = (position - bbox.Center()).length() + 1.0f;
		float boxRadius = bbox.RadiusRounded();
		znear = std::max(0.1f, distanceToBoxCenter - boxRadius);
		zfar = distanceToBoxCenter + 2 * boxRadius; // min(1000.0f, distanceToBoxCenter + boxRadius);

		defaultRenderConfig.znear = znear;
		defaultRenderConfig.zfar = zfar;
		rectShadowRenderConfig.znear = std::max(0.1f, ssg.environment.sunShadowMapNearZ);
		rectShadowRenderConfig.zfar = std::min(1000.0f, ssg.environment.sunShadowMapFarZ);
	}

	void SSPHH_Application::ParseCommandArguments(const std::vector<std::string>& cmdargs) {
		if (cmdargs.size() <= 1)
			return;

		size_t count = cmdargs.size();
		int i = 1;
		for (size_t j = 1; j < count; j++) {
			bool nextArgExists = j < count - 1;
			if ((cmdargs[j] == "-scene") && nextArgExists) {
				// next argument is the path
				FilePathInfo fpi(cmdargs[j + 1]);
				if (fpi.Exists()) {
					sceneFilename = cmdargs[j + 1];
					Hf::Log.info("%s(): loading scene file %s", __FUNCTION__, sceneFilename.c_str());
				}
				else {
					Hf::Log.error("%s(): scene file %s does not exist.", __FUNCTION__, sceneFilename.c_str());
				}
				j++;
			}

			if (cmdargs[j] == "-broker") {
				Interface.uf.uf_type = UfType::Broker;
				Hf::Log.info("Unicornfish: starting in broker mode");
			}
			if (cmdargs[j] == "-worker") {
				Interface.uf.uf_type = UfType::Worker;
				Hf::Log.info("Unicornfish: starting in client mode");
			}
			if (cmdargs[j] == "-client") {
				Interface.uf.uf_type = UfType::Client;
				Hf::Log.info("Unicornfish: starting in worker mode");
			}
			if ((cmdargs[j] == "-endpoint") && nextArgExists) {
				Interface.uf.endpoint = cmdargs[j + 1];
				j++;
				Hf::Log.info("Unicornfish: using endpoint %s", Interface.uf.endpoint.c_str());
			}
			if ((cmdargs[j] == "-service") && nextArgExists) {
				Interface.uf.service = cmdargs[j + 1];
				Hf::Log.info("Unicornfish: using service %s", Interface.uf.service.c_str());
				j++;
			}
		}
	}

	void SSPHH_Application::StartPython() {
		//python.init(string("ssphh"));
		//python.start();
		//python.run();
	}

	int init_count = 0;
	void SSPHH_Application::OnInit(const std::vector<std::string>& args) {
		HFLOGINFO("Initializing SSPHH App");
		init_count++;
		// TODO: I would like to make the following code work:
		//
		// SceneGraph sg;
		// sg.Init();
		// sg.Load("myscene.scn");
		//
		// Renderer r;
		// r.Init();
		// r.LoadRenderConfig("deferred.renderconfig")
		// r.LoadRenderConfig("pbr_monolithic.renderconfig")
		// r.LoadRenderConfig("pbr_deferred.renderconfig")
		// r.BuildBuffers(sg);

		ParseCommandArguments(args);

		InitUnicornfish();
		InitImGui();

		Hf::StopWatch stopwatch;

		// StartPython();

		InitRenderConfigs();
		LoadRenderConfigs();

		FxSetErrorMessage(__FILE__, __LINE__, "inside OnInit()");

		Interface.preCameraMatrix.LoadIdentity();

		const char* glrenderer = (const char*)glGetString(GL_RENDERER);
		const char* glvendor = (const char*)glGetString(GL_VENDOR);
		const char* glversion = (const char*)glGetString(GL_VERSION);

		my_hud_info.glRendererString = glrenderer ? glrenderer : "Unknown Renderer";
		my_hud_info.glVendorString = glvendor ? glvendor : "Unknown Vendor";
		my_hud_info.glVersionString = glversion ? glversion : "Unknown Version";

		if (!enviroCubeTexture3.LoadTextureCoronaCubeMap("export_cubemap.png", true)) {
			Hf::Log.error("%s(): enviroCubeTexture3...Could not load export_cubemap.png", __FUNCTION__);
		}
		else {
			Hf::Log.info("Loaded enviroCubeTexture3...loaded export_cubemap.png");
		}

		vcPbsky = new PbskyViewController(this);
		PBSkyCubeMap.Create();
		PBSkyCubeMap.SetTextureCubeMap(GL_RGB, GL_FLOAT, 64, 64, nullptr, true);
		PBSkyCubeMap.samplerObject.Create();
		PBSkyCubeMap.samplerObject.SetMinFilter(GL_LINEAR);
		PBSkyCubeMap.samplerObject.SetMagFilter(GL_LINEAR);
		PBSkyCubeMap.samplerObject.SetWrapSTR(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		// Initialize default sampler objects
		defaultCubeTextureSampler.Create();
		defaultCubeTextureSampler.SetMagFilter(GL_LINEAR);
		defaultCubeTextureSampler.SetMinFilter(GL_LINEAR);
		defaultCubeTextureSampler.SetWrapSTR(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		default2DTextureSampler.Create();
		default2DTextureSampler.SetMagFilter(GL_LINEAR);
		default2DTextureSampler.SetMinFilter(GL_LINEAR);
		default2DTextureSampler.SetWrapST(GL_REPEAT, GL_REPEAT);
		defaultShadowCubeTextureSampler.Create();
		defaultShadowCubeTextureSampler.SetMagFilter(GL_LINEAR);
		defaultShadowCubeTextureSampler.SetMinFilter(GL_LINEAR);
		//defaultShadowCubeTextureSampler.SetCompareFunction(GL_LESS);
		//defaultShadowCubeTextureSampler.SetCompareMode(GL_COMPARE_REF_TO_TEXTURE);
		defaultShadow2DTextureSampler.Create();
		defaultShadow2DTextureSampler.SetMagFilter(GL_LINEAR);
		defaultShadow2DTextureSampler.SetMinFilter(GL_LINEAR);
		//defaultShadow2DTextureSampler.SetCompareFunction(GL_LESS);
		//defaultShadow2DTextureSampler.SetCompareMode(GL_COMPARE_REF_TO_TEXTURE);

		FxSetErrorMessage(__FILE__, __LINE__, "before loading scene");

		if (ssgUserData) {
			delete ssgUserData;
			ssgUserData = nullptr;
		}
		ssgUserData = new SSG_SSPHHRendererPlugin(&ssg);

		LoadScene();

		FxSetDefaultErrorMessage();

		ResetScene();

		stopwatch.Stop();
		Hf::Log.info("%s(): took %3.2f seconds", __FUNCTION__, stopwatch.GetSecondsElapsed());

		Widget::OnInit(args);
	}

	void SSPHH_Application::OnKill() {
		// TODO: I would like to make the following code work:
		//
		// SceneGraph sg;
		// Renderer r;
		// ...
		// sg.Kill();
		// r.Kill();
		// r.Init();

		renderer.Reset();
		KillUnicornfish();

		if (ssgUserData) {
			ssg.userdata = nullptr;
			delete ssgUserData;
			ssgUserData = nullptr;
		}
		//python.kill();
		//python.join();

		Widget::OnKill();
	}

	void SSPHH_Application::AdvanceSunClock(double numSeconds, bool recomputeSky) {
		pbsky_timeOffsetInSeconds += numSeconds;
		ssg.environment.pbsky.SetTime(pbsky_localtime, (float)pbsky_timeOffsetInSeconds);
		Interface.recomputeSky = true;
	}


	const Matrix4f& SSPHH_Application::GetCameraMatrix() const {
		return Interface.preCameraMatrix;
	}


	void SSPHH_Application::OnReshape(int width, int height) {
		float w = (float)width;
		float h = (float)height;
		screenWidth = w;
		screenHeight = h;
		aspect = w / h;

		renderer2.SetDeferredRect(Recti(0, 0, (int)w, (int)h));

		defaultRenderConfig.projectionMatrix.LoadIdentity();
		defaultRenderConfig.projectionMatrix.Perspective(ssg.camera.fov, ssg.camera.imageAspect, ssg.camera.imageNearZ, ssg.camera.imageFarZ);

		screenOrthoMatrix.LoadIdentity();
		screenOrthoMatrix.Ortho2D(0.0f, screenWidth, screenHeight, 0.0f);

		Hf::Log.setMaxHistory(height / 30);
	}



	void SSPHH_Application::RenderTest1SunShadows() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		int w = rectShadowRenderConfig.viewportRect.w;
		int h = rectShadowRenderConfig.viewportRect.h;
		glClear(GL_DEPTH_BUFFER_BIT);
		rectShadowRenderConfig.viewportRect.x = (GLsizei)(screenWidth - 256);
		rectShadowRenderConfig.viewportRect.y = 0;
		rectShadowRenderConfig.viewportRect.w = 256;
		rectShadowRenderConfig.viewportRect.h = 256;
		rectShadowRenderConfig.clearColorBuffer = false;
		rectShadowRenderConfig.renderToFBO = false;
		ssg.AdvancedRender(rectShadowRenderConfig);
		rectShadowRenderConfig.viewportRect.w = w;
		rectShadowRenderConfig.viewportRect.h = h;
		rectShadowRenderConfig.viewportRect.x = 0;
		rectShadowRenderConfig.viewportRect.y = 0;
	}

	void SSPHH_Application::RenderTest2SphereCubeMap() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		Matrix4f cameraMatrix = Interface.inversePreCameraMatrix * ssg.camera.viewMatrix;
		Vector3f cameraPosition(cameraMatrix.m14, cameraMatrix.m24, cameraMatrix.m34);
		int s = 128;
		SimpleRenderConfiguration& cubeRC = gles30CubeMap.GetRenderConfig();

		cubeRC.clearColorBuffer = false;
		cubeRC.viewportRect.x = 0;
		cubeRC.viewportRect.y = 0;
		cubeRC.preCameraMatrix = Interface.inversePreCameraMatrix;
		cubeRC.postCameraMatrix.LoadIdentity();
		cubeRC.useZOnly = false;
		cubeRC.useMaterials = true;
		cubeRC.viewportRect.w = s;
		cubeRC.viewportRect.h = s;
		cubeRC.postCameraMatrix = ssg.spheres[1].transform;
		cubeRC.useSceneCamera = true;
		cubeRC.isCubeMap = true;
		// gles30CubeMap.Render();
	}

	void SSPHH_Application::RenderTest3EnviroCubeMap() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		SimpleProgramPtr program = renderer2.FindProgram("glut", "UnwrappedCubeMap");
		if (program != nullptr) {
			program->Use();
			GLint tloc = program->GetAttribLocation("aTexCoord");
			GLint vloc = program->GetAttribLocation("aPosition");
			SimpleUniform orthoProjectionMatrix = Matrix4f().Ortho2D(0.0f, screenWidth, 0.0f, screenHeight);
			SimpleUniform identityMatrix = Matrix4f().LoadIdentity();
			program->ApplyUniform("uCubeTexture", SimpleUniform(0));
			program->ApplyUniform("ProjectionMatrix", orthoProjectionMatrix);
			program->ApplyUniform("CameraMatrix", identityMatrix);
			program->ApplyUniform("WorldMatrix", identityMatrix);
			FxBindTextureAndSampler(0, GL_TEXTURE_CUBE_MAP, enviroCubeTexture3.GetTextureId(), ssg.environment.enviroColorMapSamplerId);
			FxDrawGL2UnwrappedCubeMap(0, 0, 256, vloc, tloc);
			FxBindTextureAndSampler(0, GL_TEXTURE_CUBE_MAP, 0, 0);
			glUseProgram(0);
		}
	}

	void SSPHH_Application::RenderSkyBox() {
		static GLfloat size = 500.0f;
		static GLfloat v[] = {
			-size, size, -size,
			size, size, -size,
			size, -size, -size,
			-size, -size, -size,
			size, size, size,
			-size, size, size,
			-size, -size, size,
			size, -size, size };

		static GLfloat texcoords[] = {
			-1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f };

		static GLfloat buffer[] = {
			-size, size, -size, -1.0f, 1.0f, -1.0f,
			size, size, -size, 1.0f, 1.0f, -1.0f,
			size, -size, -size, 1.0f, -1.0f, -1.0f,
			-size, -size, -size, -1.0f, -1.0f, -1.0f,
			size, size, size, 1.0f, 1.0f, 1.0f,
			-size, size, size, -1.0f, 1.0f, 1.0f,
			-size, -size, size, -1.0f, -1.0f, 1.0f,
			size, -size, size, 1.0f, -1.0f, 1.0f };

		static GLushort indices[] = {
			// FACE 0
			7, 4, 1,
			1, 2, 7,
			// FACE 1
			3, 0, 5,
			5, 6, 3,
			// FACE 2
			1, 4, 5,
			5, 0, 1,
			// FACE 3
			7, 2, 3,
			3, 6, 7,
			// FACE 5
			6, 5, 4,
			4, 7, 6,
			// FACE 4
			2, 1, 0,
			0, 3, 2 };

		static GLuint abo = 0;
		static GLuint eabo = 0;
		GLuint vbo = 0;
		GLint vloc = -1;
		GLint tloc = -1;
		GLuint program = 0;
		GLint uCubeTexture = -1;
		GLint uWorldMatrix = -1;
		GLint uCameraMatrix = -1;
		GLint uProjectionMatrix = -1;
		GLint uSunE0 = -1;
		GLint uSunDirTo = -1;
		GLint uGroundE0 = -1;
		GLint uIsHemisphere = -1;
		GLint uToneMapScale = -1;
		GLint uToneMapExposure = -1;
		GLint uToneMapGamma = -1;

		auto p = renderer2.FindProgram("skybox", "skybox");
		if (p) {
			program = p->GetProgram();
			uCubeTexture = p->GetUniformLocation("uCubeTexture");
			uWorldMatrix = p->GetUniformLocation("WorldMatrix");
			uCameraMatrix = p->GetUniformLocation("CameraMatrix");
			uProjectionMatrix = p->GetUniformLocation("ProjectionMatrix");
			uSunE0 = p->GetUniformLocation("SunE0");
			uGroundE0 = p->GetUniformLocation("GroundE0");
			uSunDirTo = p->GetUniformLocation("SunDirTo");
			uIsHemisphere = p->GetUniformLocation("IsHemisphere");
			uToneMapScale = p->GetUniformLocation("ToneMapScale");
			uToneMapExposure = p->GetUniformLocation("ToneMapExposure");
			uToneMapGamma = p->GetUniformLocation("ToneMapGamma");
		}

		if (program == 0)
			return;

		vloc = glGetAttribLocation(program, "aPosition");
		tloc = glGetAttribLocation(program, "aTexCoord");

		if (abo == 0) {
			glGenBuffers(1, &abo);
			glGenBuffers(1, &eabo);
			glBindBuffer(GL_ARRAY_BUFFER, abo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eabo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		}

		glUseProgram(program);
		if (uToneMapScale >= 0)
			glUniform1f(uToneMapScale, 2.5f * powf(2.0f, ssg.environment.toneMapScale));
		if (uToneMapExposure >= 0)
			glUniform1f(uToneMapExposure, 2.5f * powf(2.0f, ssg.environment.toneMapExposure));
		if (uToneMapGamma >= 0)
			glUniform1f(uToneMapGamma, ssg.environment.toneMapGamma);
		if (uCubeTexture >= 0) {
			FxBindTextureAndSampler(ssg.environment.pbskyColorMapUnit, GL_TEXTURE_CUBE_MAP, ssg.environment.pbskyColorMapId, ssg.environment.pbskyColorMapSamplerId);
			glUniform1i(uCubeTexture, ssg.environment.pbskyColorMapUnit);
		}
		if (uProjectionMatrix >= 0) {
			Matrix4f projectionMatrix = ssg.camera.projectionMatrix;
			glUniformMatrix4fv(uProjectionMatrix, 1, GL_FALSE, projectionMatrix.const_ptr());
		}
		if (uCameraMatrix >= 0) {
			Matrix4f viewMatrix = Interface.inversePreCameraMatrix * ssg.camera.viewMatrix;
			viewMatrix.m14 = viewMatrix.m24 = viewMatrix.m34 = viewMatrix.m41 = viewMatrix.m42 = viewMatrix.m43 = 0.0f;
			glUniformMatrix4fv(uCameraMatrix, 1, GL_FALSE, viewMatrix.const_ptr());
		}
		if (uWorldMatrix >= 0) {
			Matrix4f worldMatrix;
			glUniformMatrix4fv(uWorldMatrix, 1, GL_FALSE, worldMatrix.const_ptr());
		}
		if (uSunDirTo >= 0)
			glUniform3fv(uSunDirTo, 1, ssg.environment.curSunDirTo.const_ptr());
		if (uSunE0 >= 0)
			glUniform4fv(uSunE0, 1, ssg.environment.pbsky.GetSunDiskRadiance().const_ptr());
		if (uGroundE0 >= 0)
			glUniform4fv(uGroundE0, 1, ssg.environment.pbsky.GetGroundRadiance().const_ptr());
		if (uIsHemisphere >= 0)
			glUniform1i(uIsHemisphere, ssg.environment.isHemisphere);

		glBindBuffer(GL_ARRAY_BUFFER, abo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eabo);
		glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const void*)0);
		glVertexAttribPointer(tloc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const void*)12);
		if (vloc >= 0)
			glEnableVertexAttribArray(vloc);
		if (tloc >= 0)
			glEnableVertexAttribArray(tloc);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		if (vloc >= 0)
			glDisableVertexAttribArray(vloc);
		if (tloc >= 0)
			glDisableVertexAttribArray(tloc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

	void SSPHH_Application::SaveScreenshot() {
		if (Interface.saveScreenshot) {
			Interface.saveScreenshot = false;
			glFinish();

			Image3ub image((int)screenWidth, (int)screenHeight);

			std::string filename = GetPathTracerSphlRenderName(
				Interface.sceneName,
				Interface.ssphh.enableKs,
				Interface.ssphh.REF_MaxRayDepth,
				Interface.ssphh.REF_PassLimit,
				Interface.ssphh.MaxDegrees);
			filename += ".ppm";
			Interface.ssphh.lastSphlRenderPath = filename;
			glReadPixels(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight, GL_RGB, GL_UNSIGNED_BYTE, (void*)image.getPixels(0)->const_ptr());
			image.savePPMi(filename, 1.0f, 0, 255, 0, true);
		}
	}

	void SSPHH_Application::ProcessScenegraphTasks() {
		if (Interface.ssg.saveScene) {
			Interface.ssg.saveScene = false;
			sceneFilename = "resources/scenes/test_texture_scene/";
			sceneFilename += Interface.ssg.scenename;
			ssg.Save(sceneFilename);
		}

		if (Interface.ssg.resetScene) {
			Interface.ssg.resetScene = false;
			ssg.Reset();
		}

		if (Interface.ssg.createScene) {
			Interface.ssg.createScene = false;
		}

		if (Interface.ssg.loadScene) {
			Interface.ssg.loadScene = false;
			sceneFilename = "resources/scenes/test_texture_scene/";
			sceneFilename += Interface.ssg.scenename;
			ReloadScenegraph();
		}
	}

	std::string SSPHH_Application::GetPathTracerName(const std::string& sceneName, bool ks, int mrd, int pl) {
		std::ostringstream ostr;
		ostr << sceneName;
		ostr << "_" << std::setw(2) << std::setfill('0') << mrd;
		ostr << "_" << std::setw(2) << std::setfill('0') << pl;
		if (ks)
			ostr << "_Ks";
		return ostr.str();
	}

	std::string SSPHH_Application::GetSphlRenderName(const std::string& sceneName, int md) {
		std::ostringstream ostr;
		ostr << sceneName;
		ostr << "_sphlrender_" << std::setw(2) << std::setfill('0') << md;
		return ostr.str();
	}

	std::string SSPHH_Application::GetPathTracerSphlRenderName(const std::string& sceneName, bool ks, int mrd, int pl, int md) {
		std::ostringstream ostr;
		ostr << sceneName;
		ostr << "_" << std::setw(2) << std::setfill('0') << mrd;
		ostr << "_" << std::setw(2) << std::setfill('0') << pl;
		if (ks)
			ostr << "_Ks";
		ostr << "_sphlrender_";
		ostr << std::setw(2) << std::setfill('0') << md;
		return ostr.str();
	}

	std::string SSPHH_Application::GetStatsName(const std::string& sceneName, bool ks, int mrd, int pl, int md) {
		std::ostringstream ostr;
		ostr << sceneName;
		if (ks)
			ostr << "_Ks";
		ostr << "_" << std::setw(2) << std::setfill('0') << mrd;
		ostr << "_" << std::setw(2) << std::setfill('0') << pl;
		ostr << "_" << std::setw(2) << std::setfill('0') << md;
		return ostr.str();
	}
} // namespace SSPHH
