// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/KASL/GLUT Extensions
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
#version 300 es

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
	samplerCube LightProbeCubeMap;
	samplerCube EnvironmentCubeMap;
	samplerCube DepthShadowCubeMap;
	float DepthShadowZFar;
	vec3 sph[9];
};

uniform SphlLight SphlLights[16];
uniform int SphlLightCount;

uniform int ShaderDebugChoice;
uniform int ShaderDebugLight;
uniform int ShaderDebugSphl;

int SunIndex = 16;
out vec3 VS_LightI[17];

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
	vec3 k_a = texture(SphlLights[i].LightProbeCubeMap, Fragment.N).rgb;
	k_a += texture(SphlLights[i].LightProbeCubeMap, Fragment.V).rgb;
	k_a += texture(SphlLights[i].LightProbeCubeMap, Fragment.R).rgb;
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
