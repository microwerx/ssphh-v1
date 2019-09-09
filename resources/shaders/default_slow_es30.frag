#version 300 es
#pragma optionNV(strict on)

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

precision highp float;
precision highp int; 

// Uniforms ----------------------------------------------------------

uniform float UScreenWidth;
uniform float UScreenHeight;
uniform vec4 CameraPosition;
uniform float ToneMapScale;
uniform float ToneMapExposure;
uniform float ToneMapGamma;

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
uniform float PBGGXgamma;

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

struct SphlLight
{
	int Enabled;
	vec3 E0;		// also represents the first degree of SPH
	vec3 Position;
	float DepthShadowZFar;
	vec3 sph[9];
};

struct PointLight
{
	int Enabled;
	vec3 E0;
	vec3 Position;
	float DepthShadowZFar;
};

struct LightInfo
{
	vec3 L;
	vec3 H;
	vec3 I;
	float E0;
	float NdotL;
	float NdotV;
	float NdotH;
	float cos_theta_d;
	float distance;
	float distance2;
	float shadow;
	int isGlobal;
};

struct FragmentInfo
{
	vec3 N;
	vec3 R;	// reflection vector
	vec3 V; // view vector	
	float NdotV;
	float NdotR;
};

struct MaterialInfo
{
	vec3 Kd;
	vec3 Ks;
	vec3 Ka;
	float diffuseRoughness;
	float diffuseRoughness2;
	float specularRoughness;
	float specularRoughness2;
	float specularExponent;
	float specularGGXgamma;
	float specularN2;
	float specularK;
	float F0;
};

MaterialInfo Material;

LightInfo Lights[17];
const int SunIndex = 16;
FragmentInfo Fragment;

int firstLightIndex = 0;
int lastLightIndex = SunIndex;
const int ShadowQuality = 0;

uniform SphlLight SphlLights[16];
uniform int SphlLightCount;

uniform PointLight PointLights[16];
uniform int PointLightCount;

uniform samplerCube SphlLightProbeCubeMap[16];
uniform samplerCube SphlEnvironmentCubeMap[16];
uniform samplerCube SphlDepthShadowCubeMap[16];
uniform samplerCube PointLightDepthShadowCubeMap[16];

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

uniform int ShaderDebugChoice;
uniform int ShaderDebugLight;
uniform int ShaderDebugSphl;


/*
layout (std140) uniform FluxionsParameters
{
	vec3 sunDirTo; // Direction of the light from the origin of the scene
	vec3 sunColor; // Color of the sun (hopefully is (1, 1, 1))
	float sunSize; // emission level of the sun (1.0 is unity)
	//sampler2DShadow sunShadowMap;

	//samplerCube enviroCubeMap;
};
*/

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
in vec3 VS_Ambient;


// Outputs -----------------------------------------------------------


out vec4 FS_outputColor;


// Globals and Constants ---------------------------------------------


float pi = 3.1415926;
float invPi = 0.31831;
float epsilon = 0.00001;
float fragmentArea = 1.0;
float toneMapScale = 0.0390625; // 2.5 * 2^-6
float mobileMode = 0.0;
float BumpinessFactor = 0.5;//0.125;

float SunShadowDarkness = 0.1;
float ShadowDarkness = 0.001;//1;

float lambda_r = 650.0;
float lambda_g = 532.0;
float lambda_b = 473.0;

// Tristimulus
const vec3 Al_ior1 = vec3(1.095, 1.018, 0.621);
const vec3 Al_coef1 = vec3(6.924, 6.783, 5.471);
// RGB Gaussian
const vec3 Al_ior2 = vec3(1.28, 0.996, 0.642);
const vec3 Al_coef2 = vec3(7.42, 6.765, 5.554);
// Sampled at R: 615nm, G: 557nm, B: 458nm, 
const vec3 Al_ior3 = vec3(1.274, 0.99, 0.641);
const vec3 Al_coef3 = vec3(7.425, 6.771, 5.566);
// Tristimulus
const vec3 Ag_ior1 = vec3(0.131, 0.127, 0.15);
const vec3 Ag_coef1 = vec3(3.498, 3.405, 2.466);
// RGB Gaussian
const vec3 Ag_ior2 = vec3(0.129, 0.124, 0.146);
const vec3 Ag_coef2 = vec3(3.848, 3.393, 2.535);
// Sampled at R: 615nm, G: 557nm, B: 458nm, 
const vec3 Ag_ior3 = vec3(0.129, 0.122, 0.145);
const vec3 Ag_coef3 = vec3(3.842, 3.396, 2.548);
// Tristimulus
const vec3 Cu_ior1 = vec3(0.607, 0.781, 1.164);
const vec3 Cu_coef1 = vec3(2.878, 2.743, 2.385);
// RGB Gaussian
const vec3 Cu_ior2 = vec3(0.35, 0.842, 1.156);
const vec3 Cu_coef2 = vec3(3.168, 2.65, 2.413);
// Sampled at R: 615nm, G: 557nm, B: 458nm, 
const vec3 Cu_ior3 = vec3(0.298, 0.892, 1.161);
const vec3 Cu_coef3 = vec3(3.164, 2.587, 2.428);
// Tristimulus
const vec3 Au_ior1 = vec3(0.448, 0.419, 1.449);
const vec3 Au_coef1 = vec3(2.792, 2.66, 1.888);
// RGB Gaussian
const vec3 Au_ior2 = vec3(0.207, 0.36, 1.371);
const vec3 Au_coef2 = vec3(3.046, 2.708, 1.866);
// Sampled at R: 615nm, G: 557nm, B: 458nm, 
const vec3 Au_ior3 = vec3(0.2, 0.327, 1.437);
const vec3 Au_coef3 = vec3(3.045, 2.802, 1.85);
// Tristimulus
const vec3 W_ior1 = vec3(3.524, 3.504, 3.326);
const vec3 W_coef1 = vec3(2.785, 2.768, 2.524);
// RGB Gaussian
const vec3 W_ior2 = vec3(3.597, 3.5, 3.32);
const vec3 W_coef2 = vec3(2.878, 2.759, 2.544);
// Sampled at R: 615nm, G: 557nm, B: 458nm, 
const vec3 W_ior3 = vec3(3.589, 3.495, 3.309);
const vec3 W_coef3 = vec3(2.879, 2.738, 2.55);
// Tristimulus
const vec3 Ni_ior1 = vec3(1.834, 1.796, 1.635);
const vec3 Ni_coef1 = vec3(3.371, 3.301, 2.654);
// RGB Gaussian
const vec3 Ni_ior2 = vec3(1.914, 1.786, 1.636);
const vec3 Ni_coef2 = vec3(3.618, 3.291, 2.694);
// Sampled at R: 615nm, G: 557nm, B: 458nm, 
const vec3 Ni_ior3 = vec3(1.907, 1.786, 1.639);
const vec3 Ni_coef3 = vec3(3.621, 3.292, 2.702);
// Tristimulus
const vec3 Si_ior1 = vec3(4.094, 4.085, 4.722);
const vec3 Si_coef1 = vec3(0.051, 0.043, 0.159);
// RGB Gaussian
const vec3 Si_ior2 = vec3(3.92, 4.07, 4.625);
const vec3 Si_coef2 = vec3(0.023, 0.039, 0.142);
// Sampled at R: 615nm, G: 557nm, B: 458nm, 
const vec3 Si_ior3 = vec3(3.916, 4.062, 4.594);
const vec3 Si_coef3 = vec3(0.024, 0.036, 0.13);
// Tristimulus
const vec3 Ti_ior1 = vec3(1.977, 1.929, 1.686);
const vec3 Ti_coef1 = vec3(2.704, 2.646, 2.269);
// RGB Gaussian
const vec3 Ti_ior2 = vec3(2.094, 1.912, 1.7);
const vec3 Ti_coef2 = vec3(2.859, 2.637, 2.284);
// Sampled at R: 615nm, G: 557nm, B: 458nm, 
const vec3 Ti_ior3 = vec3(2.093, 1.904, 1.708);
const vec3 Ti_coef3 = vec3(2.862, 2.641, 2.287);
// Tristimulus
const vec3 Cr_ior1 = vec3(3.031, 3.077, 2.344);
const vec3 Cr_coef1 = vec3(3.277, 3.309, 3.124);
// RGB Gaussian
const vec3 Cr_ior2 = vec3(3.164, 3.14, 2.403);
const vec3 Cr_coef2 = vec3(3.309, 3.319, 3.151);
// Sampled at R: 615nm, G: 557nm, B: 458nm, 
const vec3 Cr_ior3 = vec3(3.168, 3.191, 2.391);
const vec3 Cr_coef3 = vec3(3.3, 3.323, 3.174);

#define METALTYPE 2
#define METAL 3

#if METAL == 11
vec3 Metal_N2 = Al_ior2;
vec3 Metal_K2 = Al_coef2;
#elif METAL == 2
vec3 Metal_N2 = Ag_ior2;
vec3 Metal_K2 = Ag_coef2;
#elif METAL == 3
vec3 Metal_N2 = Au_ior2;
vec3 Metal_K2 = Au_coef2;
#elif METAL == 4
vec3 Metal_N2 = W_ior2;
vec3 Metal_K2 = W_coef2;
#elif METAL == 5
vec3 Metal_N2 = Ni_ior2;
vec3 Metal_K2 = Ni_coef2;
#elif METAL == 6
vec3 Metal_N2 = Si_ior2;
vec3 Metal_K2 = Si_coef2;
#elif METAL == 7
vec3 Metal_N2 = Ti_ior2;
vec3 Metal_K2 = Ti_coef2;
#elif METAL == 8
vec3 Metal_N2 = Cr_ior2;
vec3 Metal_K2 = Cr_coef2;
#elif METAL == 9
vec3 Metal_N2 = Cu_ior2;
vec3 Metal_K2 = Cu_coef2;
#elif METAL == 10
vec3 Metal_N2 = Au_ior1;
vec3 Metal_K2 = Au_coef1;
#elif METAL == 11
vec3 Metal_N2 = Au_ior2;
vec3 Metal_K2 = Au_coef2;
#elif METAL == 12
vec3 Metal_N2 = Au_ior3;
vec3 Metal_K2 = Au_coef3;
#elif METAL == 13
vec3 Metal_N2 = Cu_ior1;
vec3 Metal_K2 = Cu_coef1;
#elif METAL == 14
vec3 Metal_N2 = Cu_ior2;
vec3 Metal_K2 = Cu_coef2;
#elif METAL == 15
vec3 Metal_N2 = Cu_ior3;
vec3 Metal_K2 = Cu_coef3;
#endif

const int metalFresnelType = METALTYPE;



// PROTOTYPES --------------------------------------------------------


vec3 PerturbNormal(vec3 N, vec3 V, vec2 texcoord);
float ComputeFresnelSchlick(float ior, float NdotH);
float ComputeFresnelSchlick2(float F0, float cos_theta);
vec3 FresnelMetalSchlick(in vec3 n, in vec3 k, in float cos_theta_i);
vec3 FresnelMetalApprox(in vec3 n, in vec3 k, in float cos_theta_i);
float ComputeBlinnPhong(float NdotH, float power);
float ComputeOrenNayer2(int i);
float ComputeDisneyDiffuse(int i);
float G(float NdotL, float NdotV, float NdotH, float VdotH);
float G1_GGX(float c, float a);
float G2_GGX(int i);

// Mock Deferred Outputs ---------------------------------------------

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
vec3 sunShadowCoord;

// outputs from light accumulation stage
vec3 DeferredLightBuffer;

void DoDeferredMain();
void DoDeferredLightAccum();


vec4 tone_map(vec4 color, float exp)
{
    return 1.0f - exp2(-color * exp);
}


// PBSky ------------------------------------------------------------//

vec4 GetPBSky(vec3 L, float spread)
{
	return texture(PBSkyCubeMap, L);
	bool inDisc = false;
	float sunE0Length = length(SunE0.rgb);
	float groundE0Length = length(GroundE0.rgb); 
	float whiteBalanceScaleFactor;
	float invWhiteBalanceScaleFactor;

	float E0Length = 2.0 * 3.14159 * log(SunDirTo.y * groundE0Length + (1.0 - SunDirTo.y) * sunE0Length);
	if (E0Length > 0.0) {
		whiteBalanceScaleFactor = 1.0 / E0Length;
		invWhiteBalanceScaleFactor = E0Length;
	}
	else {
		whiteBalanceScaleFactor = 0.1;
		invWhiteBalanceScaleFactor = 1.0;
	}
	vec4 sunE0 = whiteBalanceScaleFactor * SunE0;
	vec4 groundE0 = whiteBalanceScaleFactor * GroundE0;
	bool atNight = false;

	if (SunDirTo.y >= 0.0) {
		atNight = false;
	} else {
		atNight = true;
	}

	float cosine = max(0.0, dot(normalize(L), normalize(SunDirTo)));
	if (cosine > 0.99999) {
		inDisc = true;
	} 

	vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);
	if (atNight) {	
		outputColor = vec4(0.0, 0.0, 0.0, 0.0);		
	}
	else {
		if (L.y >= 0.0) {
			if (inDisc) {
				outputColor += SunE0;					
			}
			else {
				outputColor += texture(PBSkyCubeMap, L);// * whiteBalanceScaleFactor;
			}
		}
		else {
			//outputColor += groundE0;// * invWhiteBalanceScaleFactor;
			//outputColor += textureCube(uCubeTexture, vec3(0.0, -1.0, 0.0));
			outputColor += texture(PBSkyCubeMap, L);
		}
	}
	outputColor.a = 1.0;
	return outputColor;
}

// Plane Shadow Mapping Code -----------------------------------------


// This returns the factor of shadow using the Sun Shadow Map. If the
// coordinates are out of bounds of the shadow texture, then the default
// behavior should be to return 1.0, completely shadowed.
float GetSunShadow()
{
	float amountInShadow = 0.0;

	float bias = 0.0009;
	float offset = -0.0002;
	float biasW = bias / VS_SunShadowCoord.w;

	//if (SunSize > 0) amountInShadow = texture(SunShadowMap, VS_SunShadowCoord.xyz);
	vec3 sunShadowCoord = vec3(VS_SunShadowCoord.xy, VS_SunShadowCoord.z) / VS_SunShadowCoord.w;

	vec3 sunTexMapColor = textureProj(SunColorMap, VS_SunShadowCoord.xyw).rgb;
	float sunZ = textureProj(SunShadowMap, VS_SunShadowCoord.xyw).z;

	float sunZsamples[17];
	sunZsamples[0] = sunShadowCoord.z - bias > texture(SunShadowMap, sunShadowCoord.xy + vec2(offset, offset)).z ? 0.0 : 1.0;
	sunZsamples[1] = sunShadowCoord.z - bias > texture(SunShadowMap, sunShadowCoord.xy + vec2(-offset, offset)).z ? 0.0 : 1.0;
	sunZsamples[2] = sunShadowCoord.z - bias > texture(SunShadowMap, sunShadowCoord.xy + vec2(offset, -offset)).z ? 0.0 : 1.0;
	sunZsamples[3] = sunShadowCoord.z - bias > texture(SunShadowMap, sunShadowCoord.xy + vec2(-offset, -offset)).z ? 0.0 : 1.0;
	sunZsamples[4] = sunShadowCoord.z - bias > texture(SunShadowMap, sunShadowCoord.xy + vec2(offset, 0)).z ? 0.0 : 1.0;
	sunZsamples[5] = sunShadowCoord.z - bias > texture(SunShadowMap, sunShadowCoord.xy + vec2(-offset, 0)).z ? 0.0 : 1.0;
	sunZsamples[6] = sunShadowCoord.z - bias > texture(SunShadowMap, sunShadowCoord.xy + vec2(0, -offset)).z ? 0.0 : 1.0;
	sunZsamples[7] = sunShadowCoord.z - bias > texture(SunShadowMap, sunShadowCoord.xy + vec2(-0, -offset)).z ? 0.0 : 1.0;
	sunZsamples[8] = sunShadowCoord.z - bias > texture(SunShadowMap, sunShadowCoord.xy).z ? 0.0 : 1.0;

	float sampleSum = 0.0;
	for (int i = 0; i < 9; i++) {
		sampleSum += sunZsamples[i];
	}
	sampleSum /= 9.0;

	//sunTexMapColor = sunShadowCoord.xyz;	

	float shadowAmount = 1.0;
	if (sunShadowCoord.z < 0.0 || sunShadowCoord.z > 1.0)
		shadowAmount = 0.5;
	if ((sunShadowCoord.z - biasW) >= sunZ)
		shadowAmount = 0.0;

	return sampleSum;
}


// Cube Shadow Mapping Code ------------------------------------------


// This returns the factor of shadow using one of the light Shadow Maps.
// If the lightId is not enabled, then return 0.0, completely shadowed,
// because the absence of light is darkness.
float GetLightShadow(int lightId)
{
	if (lightId < 0 || lightId > SunIndex)
		return 0.0;

	if (Lights[lightId].NdotL <= 0.0)
		return 0.0;

	float shadow;
	if (lightId == SunIndex)
	{
		shadow = GetSunShadow();
	}
	else if (SphlLights[lightId].Enabled > 0)
	{
		float LightZ = SphlLights[lightId].DepthShadowZFar * textureSphlDepthShadowCubeMap(lightId, Lights[lightId].L);
		//*  texture(SphlDepthShadowCubeMap[lightId], Lights[lightId].L).r;
		float Z = Lights[lightId].distance;
		float bias = 1.0;
		shadow = Z - bias > LightZ ? 0.0 : 1.0;
	}
	else
	{
		shadow = 0.0;
	}	
	return shadow;
}


float GetLightShadowMS(int lightId, float samples)
{
	if (lightId < 0 || lightId > SunIndex)
		return 0.0;

	if (Lights[lightId].NdotL <= 0.0)
		return 0.0;		

	if (lightId == SunIndex)
	{
		Lights[lightId].shadow = GetSunShadow();
	}
	else if (SphlLights[lightId].Enabled > 0)
	{
		float shadow = 0.0;
		float bias = 1.0;
		float offset = 1.5; // 0.1
		//float samples = 4.0;
		float doffset = offset / samples;
		float Z = Lights[lightId].distance;

		vec3 L = -Lights[lightId].L;
		for (float x = -offset; x < offset; x += doffset)
		{
			for (float y = -offset; y < offset; y += doffset)
			{
				for (float z = -offset; z < offset; z += doffset)
				{
					float LightZ = SphlLights[lightId].DepthShadowZFar * textureSphlDepthShadowCubeMap(lightId, L + vec3(x, y, z));
					 // * texture(SphlDepthShadowCubeMap[lightId], L + vec3(x,y,z)).r;
					shadow += Z - bias > LightZ ? 0.0 : 1.0;
				}
			}
		}

		Lights[lightId].shadow = shadow /= (samples * samples * samples);
	}
	return Lights[lightId].shadow;
}


float GetLightShadowDiskPCF(int lightId)
{
	if (lightId < 0 || lightId > SunIndex)
		return 0.0;

	if (Lights[lightId].NdotL <= 0.0)
		return 0.0;		

	if (lightId == SunIndex)
	{
		Lights[lightId].shadow = GetSunShadow();
	}
	else if (SphlLights[lightId].Enabled > 0)
	{
		vec3 L = -Lights[lightId].L;

		vec3 sampleOffsetDirections[21] = vec3[]
		(
			vec3( 0,  0,  0 ),
			vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
			vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
			vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
			vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
			vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
		);
		float shadow = 0.0;
		float bias = 1.05;
		float Z = Lights[lightId].distance;
		float diskRadius = 0.05;

		int samples = 21;
		for (int i = 0; i < samples; i++)
		{
			float LightZ = SphlLights[lightId].DepthShadowZFar * textureSphlDepthShadowCubeMap(lightId, L + sampleOffsetDirections[i] * diskRadius);
						   // texture(SphlDepthShadowCubeMap[lightId],
						   // 	       L + sampleOffsetDirections[i] * diskRadius).r;
			shadow += Z - bias > LightZ ? 0.0 : 1.0;
		}

		Lights[lightId].shadow = shadow / float(samples);

		// float shadow = 0.0;
		// float bias = 1.0;
		// float offset = 0.1;
		// float samples = 4.0;
		// float doffset = offset / (samples * 0.5);
		// float Z = Lights[lightId].distance;
		// for (float x = -offset; x < offset; x += doffset)
		// {
		// 	for (float y = -offset; y < offset; y += doffset)
		// 	{
		// 		for (float z = -offset; z < offset; z += doffset)
		// 		{
		// 			float LightZ = SphlLights[lightId].DepthShadowZFar * texture(SphlLights[lightId].DepthShadowCubeMap, L + vec3(x,y,z)).r;
		// 			shadow += Z - bias > LightZ ? 0.0 : 1.0;
		// 		}
		// 	}
		// }

		// Lights[lightId].shadow = shadow /= (samples * samples * samples);
	}
	else
	{
		return 0.0;
	}
	return Lights[lightId].shadow;
}


bool PrepareForShading()
{
	vec3 N = normalize(VS_Normal);
	vec3 V = normalize(CameraPosition.xyz - VS_Position.xyz);
	if (map_normal_mix > 0.5)
	{
		N = PerturbNormal(N, V, VS_TexCoord);
	}
	Fragment.N = N;
	Fragment.V = V;
	Fragment.R = normalize(reflect(-Fragment.V, N));
	Fragment.NdotV = max(0.001, dot(Fragment.N, Fragment.V));
	Fragment.NdotR = max(0.001, dot(Fragment.N, Fragment.R));

	return true;
}


bool PrepareMaterial()
{
	//vec2 texcoord = vec2(VS_TexCoord.s, 1.0 - VS_TexCoord.t);
	if (map_Kd_mix > 0.5)
		Material.Kd = texture(map_Kd, VS_TexCoord).rgb;// * Kd;
	else
		Material.Kd = Kd;

	if (map_Ks_mix > 0.5)
		Material.Ks = texture(map_Ks, VS_TexCoord).rgb;// * Ks;
	else
		Material.Ks = Ks;

	Material.Ka = VS_Ambient;

	float ior;
	if (PBior == 0.0)
	{
		ior = 1.5;
	}
	else
	{
		ior = PBior;
	}

	Material.diffuseRoughness = PBKdm;
	Material.diffuseRoughness2 = Material.diffuseRoughness * Material.diffuseRoughness;
	float m = clamp(abs(PBKsm), 0.0, 1.0);
	Material.specularRoughness = PBKsm;
	Material.specularRoughness2 = m*m;
	Material.specularGGXgamma = 1.0 + (9.0 * PBGGXgamma);
	Material.specularExponent = max(0.0, 2.0 / (m * m) - 2.0);
	Material.specularN2 = ior;
	Material.specularK = 0.0;//PBk;
	Material.F0 = max(0.04, pow((1.0 - ior) / (1.0 + ior), 2.0));

	return true;
}


bool PrepareLight(int i)
{
	if (i >= SunIndex)
	{
		i = SunIndex;
		Lights[i].L = normalize(SunDirTo);
		Lights[i].distance = 1.0;
		Lights[i].distance2 = 1.0;
		Lights[i].I = SunE0.rgb;
		Lights[i].E0 = max(0.0, dot(Fragment.N, Lights[i].L));
	}
	else
	{
		if (SphlLights[i].Enabled <= 0)
		{
			Lights[i].E0 = 0.0;
			return false;
		}

		vec3 L = SphlLights[i].Position.xyz - VS_Position.xyz;
		Lights[i].L = normalize(L);
		Lights[i].distance = length(L);
		Lights[i].distance2 = dot(L, L);
		vec3 texcoord = vec3(L.x, -L.y, L.z);
		Lights[i].I = textureSphlLightProbeCubeMap(i, texcoord).rgb;// * 6.0;
		Lights[i].E0 = 1.0;
	}
	Lights[i].H = normalize(Fragment.V + Lights[i].L);
	Lights[i].NdotL = max(0.0, dot(Fragment.N, Lights[i].L));
	Lights[i].NdotV = max(0.001, dot(Fragment.N, Fragment.V));
	//Lights[i].NdotL = dot(Fragment.N, Lights[i].L); if (Lights[i].NdotL <= 0.0) return false;
	//Lights[i].NdotV = dot(Fragment.N, Fragment.V); if (Lights[i].NdotV <= 0.0) return false;
	Lights[i].NdotH = max(0.001, dot(Fragment.N, Lights[i].H));
	Lights[i].cos_theta_d = dot(Lights[i].L, Lights[i].H);
	float shadow = 1.0;
	switch (ShadowQuality)
	{
	case 0:
		shadow = GetLightShadow(i);
		break;
	case 1:
		shadow = GetLightShadowMS(i, 1.0);
		break;
	case 2:
		shadow = GetLightShadowMS(i, 2.0);
		break;
	case 3:
		shadow = GetLightShadowMS(i, 4.0);
		break;
	case 4:
		shadow = GetLightShadowDiskPCF(i);
		break;
	default:
		shadow = GetLightShadow(i);
	}
	Lights[i].shadow = ShadowDarkness + (1.0 - ShadowDarkness) * shadow;

	return true;
}


int FindClosestLight(int firstLightIndex, int lastLightIndex)
{
	int closest = SunIndex;
	float closest_distance = 1e6;
	for (int i = firstLightIndex; i <= lastLightIndex; i++)
	{
		if (i >= SunIndex)
			return closest;

		vec3 L = SphlLights[i].Position.xyz - VS_Position.xyz;
		float distance = dot(L,L);
		L = normalize(L);
		
		float NdotL = dot(L, Fragment.N);
		if (NdotL > 0.0 && closest_distance > distance)
		{
			closest_distance = distance;
			closest = i;
		}
	}	
	return closest;
}


float GetMobileDiffuseBRDF(int i)
{
	return 1.0 / 3.14159;
}


float GetMobileSpecularBRDF(int i)
{
	float F = ComputeFresnelSchlick(Material.specularN2, Lights[i].cos_theta_d);
	float brdf = 3.14159/4.0 * F * ComputeBlinnPhong(Lights[i].NdotH, Material.specularExponent);
	return brdf;
}


vec3 GetEnviroColor(int lightId)
{
	if (lightId == SunIndex)
	{
		return GetPBSky(Fragment.R, 0.0).rgb;
	}
	else if (lightId >= 0)
	{
		vec3 R = vec3(-Fragment.R.x, Fragment.R.y, Fragment.R.z);
		return textureSphlEnvironmentCubeMap(lightId, R).rgb;
		//return texture(SphlLights[lightId].LightProbeCubeMap, R).rgb;
	}
	return vec3(0.0);//vec3(Fragment.R * 0.5 + 0.5);
}


vec3 CalcBRDF(int lightId)
{
	int i = lightId;
	if (lightId < 0 || lightId < SunIndex)
	{
		if (SphlLights[i].Enabled <= 0)
		{
			return vec3(0.0, 0.0, 0.0);
		}
		vec3 diffuseColor;
		vec3 specularColor;
		vec3 lightColor = textureSphlLightProbeCubeMap(i, Lights[i].L).rgb;

		diffuseColor = Lights[i].NdotL * Material.Kd;
		specularColor = pow(Lights[i].NdotH, Material.specularExponent) * Material.Ks;

		return diffuseColor + specularColor;
	}
	else if (lightId == SunIndex)
	{
		if (Lights[i].L.y <= 0.0)
		{
			return vec3(0.0, 0.0, 0.0);
		}

		vec3 diffuseColor;
		vec3 specularColor;
		vec3 lightColor = SunE0.rgb;

		diffuseColor = lightColor * Lights[i].NdotL * Material.Kd;
		specularColor = lightColor * pow(Lights[i].NdotH, Material.specularExponent) * Material.Ks;

		return diffuseColor + specularColor;		
	}
	return vec3(0.0, 0.0, 0.0);
}


vec3 GetPbrBrdfDiffuse(int i)
{
	float f_r = 1.0 / 3.14159;
	if (mobileMode != 0.0)
	{
		return f_r * Material.Kd;
	}
	else if (Material.diffuseRoughness < 0.0)
	{
		f_r = ComputeDisneyDiffuse(i);
	}
	else if (Material.diffuseRoughness > 0.0)
	{
		f_r = ComputeOrenNayer2(i);
	}
	//float Fd = ComputeFresnelSchlick2(Material.F0, Lights[i].cos_theta_d);
	//float Fd2 = Fd * Fd;

	return f_r * Material.Kd;
}


vec3 GetPbrBrdfSpecular(int i)
{
	// if (Fragment.NdotV <= 0.0 || Lights[i].NdotL <= 0.0)
	// 	return vec3(0.0);
	vec3 F;
	if (Material.specularK > 0.0)
	{
		F = FresnelMetalSchlick(Metal_N2, Metal_K2, Lights[i].cos_theta_d);
	}
	else
	{
		float f = ComputeFresnelSchlick2(Material.F0, Lights[i].cos_theta_d);
		F = vec3(f, f, f) * Material.Ks;
	}

	float D;
	if (Material.specularRoughness >= 0.0)
	{		
		// GGX Generalized-Trowbridge-Reitz
		float alpha = Material.specularRoughness2;
		float c2 = Lights[i].NdotH * Lights[i].NdotH;
		float t2 = (1.0 - c2) / c2;
		D = (1.0 / 3.14159) * pow(alpha / (c2 * (alpha*alpha + t2)), Material.specularGGXgamma);
	}
	else
	{
		float C = (2.0 + Material.specularExponent) / (2.0 * 3.14159);
		D = C * pow(Lights[i].NdotH, Material.specularExponent);
	}	

	//float G(float NdotL, float NdotV, float NdotH, float VdotH)
	float G2;
	if (mobileMode != 0.0)
	{
		return 3.14159 * F * D;
	}
	else
	if (Material.specularRoughness >= 0.0)
	{
		G2 = G2_GGX(i);
	}
	else
	{
		G2 = G(Lights[i].NdotL, Lights[i].NdotV, Lights[i].NdotH, Lights[i].cos_theta_d);
	}

	vec3 f_r = 3.14159 * F * D * G2 / (4.0 * Lights[i].NdotL * Lights[i].NdotV);
	return f_r;
}


vec3 GetPbrEnviroBrdfFresnel(int i)
{
	vec3 F;
	if (Material.specularK > 0.0)
	{
		F = FresnelMetalSchlick(Metal_N2, Metal_K2, Lights[i].cos_theta_d);
	}
	else
	{
		float f = ComputeFresnelSchlick2(Material.F0, Lights[i].cos_theta_d);
		F = vec3(f, f, f) * Material.Ks;
	}
	return F;
}


vec3 GetPbrEnviroBrdf(int i)
{
	// figure out if occluded
	float z = 1.0;
	if (i == SunIndex)
	{
		z = texture(SphlDepthShadowCubeMap[0], Fragment.R).z;
		if (z != 1.0) return vec3(0.0);
	}
	//if (Fragment.NdotV <= 0.001) return vec3(0.0);
	float C = (2.0 + Material.specularExponent) / (2.0 * 3.14159);
	vec3 F = GetPbrEnviroBrdfFresnel(i);
	return GetEnviroColor(i) * F * C * G2_GGX(i) / (4.0 * Fragment.NdotV);
}


vec3 GetPbrAmbient(int i)
{
	if (i < SunIndex)
		return textureSphlLightProbeCubeMap(i, Fragment.N).rgb;
	return vec3(0.0);
}


// Normal Mapping Code -----------------------------------------------


mat3 MakeInverseMat3(mat3 M)
{
	mat3 M_t = transpose(M);
	float det = dot(cross(M_t[0], M_t[1]), M_t[2]);
	mat3 adjugate = mat3(
		cross(M_t[1], M_t[2]),
		cross(M_t[2], M_t[1]),
		cross(M_t[0], M_t[1]));
	return adjugate / det;
}

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

	fragmentArea = length(dp1) * length(dp2);

	float invmax = inversesqrt(max(dot(T,T), dot(B,B)));
	return mat3(T * invmax, B * invmax, N);
}

vec3 PerturbNormal(vec3 N, vec3 V, vec2 texcoord)
{
	vec3 map = 2.0 * texture(map_normal, texcoord).rgb - 1.0;
	map.z *= BumpinessFactor;
	//vec3 map = texture(map_normal, texcoord).rgb;
	mat3 TBN = MakeCotangentFrame(N, -V, texcoord);
	return normalize(TBN * map);
}


// Physically Based Lighting Code ------------------------------------


float G2_Smith(float lambda)
{
	return 1.0 / (1.0 + lambda);
}


float erf(float x)
{

	float x2 = x * x;
	float ax2 = 0.140012 * x2;
	float erf_result = sqrt(1.0 - exp(-x2 * (1.2732395 + ax2) / (1.0 + ax2)));
	if (x < 0.0) return -erf_result;
	return erf_result;
}


float Lambda_GGX(float alpha_s, float NdotH)
{
	// float a = 1.0 / (alpha_s * tan(theta_o));
	float NdotH2 = NdotH * NdotH;
	float a =  NdotH / (alpha_s * (1.0 - sqrt(NdotH)));
	if (NdotH > 0.0) return 0.5 * (-1.0 + sqrt(1.0 + 1.0 / (a*a)));
	return 0.0;
}


float Lambda_Beckmann(float alpha_s, float NdotH)
{
	// float a = 1.0 / (alpha_s * tan(theta_o));
	float NdotH2 = NdotH * NdotH;
	float a =  NdotH / (alpha_s * (1.0 - sqrt(NdotH)));
	if (NdotH > 0.0) return 0.5 * (erf(a) - 1.0) + 1.0 / (3.54491 * a) * exp(-a*a);
	return 0.0;
}


float G2_GGX_old(float NdotV, float NdotL)
{
	if (NdotV > 0.0 && NdotL > 0.0)
		return 1.0 / (1.0 + G2_Smith(Lambda_GGX(PBKsm, NdotV)) + G2_Smith(Lambda_GGX(PBKsm, NdotL)));
	return 0.0;
}


float G2_Beckmann(float NdotV, float NdotL)
{
	if (NdotV > 0.0 && NdotL > 0.0)
		return 1.0 / (1.0 + G2_Smith(Lambda_Beckmann(PBKsm, NdotV)) + G2_Smith(Lambda_Beckmann(PBKsm, NdotL)));
	return 0.0;
}


float G1(float NdotL, float NdotV, float NdotH, float VdotH)
{
	float x = (2.0 * NdotH) / NdotH; // I had VdotH?
	return min(min(1.0, x * NdotL), x * NdotV);
}


float G(float NdotL, float NdotV, float NdotH, float VdotH)
{
	float G1 = 2.0 * NdotH * NdotV / VdotH;
	float G2 = 2.0 * NdotH * NdotL / VdotH;
	return min(1.0, min(G1, G2));

	// float t1 = (NdotH / VdotH) / (2.0 * NdotL);
	// float t2 = (NdotH / VdotH) / (2.0 * NdotV);

	// float t1 = NdotH / (2.0 * VdotH * NdotL);
	// float t2 = NdotH / (2.0 * VdotH * NdotV);

	// min(1.0, min(2.0 * NdotH * NdotV / VdotH, 2.0 * NdotH * NdotL / VdotH)) / (4.0 * NdotV * NdotL);
}


float G1_GGX(float c, float aSquared)
{
	return 2.0 / (1.0 + sqrt(1.0 + aSquared * (1.0 - c*c) / (c*c)));
}


float G2_GGX(int i)
{	
	float t = Material.specularRoughness * 0.5 + 0.5;
	float alphaSquared = t*t;//Material.specularRoughness2;//t*t;
	return G1_GGX(Lights[i].NdotL, alphaSquared) * G1_GGX(Lights[i].NdotV, alphaSquared);
}


float FresnelDielectric(in float n_1, in float n_2, in float cos_theta_i, in float sin_theta_i)
{
	float n1c = n_1 * cos_theta_i;
	float n2c = n_2 * cos_theta_i;
	float n1n2s = pow(1.0 - pow((n_1 / n_2) * sin_theta_i, 2.0), 0.5);
	float n1s = n_1 * n1n2s;
	float n2s = n_2 * n1n2s;

	float Rs = 1.0;
	float Rp = 1.0;
	float den = n1c + n1s;
	Rs = pow((n1c - n2s)/(n1c + n2s), 2.0);
	den = n1s + n1c;
	Rp = pow((n1s - n2c)/(n1s + n2c), 2.0);
	float R = (Rs + Rp) * 0.5;

	return clamp(R, 0.0, 1.0);
}

float FresnelMetal(in float n, in float k, in float cos_theta_i)
{
	float Rs;
	float Rp;
	float n2 = n*n;
	float k2 = k*k;
	float cos2 = cos_theta_i * cos_theta_i;
	float n2k2cos2 = (n2 + k2) * cos2;
	float n2cos = 2.0 * n * cos_theta_i;
	float n_minus_cos_2 = (n - cos_theta_i) * (n - cos_theta_i);
	float n_plus_cos_2 = (n + cos_theta_i) * (n + cos_theta_i);


	Rs = (n_minus_cos_2 + k2) / (n_plus_cos_2 + k2);
	Rp = (n2k2cos2 - n2cos + 1.0) / (n2k2cos2 + n2cos + 1.0);

	float R = (Rs + Rp) * 0.5;
	return clamp(R, 0.0, 1.0);
}


float FresnelMetal2(in float n_1, in float n_2, in float k_2, in float cos_theta_i, in float sin_theta_i)
{
	if (cos_theta_i <= 0.0) return 1.0;
	float k_2Squared = k_2 * k_2;
	if (cos_theta_i >= 1.0) {
		float t1 = n_1 - n_2;
		float t2 = n_1 + n_2;
		return (t1*t1 + k_2Squared) / (t2*t2 + k_2Squared);
	}
	float n_2Squared = n_2 * n_2;
	float n_1Squared = n_1 * n_1;
	float NcrossLSquared = sin_theta_i * sin_theta_i;
	float a = n_2Squared - k_2Squared - n_1Squared * NcrossLSquared;
	float aSquared = a * a;
	float b = 4.0 * n_2Squared * k_2Squared;
	float c = sqrt(aSquared + b);
	float p2 = 0.5 * (c + a);
	float p = sqrt(p2);
	float q2 = 0.5 * (c - a);
	float d1 = n_1 * cos_theta_i - p;
	float d2 = n_1 * cos_theta_i + p;
	float rho_perp = (d1*d1 + q2)/(d2*d2 + q2);
	float e1 = p - n_1 * (1.0/cos_theta_i - cos_theta_i);
	float e2 = p + n_1 * (1.0/cos_theta_i - cos_theta_i);
	float rho_parl = rho_perp * (e1*e1 + q2)/(e2*e2 + q2);

	float R = (rho_perp + rho_parl) * 0.5;
	return clamp(R, 0.0, 1.0);
}


vec3 FresnelMetalSchlick(in vec3 n, in vec3 k, in float cos_theta_i)
{
	vec3 t1 = (1.0 - n) * (1.0 - n);
	vec3 t2 = (1.0 + n) * (1.0 + n);
	vec3 k2 = k*k;
	vec3 F0 = (t1 + k2) / (t2 + k2);
	return F0 + (1.0 - cos_theta_i) * pow(cos_theta_i, 5.0);
}


vec3 FresnelMetalApprox(in vec3 n, in vec3 k, in float cos_theta_i)
{
	vec3 Rs;
	vec3 Rp;
	vec3 n2 = n*n;
	vec3 k2 = k*k;
	float cos2 = cos_theta_i * cos_theta_i;
	vec3 n2k2cos2 = (n2 + k2) * cos2;
	vec3 n2cos = 2.0 * n * cos_theta_i;
	vec3 t1 = (n - cos_theta_i);
	vec3 t2 = (n + cos_theta_i);
	vec3 n_minus_cos_2 = t1 * t1;
	vec3 n_plus_cos_2 = t2 * t2;

	Rs = (n_minus_cos_2 + k2) / (n_plus_cos_2 + k2);
	Rp = (n2k2cos2 - n2cos + 1.0) / (n2k2cos2 + n2cos + 1.0);

	return (Rs + Rp) * 0.5;
}


vec3 FresnelMetal3(in vec3 n, in vec3 k, in float cos_theta_i)
{
	if (cos_theta_i <= 0.0) return vec3(1.0, 1.0, 1.0);
	if (cos_theta_i >= 1.0) {
		vec3 t1 = 1.0 - n;
		vec3 t2 = 1.0 + n;
		vec3 k2 = k*k;
		return (t1*t1 + k2) / (t2*t2 + k2);
	}

	float cos2 = cos_theta_i * cos_theta_i;
	float sintan = (1.0 - cos2) / cos_theta_i;
	float sin2 = 1.0 - cos2;
	vec3 n2 = n*n;
	vec3 k2 = k*k;
	vec3 a = n2 - k2 + vec3(1.0, 1.0, 1.0) * sin2;
	vec3 t1 = a*a + 4.0 * n2 * k2;
	vec3 t2 = sqrt(t1);
	vec3 p2 = 0.5 * (t2 + a);
	vec3 p = sqrt(p2);
	vec3 q2 = t2 - a;
	vec3 t3 = p - sintan;
	vec3 t4 = p + sintan;	
	vec3 t5 = cos_theta_i - p;
	vec3 t6 = cos_theta_i + p;
	vec3 rho_perp = (t5*t5 + q2) / (t6*t6 + q2);
	vec3 rho_parl = (t3*t3 + q2) / (t4*t4 + q2) * rho_perp;

	return 0.5 * (rho_perp + rho_parl);
}


float ComputeBlinnPhong(float NdotH, float power)
{
	//if (NdotH >= 0.999) return 1.0;
	float t = pow(NdotH, power);
	return (power + 2.0) / 6.283185 * t;
}


float ComputeCookTorrance(vec3 N, vec3 L, vec3 V, vec3 H, float F, float roughness)
{
	// if roughness is 0, then it is only going to return 1 if N == L == V
	float m2 = roughness * roughness;
	if (m2 < 0.0) {
		return 0.0;
	}
	// float NdotH = max(0.0, dot(N, H)); // use small epsilon to keep nan error from happening
	// float NdotV = max(0.0, dot(N, V));
	// float NdotL = max(0.0, dot(N, L));
	// float VdotH = max(0.0, dot(V, H));
	float NdotL = max(0.0, dot(N, L));
	float NdotH = dot(N, H);
	float NdotV = dot(N, V);
	float VdotH = dot(V, H);

	if (NdotL == 0.0)
		return 0.0;
	if (NdotV == 0.0 && NdotL == 0.0)
		return 1.0;

	float costheta = NdotL;
	float theta = acos(costheta);
	float sintheta = sin(theta);
	float cosalpha = NdotH;
	float alpha = acos(cosalpha);
	float tan2alpha = pow(tan(alpha), 2.0);
	float cos2alpha = NdotH * NdotH;

	float gaussian = 10000.0 * exp(-NdotH*NdotH / m2);
	// tan2alpha / m2 = ((1.0 - cos2alpha)/(cos2alpha * m2))
	//float beckmann = exp(-((1.0 - cos2alpha)/(cos2alpha * m2))) / (pi * m2 * cos2alpha * cos2alpha);

	// These two are exactly the same...
	//float beckmann = 1.0 / (3.14159 * m2 * cos2alpha*cos2alpha) * exp(-tan2alpha/m2);
	float beckmann = 1.0 / (3.14159 * m2 * cos2alpha*cos2alpha) * exp((cos2alpha-1.0)/(cos2alpha*m2));
	float pixarD = 100.0 * exp(-(alpha*alpha)/m2);

	float fKs = 0.0;

	float D = beckmann;
	float G = min(1.0, min(2.0*NdotH*NdotV/VdotH, 2.0*NdotH*NdotL/VdotH));
	float Ks_den = 4.0 * 3.14159 * NdotV * NdotL;
	fKs = D * F * G / Ks_den;

	// A BRDF with a lower value than 0 is not physically plausible
	return max(0.0, fKs);
}


float ComputeGGX(vec3 N, vec3 L, vec3 V, vec3 H, float roughness)
{
	// if roughness is 0, then it is only going to return 1 if N == L == V
	float m2 = roughness * roughness;
	if (m2 < 0.0) {
		return 0.0;
	}

	float NdotL = max(0.0, dot(N, L));
	float NdotL2 = NdotL * NdotL;
	float NdotH = dot(N, H);
	float NdotH2 = NdotH * NdotH;
	float NdotV = dot(N, V);
	float NdotV2 = NdotV * NdotV;
	float VdotH = dot(V, H);

	if (NdotL == 0.0)
		return 0.0;
	if (NdotV == 0.0 && NdotL == 0.0)
		return 1.0;

	float fKs = 0.0;

	// IF(Chi([@NdotH]) > 0, alpha2 / (PI() * POWER(POWER([@NdotH], 2) * (alpha2 - 1) + 1, 2)), 0)
	float D = 0.0;
	if (NdotH > 0.0) {
		float d = NdotH2 * (m2 - 1.0) + 1.0;
		D = m2 / (d * d);
	}

	// =1 / (1 + (-1 / 2 + 1 / 2 * SQRT(alpha2 / POWER(NdotL,2) - alpha2 + 1)) + -1 / 2 + 1 / 2 * SQRT(alpha2 / POWER([@NdotV],2) - alpha2 + 1))
	float Lambda_wi = sqrt(NdotL2 - m2 * (NdotL2 - 1.0)); // sqrt(m2 / NdotL2 - m2 + 1.0);
	float Lambda_wo = sqrt(NdotV2 - m2 * (NdotV2 - 1.0)); // sqrt(m2 / NdotV2 - m2 + 1.0);
	// 1 / (2*Pi^2) = 0.050661
	float G = 0.050661 / (Lambda_wi + Lambda_wo);

	fKs = D * G;

	// A BRDF with a lower value than 0 is not physically plausible
	return max(0.0, fKs);
}


float ComputeOrenNayer(vec3 N, vec3 L, vec3 V, float m)
{
	float NdotL = max(dot(N, L), 0.0);
	if (m <= 0.0) return NdotL;
	if (NdotL == 0.0) return 0.0;

	float NdotV = max(dot(N, V), 0.0);
	float theta_NL = acos(NdotL);
	float theta_NV = acos(NdotV);

	float alpha = max(theta_NV, theta_NL);
	float beta = min(theta_NV, theta_NL);

	float gamma = max(dot(V - N * NdotV, L - N * NdotV), 0.0);
	float m2 = m*m;

	float A = 1.0 - 0.5 * m2 / (m2 + 0.57);
	float B = 0.45 * m2 / (m2 + 0.09);
	float C = sin(alpha) * tan(beta);
	float L1 = NdotL * (A + B * gamma * C);
	return L1;
}


float ComputeOrenNayer2(int i)
{
	// According to Disney BRDF, some models use double Fresnel in this way
	// float cos_theta_d = dot(Lights[i].L, Lights[i].H);
	// float Fl = ComputeFresnelSchlick2(Material.F0, Lights[i].NdotL);
	// float Fd = ComputeFresnelSchlick2(Material.F0, cos_theta_d);
	// Oren-Nayer BRDF

	// (vec3 N, vec3 L, vec3 V, float m
	float theta_NL = acos(Lights[i].NdotL);
	float theta_NV = acos(Lights[i].NdotV);

	float alpha = max(theta_NV, theta_NL);
	float beta = min(theta_NV, theta_NL);

	float gamma = max(dot(Fragment.V - Fragment.N * Lights[i].NdotV, Lights[i].L - Fragment.N * Lights[i].NdotV), 0.0);
	float m2 = Material.diffuseRoughness * Material.diffuseRoughness;

	float A = 1.0 - 0.5 * m2 / (m2 + 0.57);
	float B = 0.45 * m2 / (m2 + 0.09);
	float C = sin(alpha) * tan(beta);
	float L1 = (A + B * gamma * C) / 3.14159;
	return L1;
}


float ComputeDisneyDiffuse(int i)
{
	// Disney Diffuse BRDF
	float cos_theta_d = dot(Lights[i].L, Lights[i].H);
	// Disney BRDF uses 0.5 + 2.0 * ...		
	float FD90 = 0.5 + 2.0 * cos_theta_d * cos_theta_d * -PBKdm;
	// But I'm allowing for the original NdotL as well...
	// float FD90 = 0.0 + 2.5 * cos_theta_d * cos_theta_d * -PBKdm;
	float t = FD90 - 1.0;
	float c1 = pow(1.0 - Lights[i].NdotL, 5.0);
	float c2 = pow(1.0 - Lights[i].NdotV, 5.0);
	return (1.0 + t * c1) * (1.0 + t * c2) / 3.14159;	
}


vec3 ComputePBDiffuse(vec3 N, vec3 L, vec3 V, vec3 myKd, float T, float E0)
{
	float NdotL = max(dot(N, L), 0.0);
	if (NdotL == 0.0) return vec3(0.0, 0.0, 0.0);

	float lambert = 0.0;
	// if not metal...
	if (PBk > 0.0)
		lambert = NdotL;
	else
	if (PBKdm > 0.0)
		lambert = ComputeOrenNayer(N, L, V, PBKdm);
	else
		lambert = NdotL;
	
	float fKd = T * invPi * lambert;

	return E0 * fKd * myKd;
}


float ComputePBDiffuse2(vec3 N, vec3 L, vec3 V, float T)
{
	float NdotL = dot(N, L);
	if (NdotL <= 0.0) return 0.0;

	float lambert = 0.0;
	// if not metal...
	if (PBk > 0.0)
		lambert = NdotL;
	else
	if (PBKdm > 0.0)
		lambert = ComputeOrenNayer(N, L, V, PBKdm);
	else
		lambert = NdotL;
	
	float fKd = T * invPi * lambert;

	return fKd;
}


float ComputeE0(float Ldistance)
{
	float Ld2 = Ldistance * Ldistance;
	float E0;
	if (Ldistance > 0.0)
		E0 = 15.0 * 4.0 * pi / Ld2;
	else
		E0 = 2.0;
	return E0;
}


float ComputeFresnel(vec3 N, vec3 L, vec3 V, vec3 H, float ior, float k)
{
	// indices of refraction
	float n_1 = 1.0001; // air
	float n_2 = 1.0001; // water

	if (ior > n_1) n_2 = ior;

	float NdotH = dot(N, H);//max(0.0, dot(N, H)); // use small epsilon to keep nan error from happening
	float NdotV = dot(V, N);//max(0.0, dot(V, N));
	float NdotL = max(0.0, dot(L, N));
	float VdotH = dot(V, H);//max(0.0, dot(V, H));
	float NcrossL = length(cross(N,L));

	//float sin_theta_i = sin(acos(VdotH));
	float sin_theta_i = length(cross(V, H));

	// Fresnel -------------------------
	float F = 0.0;

	float R_0 = (n_1 - n_2)/(n_1 + n_2); R_0 = R_0 * R_0; // for schlick's approximation
	float schlicks = R_0 + (1.0 - R_0) * pow(1.0 - VdotH, 5.0);
	float fresnel = FresnelDielectric(n_1, n_2, VdotH, sin_theta_i);
	float fresnelFace = FresnelDielectric(n_1, n_2, NdotL, sin(acos(NdotL)));

	if (k > 0.0)
		//F = FresnelMetal(n_1, k, VdotH);
		F = FresnelMetal2(n_1, n_2, k, NdotL, NcrossL);
	else
		//F = schlicks;
		F = fresnel;

	return F;
}


float ComputeFresnelSchlick(float ior, float NdotH)
{
	if (ior <= 1.0)
		ior = 1.52;
	float t = (1.0 - ior) / (1.0 + ior);
	float F0 = t * t;	
	return F0 + (1.0 - F0) * pow(max(0.0, 1.0 - NdotH), 5.0);
}


float ComputeFresnelSchlick2(float F0, float cos_theta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}


vec3 ComputePBMetalSpecular(vec3 N, vec3 L, vec3 V, vec3 myKs, float m, vec3 F, float E0)
{
	vec3 H = normalize(V + L);
	float NdotH = max(dot(N,H), epsilon);

	vec3 fKs;
	float power = Ns < 1.0 ? 1.0 : Ns;
	if (m > 0.0)
		fKs = vec3(
			ComputeCookTorrance(N, L, V, H, F.r, m),
			ComputeCookTorrance(N, L, V, H, F.g, m),
			ComputeCookTorrance(N, L, V, H, F.b, m)
			);
	else
		fKs = F * ComputeBlinnPhong(NdotH, power);

	return E0 * fKs * myKs;
}


vec3 ComputePBMetalSpecular2(vec3 N, vec3 L, vec3 V, float m, vec3 F)
{
	vec3 H = normalize(V + L);
	vec3 fKs;
	
	if (m > 0.0) {
		float ct = ComputeCookTorrance(N, L, V, H, 1.0, m);
		fKs = vec3(
			F.r * ct,
			F.g * ct,
			F.b * ct
			);
	}
	else {
		float power = Ns < 1.0 ? 1.0 : Ns;
		float NdotH = max(dot(N,H), epsilon);
		fKs = F * ComputeBlinnPhong(NdotH, power);
	}

	return fKs;
}


vec3 ComputePBMetalSpecular_GGX(vec3 N, vec3 L, vec3 V, float m, vec3 F)
{
	vec3 H = normalize(V + L);
	vec3 fKs;
	
	if (m > 0.0) {
		float ct = ComputeGGX(N, L, V, H, m);
		fKs = vec3(
			F.r * ct,
			F.g * ct,
			F.b * ct
			);
	}
	else {
		float power = Ns < 1.0 ? 1.0 : Ns;
		float NdotH = max(dot(N,H), epsilon);
		fKs = F * ComputeBlinnPhong(NdotH, power);
	}

	return fKs;
}


vec3 ComputePBSpecular(vec3 N, vec3 L, vec3 V, vec3 myKs, float m, float F, float E0)
{
	vec3 H = normalize(V + L);
	float NdotH = max(dot(N,H), 0.0);

	float fKs = 0.0;
	float power = 2.0 / (m*m) + 2.0;
	if (m > 0.0)
		fKs = ComputeCookTorrance(N, L, V, H, F, m);
	else
		fKs = ComputeBlinnPhong(NdotH, power);//ComputeCookTorrance(N, L, V, H, F, 0.99);//ComputeBlinnPhong(NdotH, power);

	return E0 * fKs * myKs;
}


float ComputePBSpecular2(vec3 N, vec3 L, vec3 V, vec3 H, float m, float F)
{
	float NdotH = dot(N, H);
	float VdotH = dot(V, H);
	float NdotL = dot(N, L);
	float NdotV = dot(N, V);

	if (NdotL <= 0.0)
		return 0.0;
	if (NdotH >= 0.99)
		return F;

	float fKs = 0.0;
	if (m > 0.0)
	{
		fKs = ComputeCookTorrance(N, L, V, H, F, m);
	}
	else
	{
		// we add 0.0001 so that we don't get division by 0
		float power = 2.0 / (m*m+0.0001) + 2.0;
		fKs = F * G(NdotL, NdotV, NdotH, VdotH) / (4.0 * NdotV * NdotL) * ComputeBlinnPhong(NdotH, power);
	}

	return fKs;
}


vec3 ComputePBLighting(vec3 N, vec3 L, vec3 V, vec3 H, vec3 myKd, vec3 myKs, vec3 reflectColor, float E0)
{
	vec3 finalColor = vec3(1.0,1.0,1.0);

	// Constants
	// roughness
	/*
	float m2 = 0.95;

	if (PBKsm > 0)
		m2 = PBKsm * PBKsm;

	// indices of refraction
	float n_1 = 1.0001; // air
	float n_2 = 1.0001; // water

	if (PBior > n_1) n_2 = PBior;

	float NdotH = max(epsilon, dot(N, H)); // use small epsilon to keep nan error from happening
	float NdotV = max(epsilon, dot(V, N));
	float NdotL = max(epsilon, dot(L, N));
	float VdotH = max(epsilon, dot(V, H));

	// Fresnel -------------------------
	/*
	float F = 0;

	float R_0 = (n_1 - n_2)/(n_1 + n_2); R_0 = R_0 * R_0; // for schlick's approximation
	float schlicks = R_0 + (1.0 - R_0) * pow(1.0 - VdotH, 5.0);
	float fresnel = FresnelDielectric(n_1, n_2, VdotH, sin(acos(VdotH)));
	float fresnelFace = FresnelDielectric(n_1, n_2, NdotL, sin(acos(NdotL)));
	float fresnelmetal = FresnelMetal(n_1, 5, VdotH);

	if (PBk > 0)
		F = fresnelmetal;
	else
		F = schlicks;
	*/

	// T is the amount of transmitted light
	float F = ComputeFresnel(N, L, V, H, 1.33, 0.0);
	float T = 1.0 - F;

	vec3 diffuseColor = ComputePBDiffuse(N, L, V, myKd, T, E0);

	//vec3 wo = V;
	//vec3 wi = L;
	//vec3 wh = V + L;
	//vec wg = N;
	//float wm_dot_wg;
	//float wo_dot_wg;
	//float wo_dot_wm;

	float fKs = 0.0;
	float power = Ns < 1.0 ? 1.0 : Ns;
	if (PBk > 0.0)
		fKs = ComputeCookTorrance(N, L, V, H, F, PBm);
	if (PBKsm > 0.0)
		fKs = ComputeCookTorrance(N, L, V, H, F, PBKsm);
	else
		fKs = ComputeCookTorrance(N, L, V, H, F, 0.99);//ComputeBlinnPhong(NdotH, power);

	vec3 specularColor = E0 * fKs * myKs;

	return specularColor + diffuseColor;
}


// -------------------------------------------------------------------


// Main Function
void main(void)
{
	if (ToneMapScale > 0.0)
		toneMapScale = ToneMapScale;
	DoDeferredMain();
	DoDeferredLightAccum();

	DeferredLightBuffer = max(vec3(0.0), DeferredLightBuffer);

	// exposure and gamma
	vec3 finalColor = DeferredLightBuffer * ToneMapExposure;
	finalColor = pow(finalColor, vec3(1.0 / ToneMapGamma));	

	// vec3 finalColor = DeferredLightBuffer * ToneMapScale;

	FS_outputColor = vec4(finalColor, 1.0);
}


void DoDeferredMainOld()
{
	PrepareForShading();
	PrepareMaterial();
}


vec4 ComputePB(in vec3 N, in vec3 L, in vec3 V, in vec3 H, in vec3 E0, in vec3 c_d, in bool specularOnly)
{
	float VdotH = max(0.0, dot(V, H));
	float NdotL = max(0.0, dot(N, L));
	float NdotH = dot(N, H);
	float ambientFactor = 1.0;
	if (NdotL <= 0.0) ambientFactor = 1.0 / (0.5*3.14159);
	float Ks = 0.0;
	float Kd = 1.0;
	vec4 specularColor;
	vec4 diffuseColor;
	if (PBk > 0.0) {
		float spec_cos_theta_i = NdotH;
		float spec_sin_theta_i = sin(acos(spec_cos_theta_i));

		vec3 pb_metalSpecularF = vec3(
			FresnelMetal(Metal_N2.r, Metal_K2.r, spec_cos_theta_i),
			FresnelMetal(Metal_N2.g, Metal_K2.g, spec_cos_theta_i),
			FresnelMetal(Metal_N2.b, Metal_K2.b, spec_cos_theta_i)
			);
		vec3 metalDiffuseColor = vec3(
			FresnelMetal(Metal_N2.r, Metal_K2.r, 0.0),
			FresnelMetal(Metal_N2.g, Metal_K2.g, 0.0),
			FresnelMetal(Metal_N2.b, Metal_K2.b, 0.0)
			);
		vec3 metalKs = ComputePBMetalSpecular2(N, L, V, PBKsm, pb_metalSpecularF);
		float metalKd = PBKdm;

		specularColor = vec4(E0.rgb * metalKs.rgb, 1.0);
		//specularColor = vec4(ComputePBMetalSpecular(N, skyboxL, V, skyboxTexel, 0.0, pb_metalSpecularF, 1.0), 1.0);
		//diffuseColor = vec4(sunE0.rgb * DeferredPB3.rgb * (1.0 - pb_metalSpecularF) * max(0.0, dot(N, skyboxL)), 0.0);// vec4(sunE0.rgb * ComputePBDiffuse(N, L, V, DeferredPB3.rgb, skyboxT, 1.0), 1.0);
		// diffuseColor = vec4(sunE0.rgb * DeferredPB3.rgb * (1.0 - pb_metalSpecularF) * ComputePBDiffuse2(N, L, V, skyboxKd), 1.0);
		// diffuseColor = vec4(E0.rgb * metalDiffuseColor.rgb * metalKd, 1.0);
	}
	else {
		float F;
		if (PBKsm > 0.0)
			F = ComputeFresnel(N, L, V, H, PBior, PBk);
		else
			F = ComputeFresnelSchlick(PBior, NdotH);
		Ks = ComputePBSpecular2(N, L, V, H, PBKsm, F);
		Kd = 1.0;// - Ks;
		specularColor = vec4(Ks * E0.rgb, 1.0);
		if (specularOnly == false)
			diffuseColor = vec4(E0.rgb * c_d.rgb * ComputePBDiffuse2(N, L, V, Kd), 1.0);
	}

	return ambientFactor * (specularColor + diffuseColor);
}


vec4 ComputePB_GRAPP2017(in vec3 N, in vec3 L, in vec3 V, in vec3 H, in vec3 c_s, in vec3 c_d, in bool specularOnly)
{
	float VdotH = max(0.0, dot(V, H));
	float NdotL = max(0.0, dot(N, L));
	float NdotH = dot(N, H);
	float NdotV = dot(N, V);
	float ambientFactor = 1.0;
	if (NdotL <= 0.0) ambientFactor = 2.0 / 3.14159;
	vec4 specularColor;
	vec4 diffuseColor = vec4(0.0, 0.0, 0.0, 0.0);
	if (NdotL <= 0.0) {
		return vec4(0.0, 0.0, 0.0, 0.0);
	}
	else if (PBk > 0.0) {
		vec3 metalKs;
		if (metalFresnelType == 1)
			metalKs = ComputePBMetalSpecular_GGX(N, L, V, PBKsm, FresnelMetal3(Metal_N2, Metal_K2, NdotH));
		else if (metalFresnelType == 2)
			metalKs = ComputePBMetalSpecular_GGX(N, L, V, PBKsm, FresnelMetalApprox(Metal_N2, Metal_K2, NdotH));
		else if (metalFresnelType == 3)
			metalKs = ComputePBMetalSpecular_GGX(N, L, V, PBKsm, FresnelMetalSchlick(Metal_N2, Metal_K2, NdotH));
		float metalKd = PBKdm;

		specularColor = vec4(c_s.rgb * metalKs.rgb, 1.0);
	}
	else {
		//float F = ComputeFresnel(N, L, V, H, PBior, PBk);
		float F;
		if (PBKsm > 0.0)
			F = ComputeFresnel(N, L, V, H, PBior, PBk);
		else
			F = ComputeFresnelSchlick(PBior, NdotH);
		specularColor = vec4(c_s * ComputePBSpecular2(N, L, V, H, PBKsm, F), 1.0);
		if (specularOnly == false)
			diffuseColor = vec4(c_d * ComputePBDiffuse2(N, L, V, 1.0), 1.0);
	}

	return ambientFactor * (specularColor + diffuseColor);
}


vec3 GetVectorColor(vec3 V)
{
	vec3 result = 0.5 * V + 0.5;
	if (V.x < 0.0)
		result.x = 0.5 - result.x;
	if (V.y < 0.0)
		result.y = 0.5 - result.y;
	if (V.z < 0.0)
		result.z = 0.5 - result.z;
	return result;
}


void DoDeferredMain()
{
	PrepareForShading();
	PrepareMaterial();
}


void DoDeferredLightAccum()
{
	// Debug Choices
	// - 0  - Do normal rendering pipeline
	// - 1  - Diffuse Color
	// - 2  - Specular Color
	// - 3  - Depth (log)
	// - 4  - Normal Map
	// - 5  - Face Normals
	// - 6  - View Normals
	// - 7  - Metal Fresnel 1
	// - 8  - Metal Fresnel 2
	// - 9  - Oren-Nayer
	// - 10 - Physically Based Shader
	// - 11 - SPH Shader
	// - 12 - 
	// - 13 - 
	// - 14 - Phong
	// - 15 - Blinn Cook-Torrance
	// - 16 - 
	// - 17 - Oren-Nayer
	// - 18 - Metal Fresnel 1
	// - 19 - Metal Fresnel 2
	// - 20 - Dielectric Fresnel
	
	// G-Buffer Inputs
	// DeferredPB1 = vec4(VS_Position.xyz, 1.0);
	// DeferredPB2 = vec4(Nmap.xyz, 1.0);
	// DeferredPB3 = vec4(Material.Kd.rgb, 1.0);
	// DeferredPB4 = vec4(Material.Ks.rgb, 1.0);
	// DeferredPB5 = vec4(N.xyz, 1.0);
	// DeferredPB6 = vec4(V.xyz, 1.0);
	int debugChoice = ShaderDebugChoice == 0 ? 15 : ShaderDebugChoice;
	int dLi = ShaderDebugSphl < 0 ? SunIndex : ShaderDebugSphl;
	if (ShaderDebugSphl >= 0)
	{
		firstLightIndex = ShaderDebugSphl;
		lastLightIndex = ShaderDebugSphl;
	}
	else
	{
		firstLightIndex = 0;
		lastLightIndex = SunIndex;
	}

	if (debugChoice == 0)
	{
	}
	else if (debugChoice == 1)
	{
		// Set the pixel color from the Material kd color
		DeferredLightBuffer += Material.Kd;
	}
	else if (debugChoice == 2)
	{
		// Set the pixel color from the material ks color
		DeferredLightBuffer += Material.Ks;
	}
	else if (debugChoice == 3)
	{
		// Set the pixel color from the face normals
		DeferredLightBuffer += 0.5 * Fragment.N + 0.5;
	}
	else if (debugChoice == 4)
	{
		// Set the pixel color from the view direction
		DeferredLightBuffer += 0.5 * Fragment.V + 0.5;
	}
	else if (debugChoice == 5)
	{
		DeferredLightBuffer += 0.5 * Fragment.R + 0.5;
	}
	else if (debugChoice == 6)
	{
		// Set the pixel color from the light vector L
		PrepareLight(dLi);
		DeferredLightBuffer += 0.5 * Lights[dLi].L + 0.5;
	}
	else if (debugChoice == 7)
	{
		// Set the pixel color from the half vector H
		PrepareLight(dLi);
		DeferredLightBuffer += 0.5 * Lights[dLi].H + 0.5;
	}
	else if (debugChoice == 8)
	{
		dLi = SunIndex;
		PrepareLight(dLi);
		DeferredLightBuffer += 1.0 / 3.14159 * Lights[dLi].NdotL * Lights[dLi].I;// / Lights[dLi].distance2;
	}
	else if (debugChoice == 9)
	{
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{			
			// Set the pixel color to basic N dot L lighting
			if (!PrepareLight(i))
				continue;

			float E0 = 1.0 / Lights[i].distance2;

			float brdf_d = GetMobileDiffuseBRDF(i);
			float brdf_s = GetMobileSpecularBRDF(i);

			float shadow = Lights[i].shadow;

			vec3 c_d = brdf_d * Material.Kd;
			vec3 c_s = brdf_s * Material.Ks;

			DeferredLightBuffer += E0 * Lights[i].I * shadow * (c_d + c_s) * Lights[i].NdotL;
		}
	}
	else if (debugChoice == 10)
	{
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (!PrepareLight(i))
				continue;

			float E0 = 1.0 / Lights[i].distance2;

			float brdf_d = GetMobileDiffuseBRDF(i);
			float brdf_s = GetMobileSpecularBRDF(i);

			float shadow = Lights[i].shadow;
			vec3 c_d = brdf_d * Material.Kd;
			vec3 c_s = brdf_s * Material.Ks;
			vec3 c_e = GetEnviroColor(i) * GetPbrEnviroBrdfFresnel(i);

			DeferredLightBuffer += E0 * Lights[i].I * shadow * (c_d + c_s) * Lights[i].NdotL + c_e * Fragment.NdotV * Material.Ks;
		}
	}
	else if (debugChoice == 11)
	{
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (!PrepareLight(i))
				continue;

			vec3 c_d = GetPbrBrdfDiffuse(i);
			

			if (i == SunIndex)
			{
				float shadow = Lights[i].shadow;
				DeferredLightBuffer += Lights[i].I * shadow * c_d * Lights[i].NdotL;
			}
			else
			{
				DeferredLightBuffer += Lights[i].I * c_d;	
			}
		}
	}
	else if (debugChoice == 12)
	{
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (!PrepareLight(i))
				continue;

			vec3 c_s = GetPbrBrdfSpecular(i);
			float shadow = Lights[i].shadow;

			DeferredLightBuffer += Lights[i].I * shadow * c_s * Lights[i].NdotL;
		}
	}
	else if (debugChoice == 13)
	{
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (!PrepareLight(i))
				continue;

			vec3 c_e = GetPbrEnviroBrdf(i);
			DeferredLightBuffer += c_e;
		}
	}
	else if (debugChoice == 14)
	{
		mobileMode = 0.0;
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (!PrepareLight(i))
				continue;

			vec3 c_d = GetPbrBrdfDiffuse(i);

			if (i == SunIndex)
			{
				float shadow = Lights[i].shadow;
				vec3 c_s = GetPbrBrdfSpecular(i);		
				DeferredLightBuffer += Lights[i].I * shadow * (c_d + c_s) * Lights[i].NdotL;
			}
			else
			{
				DeferredLightBuffer += Lights[i].I * c_d;
			}
		}		
	}
	else if (debugChoice == 15)
	{
		mobileMode = 0.0;
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (!PrepareLight(i))
				continue;

			float shadow = Lights[i].shadow;
			vec3 c_d = GetPbrBrdfDiffuse(i);
			vec3 c_s;
			vec3 c_e;
			if (shadow <= ShadowDarkness)
			{
				c_s = vec3(0.0);
				c_e = vec3(0.0);
			}
			else
			{
				c_s = GetPbrBrdfSpecular(i);
			}

			if (i != SunIndex)
			{
				vec3 c_e = GetPbrEnviroBrdf(i);
				DeferredLightBuffer += Lights[i].I * shadow * (c_d) + c_e;
			}
			else
			{
				DeferredLightBuffer += Lights[i].I * shadow * (c_d + c_s) * Lights[i].NdotL + c_e;
			}
		}		
	}
	else if (debugChoice == 16)
	{
		mobileMode = 0.0;
		bool lit = false;
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (i != SunIndex && lit == true) continue;

			if (!PrepareLight(i))
				continue;

			float shadow = i == SunIndex ? Lights[i].shadow : 1.0;

			vec3 c_d = GetPbrBrdfDiffuse(i);
			vec3 c_s;
			if (shadow <= ShadowDarkness)
			{
				c_s = vec3(0.0);		
			}
			else
			{
				if (i == SunIndex)
				{
					c_s = GetPbrBrdfSpecular(i);
				}
			}			

			if (i == SunIndex)
			{
				vec3 c_e = GetPbrEnviroBrdf(i);
				DeferredLightBuffer += Lights[i].I * shadow * (c_d + c_s) * Lights[i].NdotL;
				DeferredLightBuffer += c_e;
			}
			else
			{
				lit = true;
				DeferredLightBuffer += Lights[i].I * shadow * (c_d + c_s);
			}
		}
		//if (lastLightIndex == SunIndex) DeferredLightBuffer += VS_Ambient * Material.Kd;
	}	
	else if (debugChoice == 17)
	{
		mobileMode = 0.0;
		float bestShadow = 0.0;
		int bestShadowI = 0;
		vec3 bestShadow_c_d;
		bool lit = false;
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (i != SunIndex && lit == true) continue;
			if (!PrepareLight(i)) continue;

			vec3 c_d = GetPbrBrdfDiffuse(i);		

			if (i == SunIndex)
			{
				// float z = 1.0;
				// z = texture(SphlLights[0].DepthShadowCubeMap, Fragment.R).z;
				// if (z == 1.0)
				// {
				// 	vec3 I = GetPBSky(Fragment.V, 0.0).rgb;
				// 	DeferredLightBuffer += I * Material.Kd / 3.14159 * Fragment.NdotV;
				// }

				vec3 c_s = GetPbrBrdfSpecular(i);
				vec3 c_e = GetPbrEnviroBrdf(i) * Material.Ks;
				DeferredLightBuffer += Lights[i].I * Lights[i].shadow * (c_d + c_s) * Lights[i].NdotL;
				DeferredLightBuffer += c_e;				
			}
			else
			{
				vec3 c_e = GetPbrEnviroBrdf(i) * Material.Ks;
				DeferredLightBuffer += Lights[i].I * c_d;
				DeferredLightBuffer += c_e;
			}
		}
		if (lastLightIndex == SunIndex) DeferredLightBuffer += VS_Ambient * Material.Kd;
	}
	else if (debugChoice == 18)
	{
		mobileMode = 0.0;
		float bestShadow = 0.0;
		int bestShadowI = 0;
		vec3 bestShadow_c_d;
		bool lit = false;
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (i != SunIndex && lit == true) continue;
			if (!PrepareLight(i)) continue;

			vec3 c_d = GetPbrBrdfDiffuse(i);		

			if (i == SunIndex)
			{

				vec3 c_s = GetPbrBrdfSpecular(i);
				vec3 c_e = GetPbrEnviroBrdf(i) * Material.Ks;
				DeferredLightBuffer += Lights[i].I * Lights[i].shadow * (c_d + c_s) * Lights[i].NdotL;
				DeferredLightBuffer += c_e;				
			}
			else
			{
				lit = true;
				vec3 c_e = GetPbrEnviroBrdf(i) * Material.Ks;
				DeferredLightBuffer += Lights[i].I * c_d;
				DeferredLightBuffer += c_e;
			}
		}
		if (lastLightIndex == SunIndex) DeferredLightBuffer += VS_Ambient * Material.Kd;
	}
	else if (debugChoice == 18)
	{
		bool lit = false;
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (i == SunIndex || (!PrepareLight(i) && !lit))
				continue;

			float shadow = Lights[i].shadow;

			if (shadow >= ShadowDarkness) lit = true;
			vec3 c_d = GetPbrBrdfDiffuse(i);
			DeferredLightBuffer += Lights[i].I * 0.156;
		}		
	}
	else if (debugChoice == 19)
	{
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (!PrepareLight(i))
				continue;

			float shadow = i == SunIndex ? Lights[i].shadow : 1.0;
			vec3 c_d = GetPbrBrdfDiffuse(i);
			DeferredLightBuffer += Lights[i].I * shadow * 0.156;
		}		
	}
	else if (debugChoice == 20)
	{
		for (int i = firstLightIndex; i <= lastLightIndex; i++)
		{
			if (!PrepareLight(i))
				continue;

			float shadow = Lights[i].shadow;
			vec3 c_d = GetPbrBrdfDiffuse(i);
			DeferredLightBuffer += Lights[i].I * shadow * 0.156;// * (c_d);
		}		
	}
}
