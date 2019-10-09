#include <ssphhapp.hpp>

namespace SSPHH
{
	void SSPHH_Application::SetupRenderGLES30() {
		OptimizeClippingPlanes();
		UpdateSPHLs();
		UploadSPHLs();
	}


	void SSPHH_Application::RenderGLES30() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		SetupRenderGLES30();
		RenderGLES30Shadows();
		RenderGLES30Scene();

		RenderGLES30SPHLs();
		RenderGL11Hierarchies();

		if (counter == 1)
			RenderTest1SunShadows();
		if (counter == 2)
			RenderTest2SphereCubeMap();
		if (counter == 3)
			RenderTest3EnviroCubeMap();

		FxSetDefaultErrorMessage();
	}


	void SSPHH_Application::RenderGLES30Scene() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		defaultRenderConfig.clearDepthBuffer = true;
		defaultRenderConfig.clearColorBuffer = false;
		defaultRenderConfig.viewportRect.x = 0;
		defaultRenderConfig.viewportRect.y = 0;
		defaultRenderConfig.viewportRect.w = (size_t)screenWidth;
		defaultRenderConfig.viewportRect.h = (size_t)screenHeight;
		defaultRenderConfig.preCameraMatrix = Interface.inversePreCameraMatrix;
		defaultRenderConfig.postCameraMatrix = Interface.inversePostCameraMatrix;
		defaultRenderConfig.fov = Interface.ssg.cameraFOV;
		defaultRenderConfig.isCubeMap = false;
		defaultRenderConfig.useSceneCamera = true;
		defaultRenderConfig.useZOnly = false;
		defaultRenderConfig.useMaterials = true;
		defaultRenderConfig.shaderDebugChoice = Interface.tools.shaderDebugChoice;
		defaultRenderConfig.shaderDebugLight = Interface.tools.shaderDebugLight;
		defaultRenderConfig.shaderDebugSphl = Interface.tools.shaderDebugSphl;

		while (glGetError());
		ssg.AdvancedRender(defaultRenderConfig);
		ssg.camera.actualViewMatrix = defaultRenderConfig.cameraMatrix;
		while (glGetError()) HFLOGWARN("AdvancedRender() ERROR!");

		if (Interface.drawSkyBox) {
			FxSetErrorMessage(__FILE__, __LINE__, "skybox");
			glEnable(GL_DEPTH_TEST);
			RenderSkyBox();
			glDisable(GL_DEPTH_TEST);
			FxSetErrorMessage("ssphh.cpp", __LINE__, __FUNCTION__);
			while (glGetError()) HFLOGWARN("Draw SkyBox ERROR!");
		}

		glUseProgram(0);
		glDisable(GL_DEPTH_TEST);
	}
}