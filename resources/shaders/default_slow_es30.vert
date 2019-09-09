#version 300 es

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

// Uniforms

uniform mat4 ProjectionMatrix;
uniform mat4 InverseCameraMatrix;
uniform mat4 CameraMatrix;
uniform mat4 ModelViewMatrix;
uniform vec4 CameraPosition;

uniform mat4 SunShadowBiasMatrix;
uniform mat4 SunShadowProjectionMatrix;
uniform mat4 SunShadowViewMatrix;
uniform mat4 SunShadowInverseViewMatrix;

// Inputs

in vec4 vtxPosition;
in vec3 vtxNormal;
in vec2 vtxTexCoord;
in vec4 vtxColor;
in vec4 vtxAttrib;
in vec3 vtxTangent;
in vec3 vtxBinormal;

// Outputs

out vec4 VS_Position;
out vec4 VS_CameraPosition;
out vec3 VS_Normal;
out vec2 VS_TexCoord;
out vec4 VS_Color;
out vec3 VS_Attrib;
out vec3 VS_Tangent;
out vec3 VS_Binormal;
out vec4 VS_SunShadowCoord;
out vec3 VS_Ambient;


//////////////////////////////////////////////////////////////////////
// GI_SSPHH_PERVERTEX ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct SphlLight
{
	int Enabled;
	vec3 E0;		// also represents the first degree of SPH
	vec3 Position;
	float DepthShadowZFar;
	vec3 sph[9];
};

uniform samplerCube SphlLightProbeCubeMap[16];
uniform samplerCube SphlEnvironmentCubeMap[16];
uniform samplerCube SphlDepthShadowCubeMap[16];

uniform SphlLight SphlLights[16];
uniform int SphlLightCount;

uniform int ShaderDebugChoice;
uniform int ShaderDebugLight;
uniform int ShaderDebugSphl;

const int SunIndex = 16;
out vec3 VS_LightI[17];

vec4 textureSphlLightProbeCubeMap(int id, vec3 uvw)
{
	if (id == 0) return texture(SphlLightProbeCubeMap[0], uvw).rgba;
	if (id == 1) return texture(SphlLightProbeCubeMap[1], uvw).rgba;
	if (id == 2) return texture(SphlLightProbeCubeMap[2], uvw).rgba;
	if (id == 3) return texture(SphlLightProbeCubeMap[3], uvw).rgba;
	if (id == 4) return texture(SphlLightProbeCubeMap[4], uvw).rgba;
	if (id == 5) return texture(SphlLightProbeCubeMap[5], uvw).rgba;
	if (id == 6) return texture(SphlLightProbeCubeMap[6], uvw).rgba;
	if (id == 7) return texture(SphlLightProbeCubeMap[7], uvw).rgba;
	return vec4(0.0);
}

vec4 textureSphlEnvironmentCubeMap(int id, vec3 uvw)
{
	if (id == 0) return texture(SphlEnvironmentCubeMap[0], uvw);
	if (id == 1) return texture(SphlEnvironmentCubeMap[1], uvw);
	if (id == 2) return texture(SphlEnvironmentCubeMap[2], uvw);
	if (id == 3) return texture(SphlEnvironmentCubeMap[3], uvw);
	if (id == 4) return texture(SphlEnvironmentCubeMap[4], uvw);
	if (id == 5) return texture(SphlEnvironmentCubeMap[5], uvw);
	if (id == 6) return texture(SphlEnvironmentCubeMap[6], uvw);
	if (id == 7) return texture(SphlEnvironmentCubeMap[7], uvw);
	return vec4(0.0);	
}

float textureSphlDepthShadowCubeMap(int id, vec3 uvw)
{
	if (id == 0) return texture(SphlDepthShadowCubeMap[0], uvw).r;
	if (id == 1) return texture(SphlDepthShadowCubeMap[1], uvw).r;
	if (id == 2) return texture(SphlDepthShadowCubeMap[2], uvw).r;
	if (id == 3) return texture(SphlDepthShadowCubeMap[3], uvw).r;
	if (id == 4) return texture(SphlDepthShadowCubeMap[4], uvw).r;
	if (id == 5) return texture(SphlDepthShadowCubeMap[5], uvw).r;
	if (id == 6) return texture(SphlDepthShadowCubeMap[6], uvw).r;
	if (id == 7) return texture(SphlDepthShadowCubeMap[7], uvw).r;
	return 1.0;
}

struct FragmentInfo
{
	vec3 N;
	vec3 R;	// reflection vector
	vec3 V; // view vector	
	float NdotV;
	float NdotR;
};
FragmentInfo Fragment;

bool PrepareForShading()
{
	vec3 N = normalize(VS_Normal);
	vec3 V = normalize(CameraPosition.xyz - VS_Position.xyz);

	Fragment.N = N;
	Fragment.V = V;
	Fragment.R = normalize(reflect(-Fragment.V, N));
	Fragment.NdotV = max(0.0, dot(Fragment.N, Fragment.V));
	Fragment.NdotR = max(0.0, dot(Fragment.N, Fragment.R));

	return true;
}

bool PrepareLight(int i)
{
	if (SphlLights[i].Enabled == 0)
		return false;
	// spread
	float spread = 1.0;
	vec3 k_a = textureSphlLightProbeCubeMap(i, Fragment.N).rgb;
	k_a += textureSphlLightProbeCubeMap(i, Fragment.V).rgb;
	k_a += textureSphlLightProbeCubeMap(i, Fragment.R).rgb;
	if (spread > 0.0)
	{

	}
	VS_Ambient += k_a * 0.3333;
	return true;
}


void GI_SSPHH_PERVERTEX()
{
	PrepareForShading();
	VS_Ambient = vec3(0.0);
	float count = 0.0;
	for (int i = 0; i <= SunIndex; i++)
	{
		if (PrepareLight(i))
			count += 1.0;
	}
	if (count > 0.0)
		VS_Ambient /= (3.14159 * count);
}

//////////////////////////////////////////////////////////////////////
// END GI_SSPHH_PERVERTEX ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

mat3 Top3x3(in mat4 m)
{
	return mat3(
		m[0].x, m[0].y, m[0].z,
		m[1].x, m[1].y, m[1].z,
		m[2].x, m[2].y, m[2].z);
}

mat3 Top3x3Transpose(in mat4 m)
{
	return mat3(
		m[0].x, m[1].x, m[2].x,
		m[0].y, m[1].y, m[2].y,
		m[0].z, m[1].z, m[2].z);
}

// Main Function

void main(void)
{
	mat3 NormalMatrixWorldSpace = Top3x3(ModelViewMatrix);
	mat3 NormalMatrixCamSpace = Top3x3(CameraMatrix);
	mat3 NormalMatrix = NormalMatrixWorldSpace;
	mat3 NormalCameraMatrix = NormalMatrixCamSpace * NormalMatrixWorldSpace;

	mat4 ProjectionModelViewMatrix = ProjectionMatrix * CameraMatrix * ModelViewMatrix;
	mat4 SunBiasProjectionModelViewMatrix = SunShadowBiasMatrix * SunShadowProjectionMatrix * SunShadowViewMatrix;

	VS_Position = ModelViewMatrix * vtxPosition;
	VS_CameraPosition = vec4(CameraMatrix[2].xyz, 1.0);//(CameraPosition - VS_Position);
	VS_TexCoord = vec2(vtxTexCoord.x, 1.0 - vtxTexCoord.y);
	VS_Tangent = NormalMatrix * vtxTangent;
	VS_Binormal = NormalMatrix * vtxBinormal;
	VS_Normal = NormalMatrix * vtxNormal;
	VS_Color = vtxColor;
	VS_SunShadowCoord = SunBiasProjectionModelViewMatrix * VS_Position;

	GI_SSPHH_PERVERTEX();

	gl_Position = ProjectionModelViewMatrix * vtxPosition;
}
