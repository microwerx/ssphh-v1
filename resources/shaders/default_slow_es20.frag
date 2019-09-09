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
#version 100 es

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

uniform samplerCube PBSkyCubeMap;

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


// Outputs -----------------------------------------------------------


out vec4 FS_outputColor;


// Globals and Constants ---------------------------------------------


float pi = 3.1415926;
float invPi = 0.31831;
float epsilon = 0.00001;
float fragmentArea = 1.0;

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




// PBSky



// vec3 GetPBSky(vec3 L, highp float spread)
// {
// 	// // Sun illumination
// 	// highp float sunScaleFactor = 1.0 / length(SunE0.rgb);
// 	// highp float sunShadowR = GetSunShadow();

// 	// highp vec4 sunE0;
// 	// highp vec4 invSunE0;
// 	// if (SunDirTo.y < 0.0) {
// 	// 	sunScaleFactor = 0.0;
// 	// 	sunE0 = vec4(0.0, 0.0, 0.0, 0.0);
// 	// 	invSunE0 = vec4(0.0, 0.0, 0.0, 0.0);
// 	// }
// 	// else {
// 	// 	sunScaleFactor = 1.0;//1.0 / length(SunE0.rgb);
// 	// 	sunE0 = sunScaleFactor * SunE0;
// 	// 	invSunE0 = 1.0 / sunE0;
// 	// }

// 	bool inDisc = false;
// 	vec3 v = normalize(L);
// 	float cosine = dot(v, SunDirTo);
// 	float sunScaleFactor = 1.0;//1.0 / length(SunE0.rgb);
// 	vec4 sunE0 = sunScaleFactor * SunE0;
// 	bool atNight = false;

// 	if (SunDirTo.y >= 0.0) {
// 		atNight = false;
// 	} else {
// 		atNight = true;
// 	}

// 	if (cosine > 0.99999) {
// 		inDisc = true;
// 	} 

// 	vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);
// 	if (atNight) {	
// 		outputColor = vec4(0.0, 0.0, 0.0, 0.0);		
// 	}
// 	else {
// 		if (L.y >= 0.0) {
// 			if (inDisc) {
// 				outputColor += SunE0 * sunScaleFactor;					
// 			} else {
// 				outputColor += texture(PBSkyCubeMap, L);		
// 				if (spread > 0.0) {
// 					outputColor += texture(PBSkyCubeMap, L + vec3(spread, 0.0, 0.0));
// 					outputColor += texture(PBSkyCubeMap, L + vec3(-spread, 0.0, 0.0));
// 					outputColor += texture(PBSkyCubeMap, L + vec3(0.0, spread, 0.0));
// 					outputColor += texture(PBSkyCubeMap, L + vec3(0.0, -spread, 0.0));
// 					outputColor += texture(PBSkyCubeMap, L + vec3(0.0, 0.0, spread));
// 					outputColor += texture(PBSkyCubeMap, L + vec3(0.0, 0.0, -spread));
// 					outputColor = outputColor / 7.0;
// 				}	
// 				outputColor *= sunE0Length;					
// 			}
// 		}
// 		else {
// 			outputColor = sunScaleFactor * GroundE0;
// 		}
// 	}	
// 	return max(vec3(0.1, 0.1, 0.1), outputColor.rgb);
// }

vec4 GetPBSky(vec3 L, float spread)
{
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
				outputColor += sunE0;					
			}
			else {
				outputColor += texture(PBSkyCubeMap, L);// * whiteBalanceScaleFactor;
			}
		}
		else {
			outputColor += groundE0;// * invWhiteBalanceScaleFactor;
			//outputColor += textureCube(uCubeTexture, vec3(0.0, -1.0, 0.0));
		}
	}
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
	float offset = 0.0002;
	float biasW = bias / VS_SunShadowCoord.w;

	//if (SunSize > 0) amountInShadow = texture(SunShadowMap, VS_SunShadowCoord.xyz);
	vec3 sunCoord = vec3(VS_SunShadowCoord.xy, VS_SunShadowCoord.z) / VS_SunShadowCoord.w;

	vec3 sunTexMapColor = textureProj(SunColorMap, VS_SunShadowCoord.xyw).rgb;
	float sunZ = textureProj(SunShadowMap, VS_SunShadowCoord.xyw).z;

	float sunZsamples[17];
	sunZsamples[0] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, offset)).z ? 0.0 : 1.0;
	sunZsamples[1] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, offset)).z ? 0.0 : 1.0;
	sunZsamples[2] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, -offset)).z ? 0.0 : 1.0;
	sunZsamples[3] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, -offset)).z ? 0.0 : 1.0;
	sunZsamples[4] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, 0)).z ? 0.0 : 1.0;
	sunZsamples[5] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, 0)).z ? 0.0 : 1.0;
	sunZsamples[6] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(0, -offset)).z ? 0.0 : 1.0;
	sunZsamples[7] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(-0, -offset)).z ? 0.0 : 1.0;
	sunZsamples[8] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy).z ? 0.0 : 1.0;

	float sampleSum = 0.0;
	for (int i = 0; i < 9; i++) {
		sampleSum += sunZsamples[i];
	}
	sampleSum /= 9.0;

	//sunTexMapColor = sunCoord.xyz;	

	float shadowAmount = 1.0;
	if (sunCoord.z < 0.0 || sunCoord.z > 1.0)
		shadowAmount = 0.5;
	if ((sunCoord.z - biasW) >= sunZ)
		shadowAmount = 0.1;

	return max(0.5, sampleSum);
}


// Cube Shadow Mapping Code ------------------------------------------


// This returns the factor of shadow using one of the light Shadow Maps.
// If the lightId is not enabled, then return 1.0, completely shadowed,
// because the absence of light is darkness.
float GetLightShadow(int lightId)
{
	float amountInShadow = 0.0;
	return amountInShadow;
}


// Cube Environment Mapping Code


// This returns 
vec3 GetEnviroColor(vec3 direction)
{
	return vec3(0,0,0);
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


float G2_GGX(float NdotV, float NdotL)
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
	float x = (2.0 * NdotH) / VdotH;
	return min(min(1.0, x * NdotL), x * NdotV);
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
	return (power + 2.0) / (2.0 * 3.14159) * pow(max(0.0, NdotH), power);
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


float ComputePBSpecular2(vec3 N, vec3 L, vec3 V, float m, float F)
{
	vec3 H = normalize(V + L);
	float NdotH = dot(N, H);
	float NdotV = dot(N, V);
	float VdotH = dot(V, H);
	float NdotL = max(0.0, dot(N, L));
	float fKs = 0.0;
	float power = 2.0 / (m*m) + 2.0;//Ns < 1.0 ? 1.0 : Ns;
	if (m > 0.0)
		fKs = ComputeCookTorrance(N, L, V, H, F, m);
	else {
		fKs = F * G1(NdotL, NdotV, NdotH, VdotH) * ComputeBlinnPhong(NdotH, power) / (4.0 * NdotV * NdotL);//ComputeCookTorrance(N, L, V, H, F, 0.99);//ComputeBlinnPhong(NdotH, power);
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
	DoDeferredMain();
	DoDeferredLightAccum();
	FS_outputColor = DeferredLightBuffer;	
}

void old_main(void)
{
	// 1. Prepare our deferred variables: P, V, N, Kd
	vec3 N = normalize(VS_Normal);
	vec3 Nmap = N;
	vec3 V = normalize(VS_CameraPosition.xyz);
	//vec3 T = normalize(VS_Tangent);
	//vec3 B = normalize(VS_Binormal);
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

	// L, V, E, N, H and associated dot products
	//vec3 L = vec3(0, 3, 0) - VS_Position.xyz; 
	vec4 spherePos = Spheres[0];
	vec3 L = (spherePos - VS_Position).xyz;

	float lightRadius = Spheres[0].w;
	float lightArea = 4.0 * 3.14159 * lightRadius * lightRadius;
	float lightDistance = length(L);
	float lightDistance2 = lightDistance * lightDistance;
	float Pdistance = length(VS_CameraPosition.xyz);
	float Pdistance2 = Pdistance * Pdistance;
	L = normalize(L);
	vec3 H = normalize(V + L);
	float VcrossH = length(cross(V, H));

	float faceVisibility = max(0.0, dot(L, N)) > 0.0 ? 1.0 : 0.0;
	float cosLaw = max(0.0, dot(L, Nmap));// * faceVisibility;
	float spec = pow(max(epsilon, dot(L, H)), Ns);

	vec3 myKd = Kd;
	if (map_Kd_mix > 0.5)
	{
		vec3 map = texture(map_Kd, 1.0 - VS_TexCoord).rgb;
		myKd = (Kd * map);
	}

	vec3 myKs = Ks;
	if (map_Ks_mix > 0.5) 
	{
		vec3 map = texture(map_Ks, 1.0 - VS_TexCoord).rgb;
		myKs = Ks * map;
	}

	float kspec = pow(max(0.0, dot(N, H)), Ns);

	myKs += spec * vec3(1, 1, 1) * lightArea / (lightDistance2);
	kspec = 0.0;

	if (SpheresCount > 0)
		myKs = SpheresKe[0].rgb;

	float m;
	if (PBk > 0.0)
		m = PBm;
	else {
		if (PBmap_mix > 0.0)
			m = texture(PBmap, 1.0 - VS_TexCoord).r;
		else
			m = PBKsm;
	}

	float pb_ks_m = 0.0;
	float pb_kd_m = 0.0;
	float pb_ior = 0.0;
	float pb_metalior = 0.0;
	vec3 pb_metalF;

	if (PBmap_mix > 0.0)
	{
		vec4 pb = texture(PBmap, 1.0 - VS_TexCoord);
		pb_ks_m = pb.r;
		pb_kd_m = pb.g;
		pb_ior = PBior;
		pb_metalior = PBk;
	}
	else
	{
		pb_ks_m = PBKsm;
		pb_kd_m = PBKdm;
		pb_ior = PBior;
		pb_metalior = PBk;
	}

	if (PBk > 0.0)
	{
		float cos_theta_i = max(0.0, dot(V, H));
		pb_metalF = vec3(
			FresnelMetal(Metal_N2.x, Metal_K2.x, cos_theta_i),
			FresnelMetal(Metal_N2.y, Metal_K2.y, cos_theta_i),
			FresnelMetal(Metal_N2.z, Metal_K2.z, cos_theta_i)
			);
	}


	//vec3 finalColor = (vec3(1,1,1) * kspec + myKd) * cosLaw * 1.0/lightDistance2;
	//float exposure = pow(1.414,14);
	//finalColor *= exposure;
	//cosLaw = pow(cosLaw, 3);

	vec3 sunL = normalize(SunDirTo);
	vec3 sunH = normalize(SunDirTo + V);
	float bias = 0.00001;
	float offset = 0.00001;
	float biasW = bias / VS_SunShadowCoord.w;
	vec3 sunCoord = vec3(VS_SunShadowCoord.xy, VS_SunShadowCoord.z)/VS_SunShadowCoord.w;

	vec3 sunTexMapColor = textureProj(SunColorMap, VS_SunShadowCoord.xyw).rgb;
	float sunZ = textureProj(SunShadowMap, VS_SunShadowCoord.xyw).z;

	float sunZsamples[8];
	sunZsamples[0] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, offset)).z ? 0.0 : 1.0;
	sunZsamples[1] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, offset)).z ? 0.0 : 1.0;
	sunZsamples[2] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, -offset)).z ? 0.0 : 1.0;
	sunZsamples[3] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, -offset)).z ? 0.0 : 1.0;
	sunZsamples[4] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, 0)).z ? 0.0 : 1.0;
	sunZsamples[5] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, 0)).z ? 0.0 : 1.0;
	sunZsamples[6] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(0, -offset)).z ? 0.0 : 1.0;
	sunZsamples[7] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(-0, -offset)).z ? 0.0 : 1.0;

	//sunTexMapColor = sunCoord.xyz;	

	float shadowAmount = 1.0;
	if ((sunCoord.z - biasW) > sunZ || sunCoord.z < 0.0 || sunCoord.z > 1.0)
		shadowAmount = 0.1;

	//for (int i = 0; i < 8; i++)
	//	shadowAmount += sunZsamples[i];
	//shadowAmount = (shadowAmount) / 8;
	
	float tempKappa = 0.0;
	float tempRoughness = 0.5;
	float tempSpecularRoughness = 0.125;
	float tempDiffuseRoughness = 0.125;
	float tempMetalRoughness = 0.0;
	float tempIOR = 1.333;
	float tempE0 = 100.5;
	vec3 tempN = Nmap;
	vec3 tempV = normalize(V);
	float tempSpecularMix = 0.2;//1.0;
	float tempDiffuseMix = 1.0 - tempSpecularMix;

	vec3 enviroN = normalize(tempN);
	vec3 enviroL = normalize(reflect(tempV, enviroN));
	vec3 enviroH = normalize(enviroL + tempV);
	vec3 enviroCubeMapColor = texture(EnviroCubeMap, enviroL).rgb;

	//vec3 sunTexMapColor = sunCoord;//texture(SunColorMap, sunCoord.xy).rgb;//texture(map_Kd, sunCoord.xy).rgb;
	float sunF = ComputeFresnel(Nmap, sunL, V, sunH, pb_ior, pb_metalior);
	float sunT = 1.0 - sunF;
	float sunShadowR = GetSunShadow();
	

	//L = sunL;

	float E0 = tempE0 * ComputeE0(lightDistance);
	float F = ComputeFresnel(enviroN, L, V, H, pb_ior, pb_ks_m);
	float T = 1.0 - F;
	// vec3 enviroL = reflect(Nmap, V);
	//float enviroF = ComputeFresnel(Nmap, enviroL, V, enviroH, pb_ior, pb_metalior);
	float enviroF = ComputeFresnel(enviroN, -enviroL, V, enviroH, tempIOR, tempKappa);
	float enviroT = 1.0 - enviroF;
	vec3 enviroMapColor = texture(EnviroCubeMap, enviroL).rgb;
	float enviroKspec = ComputeCookTorrance(enviroN, enviroL, V, enviroH, enviroF, tempSpecularRoughness);
	float enviroKdiff = ComputeOrenNayer(enviroN, enviroL, V, tempDiffuseRoughness);

	float lightF = ComputeFresnel(tempN, L, V, normalize(L+V), tempIOR, tempKappa);
	float lightT = 1.0;// - lightF;

	//vec3 diffuseColor = ComputePBDiffuse(tempN, L, V, myKd, lightT, E0);
	//vec3 diffuseColor = ComputeOrenNayer(tempN, L, V, tempRoughness) * myKd;
	float Kdiff = ComputeOrenNayer(tempN, L, V, tempDiffuseRoughness);
	float Kspec = ComputeCookTorrance(tempN, L, V, normalize(L+V), lightF, tempSpecularRoughness);
	// vec3 specularColor = ComputePBSpecular(tempN, L, V, myKs, tempRoughness, lightF, tempE0);
	vec3 specularColor = Kspec * myKs * E0;
	vec3 diffuseColor = max(0.0, Kdiff) * max(0.0, (1.0 - Kspec)) * myKd * E0;
	//vec3 enviroColor = ComputePBSpecular(Nmap, enviroL, V, enviroMapColor, pb_ks_m, enviroF, EnviroCubeMapAmount);
	vec3 enviroColor = ComputePBSpecular(Nmap, enviroL, V, enviroMapColor, tempRoughness, enviroF, 1.0);
	vec3 enviroDiffuseColor = ComputePBDiffuse(Nmap, Nmap, V, myKd, enviroT, E0);
	vec3 sunColor = ComputePBDiffuse(Nmap, sunL, V, myKd, T, SunSize / 4.0) + ComputePBSpecular(Nmap, sunL, V, SunColor, pb_ks_m, sunF, SunSize / 4.0);
	vec3 finalColor = diffuseColor + specularColor;
	//finalColor = clamp(sunColor, 0, 1);
	//finalColor += enviroColor;
	finalColor *= shadowAmount;//+= sunColor + ComputePBLighting(Nmap, L, V, H, vec3(0,0,0), enviroCubeMapColor, reflectColor, E0);
	//finalColor += sunShadowR * vec3(1,1,1);
	if (isnan(finalColor.r)) finalColor = vec3(1,0,0);
	else if (isinf(finalColor.r)) finalColor = vec3(0,0,1);
	float NdotL = max(0.0, dot(Nmap, L));
	float NcrossL = length(cross(N, L));
	float aluminumF = FresnelMetal2(1.0, PBior, PBk, NdotL, NcrossL);
	if (PBk > 0.0)
	{
		float spec_cos_theta_i = max(0.0, dot(V, normalize(L+V)));
		float spec_sin_theta_i = sin(acos(spec_cos_theta_i));

		vec3 pb_metalSpecularF = vec3(
			FresnelMetal(Metal_N2.x, Metal_K2.x, spec_cos_theta_i),
			FresnelMetal(Metal_N2.y, Metal_K2.y, spec_cos_theta_i),
			FresnelMetal(Metal_N2.z, Metal_K2.z, spec_cos_theta_i)
			);
		// vec3 pb_metalSpecularF = vec3(
		// 	FresnelMetal2(1.0, Metal_N2.x, Metal_K2.x, spec_cos_theta_i, spec_sin_theta_i),
		// 	FresnelMetal2(1.0, Metal_N2.y, Metal_K2.y, spec_cos_theta_i, spec_sin_theta_i),
		// 	FresnelMetal2(1.0, Metal_N2.z, Metal_K2.z, spec_cos_theta_i, spec_sin_theta_i)
		// 	);
		float enviro_cos_theta_i = max(0.0, dot(V, normalize(enviroL+V)));
		vec3 pb_metalEnviroF = vec3(
			FresnelMetal(Metal_N2.x, Metal_K2.x, enviro_cos_theta_i),
			FresnelMetal(Metal_N2.y, Metal_K2.y, enviro_cos_theta_i),
			FresnelMetal(Metal_N2.z, Metal_K2.z, enviro_cos_theta_i)
			);
		vec3 metalSpecular = ComputePBMetalSpecular(tempN, L, V, myKs, tempSpecularRoughness, pb_metalSpecularF, tempE0);
		vec3 metalEnviro = ComputePBMetalSpecular(tempN, enviroL, V, enviroCubeMapColor, tempMetalRoughness, pb_metalEnviroF, tempE0);//pb_metalF * E0 * 0.1 * dot(Nmap, L);
		FS_outputColor = vec4(metalSpecular, 1.0);
		//FS_outputColor = vec4(enviroCubeMapColor, 1.0);
		//FS_outputColor = vec4(metalSpecular, 1.0);
		FS_outputColor = vec4(diffuseColor * (1.0 - aluminumF) + E0*enviroCubeMapColor * aluminumF * NdotL, 1.0);
	}
	else
	{
		FS_outputColor = vec4(diffuseColor + specularColor, 1.0);
		//FS_outputColor = vec4(finalColor, 1.0);		
	}
	//FS_outputColor = vec4(diffuseColor + specularColor + enviroKspec * enviroCubeMapColor, 1.0);
	//FS_outputColor = vec4(diffuseColor + specularColor + enviroT * enviroKdiff * myKd + enviroF * enviroCubeMapColor, 1.0);
	//FS_outputColor = vec4(diffuseColor + specularColor, 1.0);

	//FS_outputColor = vec4(dot(V,H), 0.0, 0.0, 1.0);
	//FS_outputColor = vec4(VcrossH, 0.0, 0.0, 1.0);
	//FS_outputColor = vec4(0.5*enviroF+0.5, 0.0, 0.0, 1.0);
	//FS_outputColor = vec4(enviroColor + enviroDiffuseColor, 1.0);//enviroT * vec3(1.0, 1.0, 1.0), 1.0);

	//FS_outputColor = vec4(vec3(1,1,1) * cosLaw, 1.0);
	//FS_outputColor = vec4(myKd, 1);
	//FS_outputColor = vec4(myKd * (cosLaw*exposure)*lightArea/lightDistance2, 1);

	DoDeferredMain();
	DoDeferredLightAccum();
	FS_outputColor = vec4(1,1,1,1);DeferredLightBuffer;
}



void DoDeferredMain()
{
	// 1. Prepare our deferred variables: P, V, N, Kd
	vec3 N = normalize(VS_Normal);
	vec3 Nmap = N;
	vec3 V = normalize(CameraPosition.xyz - VS_Position.xyz);
	//vec3 V = vec3(0.0, 0.0, -1.0);//normalize(VS_CameraPosition.xyz);

	float NdotV = dot(N, V);

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
		//if (dot(fragN, fragNmap) < 0.0) fragNmap = vec3(0.0, 0.0, 0.0);
	}

	if (map_Kd_mix > 0.5)
		material.kd = texture(map_Kd, 1.0 - VS_TexCoord).rgb * Kd;
	else
		material.kd = Kd;

	if (map_Ks_mix > 0.5)
		material.ks = texture(map_Ks, 1.0 - VS_TexCoord).rgb * Ks;
	else
		material.ks = Ks;

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
		float F = ComputeFresnel(N, L, V, H, PBior, PBk);
		Ks = clamp(ComputePBSpecular2(N, L, V, PBKsm, F), 0.0, 1.0);
		Kd = 1.0 - Ks;
		specularColor = vec4(Ks * E0.rgb, 1.0);
		if (specularOnly == false)
			diffuseColor = vec4(E0.rgb * c_d.rgb * ComputePBDiffuse2(N, L, V, Kd), 1.0);
	}

	return ambientFactor * (specularColor + diffuseColor);
}


vec4 ComputePB_GRAPP2017(in vec3 N, in vec3 L, in vec3 V, in vec3 H, in vec3 E0, in vec3 c_d, in bool specularOnly)
{
	float VdotH = max(0.0, dot(V, H));
	float NdotL = max(0.0, dot(N, L));
	float NdotH = dot(N, H);
	float ambientFactor = 1.0;
	if (NdotL <= 0.0) ambientFactor = 2.0 / 3.14159;
	vec4 specularColor;
	vec4 diffuseColor;
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

		specularColor = vec4(E0.rgb * metalKs.rgb, 1.0);
	}
	else {
		float F = ComputeFresnel(N, L, V, H, PBior, PBk);
		float Ks = clamp(ComputePBSpecular2(N, L, V, PBKsm, F), 0.0, 1.0);
		float Kd = 1.0 - Ks;
		specularColor = vec4(Ks * E0.rgb, 1.0);
		if (specularOnly == false)
			diffuseColor = vec4(E0.rgb * c_d.rgb * ComputePBDiffuse2(N, L, V, Kd), 1.0);
	}

	return ambientFactor * (specularColor + diffuseColor);
}


void DoDeferredLightAccum()
{
	// Debug Choices
	// - 0 - Do normal rendering pipeline
	// - 1 - Diffuse Color
	// - 2 - Specular Color
	// - 3 - Depth (log)
	// - 4 - Normal Map
	// - 5 - Face Normals
	// - 6 - Dielectric Fresnel
	// - 7 - Metal Fresnel 1
	// - 8 - Metal Fresnel 2
	// - 9 - Oren-Nayer
	
	// G-Buffer Inputs
	// DeferredPB1 = vec4(VS_Position.xyz, 1.0);
	// DeferredPB2 = vec4(Nmap.xyz, 1.0);
	// DeferredPB3 = vec4(material.kd.rgb, 1.0);
	// DeferredPB4 = vec4(material.ks.rgb, 1.0);
	// DeferredPB5 = vec4(N.xyz, 1.0);
	// DeferredPB6 = vec4(V.xyz, 1.0);
	int debugChoice = 10;

	if (debugChoice == 0)
	{
		// unpack deferred parameters
		vec3 P = DeferredPB1.xyz;
		vec3 N = DeferredPB2.xyz;
		vec3 V = DeferredPB6.xyz;
		vec3 kd = DeferredPB3.rgb;

		// For each light ...
		vec4 spherePos = Spheres[0];
		vec3 ks = SpheresKe[0].rgb;
		vec3 L = (spherePos.xyz - P);
		float Ldistance = length(L);
		L = normalize(L);
		vec3 H = normalize(V+L);
		float E0 = 1000.0;
		float Lfactor = E0 / (4.0 * 3.14159 * Ldistance * Ldistance);

		float NdotL = clamp(dot(N, L), 0.0, 1.0);
		float F = ComputeFresnel(N, L, V, H, material.specularN2, material.specularK);
		float d = ComputeOrenNayer(N, L, V, material.diffuseRoughness) * Lfactor;
		float s = ComputeCookTorrance(N, L, V, H, F, material.specularRoughness) * Lfactor;
		vec3 metalColor = vec3(1.0, 1.0, 1.0);
		float VdotH = dot(V, H);

		DeferredLightBuffer += (1.0 - F) * d * vec4(kd, 1) + s * vec4(metalColor * ks, 1);

		// The Sun
		Lfactor = 1.0;
		vec3 enviroL = normalize(reflect(V, N));
		vec3 enviroMapColor = texture(EnviroCubeMap, enviroL).rgb;

		L = normalize(SunDirTo);
		H = normalize(SunDirTo + V);

		NdotL = clamp(dot(N, L), 0.0, 1.0);
		F = ComputeFresnel(N, L, V, H, material.specularN2, material.specularK);
		float F2 = ComputeFresnel(N, enviroL, V, H, material.specularN2, material.specularK);
		d = ComputeOrenNayer(N, L, V, material.diffuseRoughness) * Lfactor;
		s = ComputeCookTorrance(N, L, V, H, F, material.specularRoughness) * Lfactor;
		float s2 = ComputeCookTorrance(N, L, V, H, F2, material.specularRoughness) * Lfactor;
		metalColor = vec3(1.0, 1.0, 1.0);
		VdotH = dot(V, H);

		float sunShadowR = 1.0;//GetSunShadow();
		d *= sunShadowR;
		s *= sunShadowR;

		DeferredLightBuffer += (1.0 - F) * d * vec4(kd, 1.0) + s * vec4(metalColor, 1.0) + s2 * vec4(metalColor * enviroMapColor, 1.0);
	}
	else if (debugChoice == 1)
	{
		// Set the pixel color from the Material kd color
		DeferredLightBuffer += vec4(DeferredPB3.rgb, 1.0);
	}
	else if (debugChoice == 2)
	{
		// Set the pixel color from the material ks color
		DeferredLightBuffer += vec4(DeferredPB4.rgb, 1.0);
	}
	else if (debugChoice == 3)
	{
		// Set the pixel color from the position
		DeferredLightBuffer += vec4(DeferredPB1.rgb, 1.0);		
	}
	else if (debugChoice == 4)
	{
		// Set the pixel color from the normal map
		DeferredLightBuffer += vec4((DeferredPB2.rgb+1.0)/2.0, 1.0);		
	}
	else if (debugChoice == 5)
	{
		// Set the pixel color from the face normals
		DeferredLightBuffer += vec4((DeferredPB5.rgb+1.0)/2.0, 1.0);		
	}
	else if (debugChoice == 6)
	{
		// Set the pixel color from the view direction
		DeferredLightBuffer += vec4((DeferredPB6.rgb+1.0)/2.0, 1.0);		
	}
	else if (debugChoice == 7)
	{
		// Set the pixel color to basic N dot L lighting

		// find out intensity of sun...it is zero if below the horizon...
		float maxSunIntensity = 1.0;
		if (SunDirTo.y < 0.0) maxSunIntensity = max(0.0, 1.0 - 5.0 * abs(SunDirTo.y));
		highp vec3 N = normalize(DeferredPB2.xyz);
		highp vec3 L = normalize(SunDirTo);
		highp vec3 V = normalize(DeferredPB6.xyz);
		highp vec3 H = normalize(SunDirTo + V);
		highp float NdotL = max(0.0, dot(N, L));

		// material properties
		vec3 pb_metalDiffuse = vec3(
			FresnelMetal(Metal_N2.r, Metal_K2.r, 0.0),
			FresnelMetal(Metal_N2.g, Metal_K2.g, 0.0),
			FresnelMetal(Metal_N2.b, Metal_K2.b, 0.0)
			);

		// Sun illumination
		bool inDisc = false;
		if (NdotL > 0.99999) inDisc = true;
		float sunRadiance = length(SunE0.rgb);
		float sunScaleFactor = 1.0 / length(SunE0.rgb);
		float sunF = ComputeFresnel(N, L, V, H, PBior, PBk);
		float sunT = 1.0 - sunF;

		highp vec4 sunE0;
		if (SunDirTo.y < 0.0) {
			sunE0 = vec4(0.0, 0.0, 0.0, 0.0);
			sunF = 0.0;
			sunT = 0.0;
		}
		else {
			sunE0 = sunScaleFactor * SunE0;
			// sunE0 = vec3(1.0, 1.0, 1.0);
		}


		vec4 sunDiffuseColor;
		vec4 sunSpecularColor;
		float sunKs = 0.0;
		float sunKd = 1.0;
		if (inDisc) {
			if (PBk > 0.0) {
				float spec_cos_theta_i = max(0.0, dot(V, normalize(L+V)));
				float spec_sin_theta_i = sin(acos(spec_cos_theta_i));

				vec3 pb_metalSpecularF = vec3(
					FresnelMetal(Metal_N2.x, Metal_K2.x, spec_cos_theta_i),
					FresnelMetal(Metal_N2.y, Metal_K2.y, spec_cos_theta_i),
					FresnelMetal(Metal_N2.z, Metal_K2.z, spec_cos_theta_i)
					);
				vec3 skyboxMetalKs = ComputePBMetalSpecular2(N, L, V, PBKsm, pb_metalSpecularF);
				vec3 skyboxMetalKd = 1.0 - skyboxMetalKs;
				sunSpecularColor = vec4(skyboxMetalKs * sunE0.rgb, 1.0);
				//sunDiffuseColor = vec4(skyboxMetalKd * sunE0, 1.0);
			}
			else {
				sunKs = ComputePBSpecular2(N, L, V,  PBKsm, sunF);
				sunKd = 1.0 - sunKs;
				sunSpecularColor = sunKs * sunE0;
				// sunSpecularColor = vec4(ComputePBSpecular(N, L, V, sunE0, PBKsm, sunF, 1.0), 1.0);
			}
		}
		if (PBk > 0.0) {
			float spec_cos_theta_i = max(0.0, dot(V, normalize(L+V)));
			float spec_sin_theta_i = sin(acos(spec_cos_theta_i));

			vec3 pb_metalSpecularF = vec3(
				FresnelMetal(Metal_N2.x, Metal_K2.x, spec_cos_theta_i),
				FresnelMetal(Metal_N2.y, Metal_K2.y, spec_cos_theta_i),
				FresnelMetal(Metal_N2.z, Metal_K2.z, spec_cos_theta_i)
				);
			vec3 skyboxMetalKs = (1.0 - PBKdm) * ComputePBMetalSpecular2(N, L, V, PBKsm, pb_metalSpecularF);
			vec3 skyboxMetalKd = PBKdm * vec3(1.0, 1.0, 1.0);//1.0 - skyboxMetalKs;
			sunSpecularColor = vec4(sunE0.rgb * skyboxMetalKs, 1.0);
			// sunDiffuseColor = vec4(sunE0.rgb * DeferredPB3.rgb * ComputePBDiffuse2(N, L, V, 1.0 - length(skyboxMetalKs)), 1.0);
			//sunDiffuseColor = vec4(sunE0.rgb * pb_metalDiffuse.rgb * ComputePBDiffuse2(N, L, V, 1.0 - length(skyboxMetalKs)), 1.0);
			sunDiffuseColor = vec4(sunE0.rgb * DeferredPB3.rgb * ComputePBDiffuse2(N, L, V, PBKdm), 1.0);
		} else {
			//sunDiffuseColor = vec4(sunE0.rgb * ComputePBDiffuse(N, L, V, DeferredPB3.rgb, sunKd, 1.0), 1.0);
			//sunDiffuseColor = vec4(sunE0.rgb * ComputePBDiffuse(N, L, V, DeferredPB3.rgb, sunKd, sunKd), 1.0);
			sunKs = ComputePBSpecular2(N, L, V,  PBKsm, sunF);
			sunKd = 1.0 - sunKs;
			sunSpecularColor = vec4(sunKs * sunE0.rgb, 1.0);
			sunDiffuseColor = vec4(sunE0.rgb * DeferredPB3.rgb * ComputePBDiffuse2(N, L, V, sunKd), 1.0);
		}

		vec4 sunColor = ComputePB(
			N,
			normalize(SunDirTo),
			V,
			normalize(SunDirTo + V),
			sunE0.rgb,
			DeferredPB3.rgb,
			false);


		// if (inDisc == true)
		// {
		// 	vec4 sunDiscColor = ComputePB(
		// 		N,
		// 		normalize(SunDirTo),
		// 		V,
		// 		normalize(SunDirTo + V),
		// 		SunE0.rgb,
		// 		DeferredPB3.rgb);
		// 	sunColor = 0.0*vec4(sunColor.rgb + sunDiscColor.rgb, 1.0);
		// }

		// Skybox illumination
		highp vec3 skyboxL = normalize(reflect(-V, N));
		highp vec3 skyboxTexel;

		if (SunDirTo.y > 0.0) {
			if (skyboxL.y < 0.0) {
				skyboxTexel = sunScaleFactor * GroundE0.rgb;
			}
			else {
				skyboxTexel = texture(PBSkyCubeMap, skyboxL).rgb;
				if (inDisc) {
					skyboxTexel += SunE0.rgb * sunScaleFactor;
				}
			}
		}

		vec4 skyboxColor = ComputePB(
			N,
			skyboxL,
			V,
			normalize(skyboxL + V),
			skyboxTexel,
			DeferredPB3.rgb,
			true);

		// float skyboxKs = 0.0;
		// float skyboxKd = 1.0;
		// float skyboxF = ComputeFresnel(N, skyboxL, V, H, PBior, PBk);
		// float skyboxT = 1.0 - skyboxF;

		// highp vec4 specularColor;
		// highp vec4 diffuseColor;
		// if (PBk > 0.0) {
		// 	float spec_cos_theta_i = max(0.0, dot(V, normalize(skyboxL+V)));
		// 	float spec_sin_theta_i = sin(acos(spec_cos_theta_i));

		// 	vec3 pb_metalSpecularF = vec3(
		// 		FresnelMetal(Metal_N2.x, Metal_K2.x, spec_cos_theta_i),
		// 		FresnelMetal(Metal_N2.y, Metal_K2.y, spec_cos_theta_i),
		// 		FresnelMetal(Metal_N2.z, Metal_K2.z, spec_cos_theta_i)
		// 		);
		// 	/*
		// 	vec3 pb_metalSpecularF = vec3(
		// 		FresnelMetal2(1.0001, Metal_N2.x, Metal_K2.x, spec_cos_theta_i, spec_sin_theta_i),
		// 		FresnelMetal2(1.0001, Metal_N2.y, Metal_K2.y, spec_cos_theta_i, spec_sin_theta_i),
		// 		FresnelMetal2(1.0001, Metal_N2.z, Metal_K2.z, spec_cos_theta_i, spec_sin_theta_i)
		// 		);			
		// 	float metalF = FresnelMetal2(1.0001, PBior, PBk, spec_cos_theta_i, spec_sin_theta_i);
		// 	vec3 pb_metalSpecularF = vec3(metalF, metalF, metalF);
		// 	*/
		// 	vec3 skyboxMetalKs = ComputePBMetalSpecular2(N, skyboxL, V, PBKsm, pb_metalSpecularF);
		// 	vec3 skyboxMetalKd = 1.0 - skyboxMetalKs;

		// 	specularColor = 0.0*vec4(skyboxMetalKs * skyboxTexel, 1.0);
		// 	//specularColor = vec4(ComputePBMetalSpecular(N, skyboxL, V, skyboxTexel, 0.0, pb_metalSpecularF, 1.0), 1.0);
		// 	//diffuseColor = vec4(sunE0.rgb * DeferredPB3.rgb * (1.0 - pb_metalSpecularF) * max(0.0, dot(N, skyboxL)), 0.0);// vec4(sunE0.rgb * ComputePBDiffuse(N, L, V, DeferredPB3.rgb, skyboxT, 1.0), 1.0);
		// 	// diffuseColor = vec4(sunE0.rgb * DeferredPB3.rgb * (1.0 - pb_metalSpecularF) * ComputePBDiffuse2(N, L, V, skyboxKd), 1.0);
		// 	diffuseColor = 0.0*vec4(sunE0.rgb * pb_metalDiffuse.rgb * (1.0 - pb_metalSpecularF) * ComputePBDiffuse2(N, L, V, skyboxKd), 1.0);
		// }
		// else {
		// 	skyboxKs = ComputePBSpecular2(N, skyboxL, V, PBKsm, skyboxF);
		// 	skyboxKd = 1.0 - skyboxKs;
		// 	// specularColor =  vec4(ComputePBSpecular(N, skyboxL, V, skyboxTexel, PBKsm, skyboxF, 1.0), 1.0);
		// 	specularColor = vec4(skyboxKs * skyboxTexel, 1.0);
		// 	diffuseColor = vec4(sunE0.rgb * ComputePBDiffuse(N, L, V, DeferredPB3.rgb, skyboxKd, 1.0), 1.0);
		// }		

		//DeferredLightBuffer += GetSunShadow() * log(1.0 + diffuseColor + specularColor + sunDiffuseColor + sunSpecularColor);
		DeferredLightBuffer += 1.0 / sunE0 * (sunColor + skyboxColor);// + sunDiffuseColor + sunSpecularColor);
		// DeferredLightBuffer += 4.0 * GetSunShadow() * sunDiffuseColor + sunSpecularColor;
	}
	else if (debugChoice == 8)
	{
		vec3 debugSunDirTo = vec3(1.0, 2.0, 3.0);
		vec3 P = DeferredPB1.xyz;
		vec3 N = DeferredPB2.xyz;
		vec3 V = DeferredPB6.xyz;
		vec3 L = normalize(SunDirTo);
		vec3 H = normalize(SunDirTo + V);
		float F = ComputeFresnel(N, L, V, H, 3.5, 1.0 );
		float T = 1.0 - F;		
		float NdotL = clamp(dot(N, L), 0.0, 1.0);
		
		// if there is no light, there is no Fresnel transmission or reflectance
		if (NdotL == 0.0 || SunDirTo.z < 0.0)
		{
			F = 0.0;
			T = 0.0;
		}
		// DeferredLightBuffer += vec4(DeferredPB3.rgb, 1.0) * NdotL * (1.0 - F);
		DeferredLightBuffer += vec4(1.0, 1.0, 1.0, 1.0) * NdotL * T;
		DeferredLightBuffer += vec4(1.0, 1.0, 1.0, 1.0) * F;				
	}
	else if (debugChoice == 9)
	{
		vec3 P = DeferredPB1.xyz;
		vec3 N = DeferredPB2.xyz;
		vec3 V = DeferredPB6.xyz;
		vec3 L = normalize(SunDirTo);
		vec3 H = normalize(SunDirTo + V);
		//float F = ComputeFresnel(N, L, V, H, material.specularN2, material.specularK);
		float F = ComputeFresnel(N, L, V, H, 1.5, 0.0);
		float T = 1.0 - F;		
		float NdotL = clamp(dot(N, L), 0.0, 1.0);
		float Lfactor = 1.0;

		float d = ComputeOrenNayer(N, L, V, 0.0) * Lfactor;
		float s = ComputeCookTorrance(N, L, V, H, F, 1.0) * Lfactor;
		// if there is no light, there is no Fresnel transmission or reflectance
		if (NdotL == 0.0 || SunDirTo.z < 0.0)
		{
			F = 0.0;
			T = 0.0;
		}
		// DeferredLightBuffer += vec4(DeferredPB3.rgb, 1.0) * NdotL * (1.0 - F);
		DeferredLightBuffer += vec4(DeferredPB3.rgb, 1.0) * d;
		DeferredLightBuffer += vec4(1.0, 1.0, 1.0, 1.0) * s;
		//DeferredLightBuffer = vec4(material.specularRoughness, material.diffuseRoughness, 0.0, 1.0) * s;
	}
	else if (debugChoice == 10)
	{
		highp vec3 N = normalize(DeferredPB2.xyz);
		highp vec3 Nflat = normalize(DeferredPB5.xyz);
		highp vec3 V = normalize(DeferredPB6.xyz);

		highp float sunShadowR = GetSunShadow();
		highp vec3 sunE0 = GetPBSky(SunDirTo, 0.0).rgb;

		highp vec4 sunColor = sunShadowR * ComputePB_GRAPP2017(
			N,
			normalize(SunDirTo),
			V,
			normalize(SunDirTo + V),
			sunE0,
			DeferredPB3.rgb,
			false);

		DeferredLightBuffer += sunColor;

		// Skybox illumination
		highp vec3 skyboxL = normalize(reflect(-V, N));
		highp vec3 skyboxTexel = GetPBSky(skyboxL, 0.0).rgb;

		highp vec4 skyboxColor = ComputePB_GRAPP2017(
					N,
					skyboxL,
					V,
					normalize(skyboxL + V),
					skyboxTexel,
					DeferredPB3.rgb,
					false);

		DeferredLightBuffer += skyboxColor;


		// Diffuse environmental secondary illumination
		highp vec3 diffuseEnvL = Nflat;
		highp vec3 diffuseEnvPixel = GetPBSky(diffuseEnvL, 0.0).rgb;

		highp float diffuseEnvNdotL = max(0.0, dot(N, diffuseEnvL));
		highp vec4 diffuseEnvColor = ComputePB_GRAPP2017(
			diffuseEnvL,
			diffuseEnvL,
			V,
			normalize(diffuseEnvL + V),
			diffuseEnvPixel,
			DeferredPB3.rgb,
			false
		);


		DeferredLightBuffer += diffuseEnvColor;
		//DeferredLightBuffer *= invSunE0;
	}

}
