#include <ssphhapp.hpp>
#include <fluxions_render_utilities.hpp>

namespace SSPHH
{
	void SSPHH_Application::RenderGLES30Shadows() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		std::map<GLenum, RenderTarget*> rts;

		double sunShadowT0 = Hf::Log.getSecondsElapsed();
		rectShadowRenderConfig.renderSkyBox = false;
		rectShadowRenderConfig.viewportRect.w = Interface.renderconfig.sunShadowMapSize;
		rectShadowRenderConfig.viewportRect.h = Interface.renderconfig.sunShadowMapSize;
		rectShadowRenderConfig.preCameraMatrix.LoadIdentity();
		rectShadowRenderConfig.postCameraMatrix.LoadIdentity();
		rectShadowRenderConfig.projectionMatrix = ssg.environment.sunShadowProjectionMatrix;
		rectShadowRenderConfig.cameraMatrix = ssg.environment.sunShadowViewMatrix;
		rectShadowRenderConfig.cameraPosition = ssg.environment.sunShadowMapOrigin;
		rectShadowRenderConfig.znear = ssg.environment.sunShadowMapNearZ;
		rectShadowRenderConfig.zfar = ssg.environment.sunShadowMapFarZ;
		rectShadowRenderConfig.clearColor.reset(1.0f, 0.0f, 1.0f, 1.0f);
		rectShadowRenderConfig.clearColorBuffer = true;
		rectShadowRenderConfig.renderToFBO = true;
		ssg.AdvancedRender(rectShadowRenderConfig);
		rectShadowRenderConfig.fbo.GenerateMipmaps();

		// Sun Shadow Map is rendered, now let's bind it to a texture_ map...

		for (auto& rt : rectShadowRenderConfig.fbo.renderTargets) {
			if (rt.second.attachment == GL_COLOR_ATTACHMENT0)
				ssg.environment.sunColorMapId = rt.second.object;
			if (rt.second.attachment == GL_DEPTH_ATTACHMENT)
				ssg.environment.sunDepthMapId = rt.second.object;
		}

		//for (int i = 0; i < (int)rectShadowRenderConfig.fbo.renderTargets.size(); i++)
		//{
		//	rts[rectShadowRenderConfig.fbo.renderTargets[i].first] = &(rectShadowRenderConfig.fbo.renderTargets[i].second);
		//}
		//for (auto rtIt = rts.begin(); rtIt != rts.end(); rtIt++)
		//{
		//	RenderTarget &rt = *(rtIt->second);
		//	if (rt.attachment == GL_COLOR_ATTACHMENT0)
		//	{
		//		ssg.environment.sunColorMapId = rt.object;

		//		rt.unit = ssg.environment.sunColorMapUnit;
		//		rt.sampler = ssg.environment.sunColorMapSamplerId;
		//	}
		//	else if (rt.attachment == GL_DEPTH_ATTACHMENT)
		//	{
		//		ssg.environment.sunDepthMapId = rt.object;

		//		rt.unit = ssg.environment.sunDepthMapUnit;
		//		rt.sampler = ssg.environment.sunDepthMapSamplerId;
		//	}
		//	else
		//	{
		//		rt.unit = 0;
		//		rt.sampler = 0;
		//	}

		//	FxBindTextureAndSampler(rt.unit, rt.target, rt.object, rt.sampler);

		//	int w, h;
		//	int mipLevel = 0;
		//	glGetTexLevelParameteriv(rt.target, mipLevel, GL_TEXTURE_WIDTH, &w);
		//	glGetTexLevelParameteriv(rt.target, mipLevel, GL_TEXTURE_HEIGHT, &h);

		//	FxBindTextureAndSampler(rt.unit, rt.target, 0, 0);
		//	rt.unit = 0;
		//}
		ssg.environment.sunShadowMapTime = (float)(1000.0f * (Hf::Log.getSecondsElapsed() - sunShadowT0));

		if (Interface.captureShadows) {
			SaveTextureMap(GL_TEXTURE_2D, ssg.environment.sunColorMapId, "sun_color.ppm");
			SaveTextureMap(GL_TEXTURE_2D, ssg.environment.sunDepthMapId, "sun_depth.ppm");
		}

		// Render cube shadow map for light 0
		for (int i = 0; i < ssg.pointLights.size(); i++) {
			auto& spl = ssg.pointLights[i];
			auto& scs = ssg.pointLights[i].scs;

			scs.zfar = cubeShadowRenderConfig.zfar;
			cubeShadowRenderConfig.fbo_gen_color = false;

			cubeShadowRenderConfig.renderToFBO = false;
			cubeShadowRenderConfig.useSceneCamera = false;
			cubeShadowRenderConfig.cameraMatrix.LoadIdentity();
			cubeShadowRenderConfig.cameraPosition = Vector4f(spl.position, 1.0f);

			RenderCubeShadowMap(ssg, scs, cubeShadowRenderConfig);
			FxSetErrorMessage("ssphh.cpp", __LINE__, __FUNCTION__);
		}

		for (int i = 0; i < ssgUserData->ssphhLights.size(); i++) {
			auto& sphl = ssgUserData->ssphhLights[i];
			auto& scs = sphl.depthSphlMap;

			scs.zfar = cubeShadowRenderConfig.zfar;
			if (Interface.ssphh.enableShadowColorMap) {
				sphl.colorSphlMap.texture.CreateTextureCube();
				cubeShadowRenderConfig.clearColor.reset(0.2f, 0.4f, 0.6f, 1.0f);
				cubeShadowRenderConfig.fbo_gen_color = true;
				cubeShadowRenderConfig.fbo_color_map = sphl.colorSphlMap.texture.GetTexture();
			}
			else {
				cubeShadowRenderConfig.fbo_gen_color = false;
				cubeShadowRenderConfig.fbo_color_map = 0;
			}
			cubeShadowRenderConfig.renderToFBO = false;
			cubeShadowRenderConfig.useSceneCamera = false;
			cubeShadowRenderConfig.cameraMatrix.LoadIdentity();
			cubeShadowRenderConfig.cameraPosition = sphl.position;

			RenderCubeShadowMap(ssg, sphl.depthSphlMap, cubeShadowRenderConfig);
			FxSetErrorMessage("ssphh.cpp", __LINE__, __FUNCTION__);

			if (Interface.captureShadows) {
				std::ostringstream ostr;
				ostr << "sphl" << std::setw(2) << std::setfill('0') << i;
				SaveTextureMap(GL_TEXTURE_CUBE_MAP, sphl.colorSphlMap.texture.GetTexture(), ostr.str() + "_color.ppm");
				SaveTextureMap(GL_TEXTURE_CUBE_MAP, sphl.depthSphlMap.texture.GetTexture(), ostr.str() + "_depth.ppm");
			}
		}

		Interface.captureShadows = false;
	}
} // namespace SSPHH
