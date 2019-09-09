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
#version 300 es
//#version 400

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
	VS_CameraPosition = (CameraPosition - VS_Position);
	VS_TexCoord = vec2(vtxTexCoord.x, 1.0 - vtxTexCoord.y);
	VS_Tangent = NormalMatrix * vtxTangent;
	VS_Binormal = NormalMatrix * vtxBinormal;
	VS_Normal = NormalMatrix * vtxNormal;
	VS_Color = vtxColor;
	VS_SunShadowCoord = SunBiasProjectionModelViewMatrix * VS_Position;

	gl_Position = ProjectionModelViewMatrix * vtxPosition;
}
