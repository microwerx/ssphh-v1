#include <fluxions_fileio_iostream.hpp>
#include <fluxions_ssg_ssphh.hpp>

namespace Fluxions
{
	SSG_SSPHH::SSG_SSPHH(SimpleSceneGraph* pointerToSSG)
		: ISimpleRendererPlugin(pointerToSSG) {}

	bool SSG_SSPHH::Reset() {
		ssphhLights.clear();
		ssphhLights.reserve(16);
	}

	bool SSG_SSPHH::Init() {
		ssphhLights.clear();
		ssphhLights.resize(MaxSphlLights);
		for (int i = 0; i < MaxSphlLights; i++) {
			ssphhLights[i].ChangeDegrees(MaxSphlDegree);
			ssphhLights[i].enabled = false;
		}
		return true;
	}

	bool SSG_SSPHH::Redo() {
		for (size_t i = 0; i < ssphhLights.size(); i++) {
			ssphhLights[i].dirty = true;
			ssphhLights[i].depthSphlMap.dirty = true;
		}
		return true;
	}

	bool SSG_SSPHH::Read(const std::string& cmd, std::istringstream& istr) {
		if (cmd == "sphLight") {
			SimpleSSPHHLight sphl;
			sphl.name = ReadString(istr);
			sphl.index = (int)ssphhLights.size();
			sphl.E0 = ReadFloat(istr);
			sphl.falloffRadius = ReadFloat(istr);
			sphl.position = ReadVector3f(istr);
			sphl.orientation = ReadQuaternionf(istr);
			sphl.msph[0] = ReadSphericalHarmonicf(istr);
			sphl.msph[1] = sphl.msph[0];
			sphl.msph[2] = sphl.msph[0];
			sphl.Standardize();
			ssphhLights.push_back(sphl);
			return true;
		}
		else if (cmd == "sphl") {
			ssphhLights.resize(ssphhLights.size() + 1);
			SimpleSSPHHLight& sphl = ssphhLights.back();
			sphl.name = ReadString(istr);
			sphl.index = (int)ssphhLights.size() - 1;
			sphl.E0 = ReadFloat(istr);
			sphl.falloffRadius = ReadFloat(istr);
			sphl.position = ReadVector3f(istr);
			sphl.ChangeDegrees(MaxSphlDegree);
			return true;
		}
		return false;
	}

	bool SSG_SSPHH::ReadUniformLocs(SimpleProgram& program) {
		for (int i = 0; i < 16; i++) {
			char name[64];
			sprintf_s(name, "SphlLights[%d].Enabled", i);
			locs.sphl_lights_enabled[i] = program.GetUniformLocation(name);
			sprintf_s(name, "SphlLights[%d].E0", i);
			locs.sphl_lights_E0[i] = program.GetUniformLocation(name);
			sprintf_s(name, "SphlLights[%d].Position", i);
			locs.sphl_lights_position[i] = program.GetUniformLocation(name);
			sprintf_s(name, "SphlLights[%d].LightProbeCubeMap", i);
			locs.sphl_lights_lightProbeCubeMap[i] = program.GetUniformLocation(name);
			sprintf_s(name, "SphlLights[%d].EnvironmentCubeMap", i);
			locs.sphl_lights_environmentCubeMap[i] = program.GetUniformLocation(name);
			sprintf_s(name, "SphlLights[%d].DepthShadowCubeMap", i);
			locs.sphl_lights_depthShadowCubeMap[i] = program.GetUniformLocation(name);
			sprintf_s(name, "SphlLights[%d].DepthShadowZFar", i);
			locs.sphl_lights_depthShadowZFar[i] = program.GetUniformLocation(name);
			for (int j = 0; j < 9; j++) {
				locs.sphl_lights_sph[i][j] = -1;
				sprintf_s(name, "SphlLights[%d].sph[%d]", i, j);
				locs.sphl_lights_sph[i][j] = program.GetUniformLocation(name);
			}
		}
		return true;
	}

	bool SSG_SSPHH::Prog() {
		int v_Enabled[16] = { 0 };
		Vector3f v_E0[16];
		Vector3f v_Position[16];
		int v_LightProbeCubeMapUnit[16] = { 0 };
		int v_EnvironmentCubeMapUnit[16] = { 0 };
		int v_DepthShadowCubeMapUnit[16] = { 0 };
		GLuint v_LightProbeCubeMapTexId[16] = { 0 };
		GLuint v_EnvironmentCubeMapTexId[16] = { 0 };
		GLuint v_DepthShadowCubeMapTexId[16] = { 0 };
		GLfloat v_DepthShadowZFar[16] = { 0.0 };
		Vector3f v_sph[16 * 9] = { 0.0 };
		int v_MaxIndex = -1;

		for (unsigned i = 0; i < 16; i++) {
			if (locs.sphl_lights_enabled[i] < 0)
				continue;

			if (ssphhLights.size() > i && ssphhLights[i].enabled) {
				v_MaxIndex = i;
				v_Enabled[i] = 1;
				v_E0[i] = Vector3f(ssphhLights[i].E0);
				v_Position[i] = ssphhLights[i].position.xyz();
				v_LightProbeCubeMapTexId[i] = ssphhLights[i].hierLightProbeTexture.GetTexture();
				v_LightProbeCubeMapUnit[i] = ssphhLights[i].colorSphlMap.unit = pssg->GetTexUnit();
				FxBindTexture(v_LightProbeCubeMapUnit[i], GL_TEXTURE_CUBE_MAP, v_LightProbeCubeMapTexId[i]);
				v_EnvironmentCubeMapTexId[i] = ssphhLights[i].ptrcLightProbeTexture.GetTexture();
				v_EnvironmentCubeMapUnit[i] = ssphhLights[i].environmentLightProbeMap.unit = pssg->GetTexUnit();
				FxBindTexture(v_EnvironmentCubeMapUnit[i], GL_TEXTURE_CUBE_MAP, v_EnvironmentCubeMapTexId[i]);
				v_DepthShadowCubeMapTexId[i] = ssphhLights[i].depthSphlMap.texture.GetTexture();
				v_DepthShadowCubeMapUnit[i] = ssphhLights[i].depthSphlMap.unit = pssg->GetTexUnit();
				FxBindTexture(v_DepthShadowCubeMapUnit[i], GL_TEXTURE_CUBE_MAP, v_DepthShadowCubeMapTexId[i]);
				v_DepthShadowZFar[i] = ssphhLights[i].depthSphlMap.zfar;
				for (int sph = 0; sph < 9; sph++) {
					v_sph[i * 9 + sph].x = ssphhLights[i].msph[0].getCoefficient(sph);
					v_sph[i * 9 + sph].y = ssphhLights[i].msph[1].getCoefficient(sph);
					v_sph[i * 9 + sph].z = ssphhLights[i].msph[2].getCoefficient(sph);
				}
			}
		}
		if (v_MaxIndex >= 0) {
			if (locs.sphl_light_count >= 0)
				glUniform1i(locs.sphl_light_count, v_MaxIndex + 1);

			for (int i = 0; i < 16; i++) {
				glUniform1iv(locs.sphl_lights_enabled[i], 1, &v_Enabled[i]);
				glUniform3fv(locs.sphl_lights_E0[i], 1, v_E0[i].const_ptr());
				glUniform3fv(locs.sphl_lights_position[i], 1, v_Position[i].const_ptr());
				glUniform1iv(locs.sphl_lights_lightProbeCubeMap[i], 1, &v_LightProbeCubeMapUnit[i]);
				glUniform1iv(locs.sphl_lights_environmentCubeMap[i], 1, &v_EnvironmentCubeMapUnit[i]);
				glUniform1iv(locs.sphl_lights_depthShadowCubeMap[i], 1, &v_DepthShadowCubeMapUnit[i]);
				glUniform1fv(locs.sphl_lights_depthShadowZFar[i], 1, &v_DepthShadowZFar[i]);
				for (int j = 0; j < 9; j++) {
					glUniform3fv(locs.sphl_lights_sph[i][j], 9, v_sph[i * 9].const_ptr());
				}
			}

			glActiveTexture(GL_TEXTURE0);
		}
		else {
			if (locs.sphl_light_count >= 0)
				glUniform1i(locs.sphl_light_count, 0);
		}
		return true;
	}

	bool SSG_SSPHH::Draw() {
		for (unsigned i = 0; i < 16; i++) {
			if (ssphhLights.size() > i && ssphhLights[i].colorSphlMap.unit > 0) {
				pssg->FreeTexUnit(ssphhLights[i].colorSphlMap.unit);
				pssg->FreeTexUnit(ssphhLights[i].depthSphlMap.unit);
				pssg->FreeTexUnit(ssphhLights[i].environmentLightProbeMap.unit);
			}
		}
		return true;
	}
}