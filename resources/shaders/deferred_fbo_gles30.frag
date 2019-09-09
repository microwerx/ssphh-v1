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

precision highp float;
precision highp int; 

// Uniforms ----------------------------------------------------------

uniform float UScreenWidth;
uniform float UScreenHeight;
uniform vec4 CameraPosition;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 Ke;
uniform float Tf;
uniform vec3 Tr;
uniform float Ni;
uniform float Ns;

uniform float PBKsm;
uniform float PBior;
uniform float PBKdm;
uniform float PBk;
uniform float PBm;

uniform sampler2D map_Kd;
uniform sampler2D map_Ks;
uniform sampler2D map_Ke;
uniform sampler2D map_Ka;
uniform sampler2D map_Tf;
uniform sampler2D map_Tr;
uniform sampler2D map_Ns;
uniform sampler2D map_Ni;
uniform sampler2D map_bump;
uniform sampler2D map_normal;
uniform sampler2D PBmap;

uniform vec4 Spheres[16];
uniform vec4 SpheresKe[16];
uniform int SpheresCount;

uniform float map_Kd_mix;
uniform float map_Ks_mix;
uniform float map_normal_mix;
uniform float PBmap_mix;

uniform vec3 SunDirTo;	// Direction of the light from the origin of the scene
uniform vec3 SunColor;	// Color of the sun (hopefully is (1, 1, 1))
uniform float SunSize;	// emission level of the sun (1.0 is unity)
uniform vec4 SunE0;		// Sun Disk Radiance - vec4(vec3(SunE0), SunMonoE0)
uniform vec4 GroundE0;  // Ground Radiance from the reflected sun.
uniform sampler2D SunShadowMap;
uniform sampler2D SunColorMap;
uniform samplerCube EnviroCubeMap;
uniform float EnviroCubeMapAmount;
uniform samplerCube PBSkyCubeMap;

// Inputs ------------------------------------------------------------


in vec4 VS_Position;
in vec4 VS_CameraPosition;
in vec3 VS_Normal;
in vec2 VS_TexCoord;
in vec4 VS_Color;
in vec3 VS_Attrib;
in vec3 VS_Tangent;
in vec3 VS_Binormal;
in vec4 VS_SunShadowCoord;


// Outputs -----------------------------------------------------------


out vec4 FS_outputColor;


// Mock Deferred Outputs ---------------------------------------------

struct Material
{
	vec3 kd;
	vec3 ks;
	float diffuseRoughness;
	float specularRoughness;
	float specularN2;
	float specularK;
};

Material material;

// outputs from deferred stage
// inputs to light accumulation stage
vec4 DeferredPB1;
vec4 DeferredPB2;
vec4 DeferredPB3;
vec4 DeferredPB4;
vec4 DeferredPB5;
vec4 DeferredPB6;
vec4 DeferredPB7;
vec4 DeferredPB8;
vec4 DeferredDepthStencil;

// outputs from light accumulation stage
vec4 DeferredLightBuffer;

void DoDeferredMain();
void DoDeferredLightAccum();


// -------------------------------------------------------------------


mat3 MakeCotangentFrame(vec3 N, vec3 p, vec2 uv)
{
	vec3 dp1 = dFdx(p);
	vec3 dp2 = dFdy(p);
	vec2 duv1 = dFdx(uv);
	vec2 duv2 = dFdy(uv);

	vec3 dp2perp = cross(dp2, N);
	vec3 dp1perp = cross(N, dp1);
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	float invmax = inversesqrt(max(dot(T,T), dot(B,B)));
	return mat3(T * invmax, B * invmax, N);
}


vec3 PerturbNormal(vec3 N, vec3 V, vec2 texcoord)
{
	vec3 map = 2.0 * texture(map_normal, texcoord).rgb - 1.0;
	mat3 TBN = MakeCotangentFrame(N, -V, texcoord);
	return normalize(TBN * map);
}


// Main Function
void main(void)
{
	DoDeferredMain();
	DoDeferredLightAccum();
	FS_outputColor = DeferredLightBuffer;	
}


vec3 GetMap2dRGB(sampler2D map, vec3 k0, float mixAmount)
{
	if (mixAmount > 0.0) {
		return mixAmount * texture(map, 1.0 - VS_TexCoord).rgb + (1.0 - mixAmount) * k0;
	}
	return k0;
}


vec4 GetMap2dRGBA(sampler2D map, vec4 k0, float mixAmount)
{
	if (mixAmount > 0.0) {
		return mixAmount * texture(map, 1.0 - VS_TexCoord) + (1.0 - mixAmount) * k0;
	}
	return k0;
}


void DoDeferredMain()
{
	// 1. Prepare our deferred variables: P, V, N, Kd
	vec3 N = normalize(VS_Normal);
	vec3 Nmap = N;
	vec3 V = normalize(CameraPosition.xyz - VS_Position.xyz);

	if (map_normal_mix > 0.5) {
		// apply normal map to the normal 
		Nmap = PerturbNormal(N, V, 1.0 - VS_TexCoord);
	}

	vec4 dP1 = dFdx(VS_Position);
	vec4 dP2 = dFdy(VS_Position);
	vec3 fragN = normalize(cross(dP1.xyz, dP2.xyz));
	vec3 fragNmap = fragN;
	if (map_normal_mix > 0.5) {
		fragNmap = PerturbNormal(fragN, V, 1.0 - VS_TexCoord);
	}

	if (map_Kd_mix > 0.5)
		material.kd = texture(map_Kd, 1.0 - VS_TexCoord).rgb * Kd;
	else
		material.kd = Kd;

	material.kd = GetMap2dRGB(map_Kd, Kd, map_Kd_mix);

	if (map_Ks_mix > 0.5)
		material.ks = texture(map_Ks, 1.0 - VS_TexCoord).rgb * Ks;
	else
		material.ks = Ks;

	material.ks = GetMap2dRGB(map_Ks, Ks, map_Ks_mix);

	material.diffuseRoughness = PBKdm;
	material.specularRoughness = PBKsm;
	material.specularN2 = PBior;
	material.specularK = PBk;

	DeferredPB1 = vec4(VS_Position.xyz, 1.0);
	DeferredPB2 = vec4(Nmap.xyz, 1.0);
	DeferredPB3 = vec4(material.kd.rgb, 1.0);
	DeferredPB4 = vec4(material.ks.rgb, 1.0);
	DeferredPB5 = vec4(N.xyz, 1.0);
	DeferredPB6 = vec4(V.xyz, 1.0);
	DeferredLightBuffer = vec4(0.0f, 0.0f, 0.0f, 0.0f);
}
