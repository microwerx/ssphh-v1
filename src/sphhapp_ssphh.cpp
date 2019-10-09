#include <ssphhapp.hpp>

namespace SSPHH
{
	void SSPHH_Application::DirtySPHLs() {
		int i = 0;
		for (auto& sphl : ssgUserData->ssphhLights) {
			sphls[i++];
			sphl.dirty = true;
		}
	}

	void SSPHH_Application::UpdateSPHLs() {
		if (!coefs_init) {
			geosphere.load("resources/models/icos4.txt");
			//sphl.randomize();
			//sphl.createMesh(geosphere);
			//sphl.createLightProbe();
			sph_renderer.SetSceneGraph(ssg);
			coefs_init = true;
		}

		if (ssgUserData->ssphhLights.size() != sphls.size()) {
			sphls.clear();
			DirtySPHLs();
		}

		// Determine which sphls we need to recalculate and actually draw
		for (auto& sphl : sphls) {
			sphl.second.enabled = false;
		}

		int i = 0;
		for (auto& sphl : ssgUserData->ssphhLights) {
			if (sphl.randomize) {
				//sphls[i].randomize();
				//for (int lm = 0; lm < sphl.GetMaxCoefficients(); lm++)
				//{
				//	sphl.msph[0][lm] = sphls[i].v_coefs[Sphl::RedComponent][lm];
				//	sphl.msph[1][lm] = sphls[i].v_coefs[Sphl::GreenComponent][lm];
				//	sphl.msph[2][lm] = sphls[i].v_coefs[Sphl::BlueComponent][lm];
				//}
				sphl.Randomize();
				sphl.Standardize();
				sphl.randomize = false;
				sphl.dirty = true;
			}
			if (sphl.randomizePosition) {
				sphl.position.reset(randomSampler(-5.0f, 5.0f), randomSampler(2.0f, 3.0f), randomSampler(-6.0f, 6.0f));
				sphl.randomizePosition = false;
				sphl.depthSphlMap.dirty = true;
			}
			if (sphl.dirty) {
				// Copy coefficients from the scene graph SPHL list
				sphls[i].copyCoefficients(sphl, Interface.ssphh.MaxDegrees, Interface.ssphh.enableBasicShowSPHLs);
				sphls[i].createMesh(geosphere);
				//sphls[i].createLightProbe();
				sphl.dirty = false;
				sphl.depthSphlMap.dirty = true;
			}
			sphls[i].position = sphl.position;
			sphls[i].enabled = sphl.enabled;
			i++;
		}
	}

	void SSPHH_Application::UploadSPHLs() {
		unsigned i = 0;
		for (auto& sphl : ssgUserData->ssphhLights) {
			if (sphl.ptrcLightProbeImage.empty()) {
				sphl.UploadLightProbe(sphl.ptrcLightProbeImage, sphl.ptrcLightProbeTexture);
				sphl.UploadLightProbe(sphl.msphLightProbeImage, sphl.msphLightProbeTexture);
				sphl.UploadLightProbe(sphl.hierLightProbeImage, sphl.hierLightProbeTexture);
			}

			if (Interface.ssphh.enableShadowColorMap) {
				sphls[i].lightProbeTexIds[0] = sphl.colorSphlMap.texture.GetTexture();
			}
			else {
				sphls[i].lightProbeTexIds[0] = sphl.hierLightProbeTexture.GetTexture();
			}
			sphls[i].lightProbeTexIds[1] = sphl.ptrcLightProbeTexture.GetTexture();
			sphls[i].lightProbeTexIds[2] = sphl.msphLightProbeTexture.GetTexture();
			i++;
		}

	}

	void SSPHH_Application::RenderGLES30SPHLs() {
		if (!Interface.ssphh.enableShowSPHLs)
			return;
		//Renderer gl;
		SimpleRenderConfiguration rc = defaultRenderConfig;
		rc.shaderProgram = renderer2.FindProgram("sphl", "sphl");
		rc.clearDepthBuffer = false;
		rc.clearColorBuffer = false;
		rc.enableBlend = false;
		rc.blendFuncSrcFactor = GL_SRC_ALPHA;
		rc.blendFuncDstFactor = GL_ONE;

		if (!rc.shaderProgram) {
			//Hf::Log.info("%s(): sphl shader not found", __FUNCTION__);
			return;
		}

		GLint vloc = rc.shaderProgram->GetAttribLocation("aPosition");
		GLint tloc = rc.shaderProgram->GetAttribLocation("aTexCoord");

		// BEGIN RENDER SPH
		sph_renderer.SetRenderConfig(rc);
		//auto program1 = rc.shaderProgram = renderer2.FindProgram("sphl", "sphl");
		//auto program2 = rc.shaderProgram = renderer2.FindProgram("glut", "cubemap");
		sph_renderer.SaveGLState();
		if (sph_renderer.ApplyRenderConfig()) {
			glActiveTexture(GL_TEXTURE0);
			for (auto& it : sphls) {
				auto& sphl = it.second;

				if (!sphl.enabled)
					continue;

				// glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.lightProbeTexIds[0]);

				rc.shaderProgram->ApplyUniform("SPHL_LightProbeMode", (SimpleUniform)0);
				rc.shaderProgram->ApplyUniform("SPHL_NumDegrees", (SimpleUniform)2);
				rc.shaderProgram->ApplyUniform("SPHL_Coefs", SimpleUniform(GL_FLOAT_VEC4, 9, GL_FALSE, (GLubyte*)sphl.coefs));
				rc.shaderProgram->ApplyUniform("SPHL_LightProbe", (SimpleUniform)0);

				Matrix4f worldMatrix;
				worldMatrix.Translate(sphl.position.x, sphl.position.y, sphl.position.z);
				sph_renderer.RenderMesh(sphl.sph_model, worldMatrix);

				if (!Interface.ssphh.enableBasicShowSPHLs) {
					Matrix4f identityMatrix;
					rc.shaderProgram->ApplyUniform("SPHL_LightProbeMode", (SimpleUniform)1);
					rc.shaderProgram->ApplyUniform("WorldMatrix", (SimpleUniform)identityMatrix);
					// Render light probe cube maps
					float angles[3] = { 270.0f, 135.0f, 45.0f };
					for (auto& a : angles)
						a *= FX_F32_DEGREES_TO_RADIANS;
					float R = 1.0f;
					float S = 0.5f;
					Matrix4f lpWorldMatrix[3];
					rc.shaderProgram->Use();
					for (int i = 0; i < 3; i++) {

						// glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.lightProbeTexIds[i]);
						//if (i == 0)
						//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, enviroCubeTexture3.GetTextureId());
						//else if (i == 1)
						//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, ssg.environment.pbskyColorMapId);
						//else if (i == 2)

						FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.lightProbeTexIds[i]);

						//sph_renderer.RenderMesh(sphl.lightProbeModel, lpWorldMatrix[i]);
						FxDrawGL2CubeMap(
							sphl.position.x + R * cos(angles[i]),
							sphl.position.y + R * sin(angles[i]),
							sphl.position.z,
							S, vloc, tloc);
					}
				}
			}

			FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glActiveTexture(GL_TEXTURE0);
		}

		if (0) //(!Interface.ssphh.enableBasicShowSPHLs)
		{
			for (auto& sphl : ssgUserData->ssphhLights) {
				double S = 0.25;
				double R = 1.0;
				FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.ptrcLightProbeTexture.GetTexture());
				FxDrawGL2CubeMap(
					sphl.position.x + R * 0.707 + S * 1,
					sphl.position.y - R * 0.707,
					sphl.position.z,
					S, vloc, tloc);
				FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.msphLightProbeTexture.GetTexture());
				FxDrawGL2CubeMap(
					sphl.position.x + R * 0.707 + S * 3,
					sphl.position.y - R * 0.707,
					sphl.position.z,
					S, vloc, tloc);
				FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.hierLightProbeTexture.GetTexture());
				FxDrawGL2CubeMap(
					sphl.position.x + R * 0.707 + S * 5,
					sphl.position.y - R * 0.707,
					sphl.position.z,
					S, vloc, tloc);

				//auto i = 0;
				//for (auto & hier : sphl.hierarchies)
				//{
				//	GLuint texture = 0;

				//	texture = hier.debugLightProbe.texture.GetTexture();
				//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
				//	FxDrawGL2CubeMap(
				//		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
				//		sphl.position.y - R * 0.707 - S * 2,
				//		sphl.position.z,
				//		S, vloc, tloc);

				//	texture = hier.debugSphLightProbe.texture.GetTexture();
				//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
				//	FxDrawGL2CubeMap(
				//		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
				//		sphl.position.y - R * 0.707 - S * 4,
				//		sphl.position.z,
				//		S, vloc, tloc);

				//	texture = ssgUserData->ssphhLights[hier.index].ptrcLightProbeTexture.GetTexture();
				//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
				//	FxDrawGL2CubeMap(
				//		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
				//		sphl.position.y - R * 0.707 - S * 6,
				//		sphl.position.z,
				//		S, vloc, tloc);

				//	texture = ssgUserData->ssphhLights[hier.index].msphLightProbeTexture.GetTexture();
				//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
				//	FxDrawGL2CubeMap(
				//		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
				//		sphl.position.y - R * 0.707 - S * 8,
				//		sphl.position.z,
				//		S, vloc, tloc);
				//	i++;
				//}

				FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			}
		}
		sph_renderer.RestoreGLState();
	}

	void SSPHH_Application::RenderGL11Hierarchies() {
		if (Interface.ssphh.enableShowHierarchies) {
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glMultMatrixf(defaultRenderConfig.projectionMatrix.const_ptr());
			glMultMatrixf(defaultRenderConfig.cameraMatrix.const_ptr());

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			std::vector<SimpleVertex> vertices;
			for (int i = 0; i < ssgUserData->ssphhLights.size(); i++) {
				auto& sphl = ssgUserData->ssphhLights[i];
				if (!sphl.enabled)
					continue;

				Color3f color1;
				Color3f color2;

				for (int j = 0; j < ssgUserData->ssphhLights.size(); j++) {
					if (i == j || j < 0)
						continue;

					float h = 0.0f; // clamp(sphl.hierarchies[j].percentVisible, 0.0f, 1.0f) * 0.5f + 0.5f;
					float s = sphl.enabled ? 1.0f : 0.5f;
					float l;
					if (i >= Interface.ssphh.HierarchiesMaxSphls)
						l = 0.25f;
					else
						l = 0.5f;

					SimpleVertex v1, v2;
					v1.color = HLSToRGBf(h, l, s).ToColor4();
					v1.position = ssgUserData->ssphhLights[i].position.xyz();
					v2.color = v1.color;
					v2.position = ssgUserData->ssphhLights[j].position.xyz();

					v1.position += Vector3f(0.0f, -0.5f, 0.0f);
					v2.position += Vector3f(0.0f, 0.5f, 0.0f);

					vertices.push_back(v1);
					vertices.push_back(v2);
				}
				SimpleVertex v1, v2;
				v1.color = sphl.E0 * sphl.GetCoefficientColor(0, 0);
				v2.color = v1.color;
				v1.position = sphl.position.xyz() - Vector3f(0.0f, 0.5f, 0.0f);
				v2.position = sphl.position.xyz() + Vector3f(0.0f, 0.5f, 0.0f);
				vertices.push_back(v1);
				vertices.push_back(v2);
			}

			//glLineWidth(4.0f);
			glEnable(GL_LINE_SMOOTH);
			glBegin(GL_LINES);
			for (auto& v : vertices) {
				glColor4fv(v.color.const_ptr());
				glVertex3fv(v.position.const_ptr());
			}
			glEnd();
			//glLineWidth(1.0f);
			glDisable(GL_LINE_SMOOTH);

			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}

		//glMatrixMode(GL_PROJECTION);
		//glPushMatrix();
		//glLoadIdentity();
		//glMultMatrixf(rc.projectionMatrix.const_ptr());
		//glMultMatrixf(rc.cameraMatrix.const_ptr());

		//glMatrixMode(GL_MODELVIEW);
		//glPushMatrix();
		//glLoadIdentity();
		//for (auto & it : sphls)
		//{
		//	auto & sphl = it.second;
		//	if (!sphl.enabled) continue;

		//	glPushMatrix();
		//	glTranslatef(sphl.position.X, sphl.position.y, sphl.position.z);
		//	FxDrawGL1SolidSphere(1.0, 16, 16);
		//	glPopMatrix();
		//}
		//glPopMatrix();
		//glMatrixMode(GL_PROJECTION);
		//glPopMatrix();
		//glMatrixMode(GL_MODELVIEW);
	}

}