#include <ssphhapp.hpp>
#include <hatchetfish_stopwatch.hpp>

namespace SSPHH
{
	void SSPHH_Application::InitRenderConfigs() {
		FxSetErrorMessage(__FILE__, __LINE__, "inside InitRenderConfigs()");

		rectShadowRenderConfig.clearColorBuffer = true;
		rectShadowRenderConfig.clearDepthBuffer = true;
		rectShadowRenderConfig.viewportRect.x = 0;
		rectShadowRenderConfig.viewportRect.y = 0;
		rectShadowRenderConfig.viewportRect.w = Interface.renderconfig.sunShadowMapSize;
		rectShadowRenderConfig.viewportRect.h = Interface.renderconfig.sunShadowMapSize;
		rectShadowRenderConfig.fov = 90.0;
		rectShadowRenderConfig.isCubeMap = false;
		rectShadowRenderConfig.useSceneCamera = false;
		rectShadowRenderConfig.useZOnly = true;
		rectShadowRenderConfig.recomputeProjectionMatrix = false;
		rectShadowRenderConfig.fbo.Delete();
		rectShadowRenderConfig.fbo.SetDimensions(rectShadowRenderConfig.viewportRect.w, rectShadowRenderConfig.viewportRect.h);
		rectShadowRenderConfig.fbo.AddTexture2D(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GL_RGBA8, true);
		rectShadowRenderConfig.fbo.AddTexture2D(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F, false);
		if (!rectShadowRenderConfig.fbo.Make()) {
			Hf::Log.error("%s(): Could not make rect shadow map FBO.", __FUNCTION__);
		}

		cubeShadowRenderConfig.clearColorBuffer = true;
		cubeShadowRenderConfig.clearDepthBuffer = true;
		cubeShadowRenderConfig.viewportRect.x = 0;
		cubeShadowRenderConfig.viewportRect.y = 0;
		cubeShadowRenderConfig.viewportRect.w = 64;
		cubeShadowRenderConfig.viewportRect.h = 64;
		cubeShadowRenderConfig.isCubeMap = true;
		cubeShadowRenderConfig.useSceneCamera = false;
		cubeShadowRenderConfig.useZOnly = true;
		cubeShadowRenderConfig.fov = 90.0;
		cubeShadowRenderConfig.recomputeProjectionMatrix = false;
		cubeShadowRenderConfig.fbo.Delete();

		// new method with the Simple GLES 30 Renderer
		int cubeMapSize = 128;
		defaultRenderConfig.viewportRect.w = cubeMapSize;
		defaultRenderConfig.viewportRect.h = cubeMapSize;
		defaultRenderConfig.postCameraMatrix = ssg.spheres[1].transform;
		defaultRenderConfig.useSceneCamera = false;
		defaultRenderConfig.isCubeMap = true;
		gles30CubeMap.SetRenderConfig(defaultRenderConfig);

		FxSetDefaultErrorMessage();
	}

	void SSPHH_Application::LoadRenderConfigs() {
		FxSetErrorMessage(__FILE__, __LINE__, "inside LoadRenderConfigs()");

		Hf::Log.info("%s(): resetting and loading...", __FUNCTION__);
		renderer2.Reset();

		const char* renderconfig_filename = "resources/config/pb_monolithic_2017.renderconfig";
		if (!renderer2.LoadConfig(renderconfig_filename)) {
			Hf::Log.error("%s(): %s file not found.", __FUNCTION__, renderconfig_filename);
		}

		renderer2.LoadShaders();

		defaultRenderConfig.zShaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultZProgram");
		defaultRenderConfig.shaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultSlowProgram");

		rectShadowRenderConfig.zShaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultZProgram");
		rectShadowRenderConfig.shaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultSlowProgram");

		cubeShadowRenderConfig.zShaderProgram = renderer2.FindProgram("cube_shadow", "cube_shadow");

		gles30CubeMap.GetRenderConfig().shaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultSlowProgram");
		gles30CubeMap.GetRenderConfig().zShaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultZProgram");

		// New four split render config

		foursplitULRenderConfig.shaderProgram = renderer2.FindProgram("foursplit", "upperleft");
		foursplitURRenderConfig.shaderProgram = renderer2.FindProgram("foursplit", "upperright");
		foursplitLLRenderConfig.shaderProgram = renderer2.FindProgram("foutsplit", "lowerleft");
		foursplitLRRenderConfig.shaderProgram = renderer2.FindProgram("foursplit", "lowerright");
		if (!foursplitLLRenderConfig.check()) HFLOGERROR("Four split renderconfig failure! --> Lower left");
		if (!foursplitLRRenderConfig.check()) HFLOGERROR("Four split renderconfig failure! --> Lower right");
		if (!foursplitULRenderConfig.check()) HFLOGERROR("Four split renderconfig failure! --> Upper left");
		if (!foursplitURRenderConfig.check()) HFLOGERROR("Four split renderconfig failure! --> Lower right");

		FxSetDefaultErrorMessage();
	}

	void SSPHH_Application::ReloadRenderConfigs() {
		Hf::StopWatch stopwatch;
		InitRenderConfigs();
		LoadRenderConfigs();
		stopwatch.Stop();
		Hf::Log.info("%s(): reload took %4.2f milliseconds", __FUNCTION__, stopwatch.GetMillisecondsElapsed());
		Interface.lastRenderConfigLoadTime = stopwatch.GetMillisecondsElapsed();
	}
} // namespace SSPHH