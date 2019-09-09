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
	samplerCube LightProbeCubeMap;
	samplerCube EnvironmentCubeMap;
	samplerCube DepthShadowCubeMap;
	float DepthShadowZFar;
	vec3 sph[9];
};

struct PointLight
{
	int Enabled;
	vec3 E0;
	vec3 Position;
	samplerCube DepthShadowCubeMap;
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
int ShadowQuality = 0;

uniform SphlLight SphlLights[16];
uniform int SphlLightCount;

uniform PointLight PointLights[16];
uniform int PointLightCount;

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
float BumpinessFactor = 0.5;

float SunShadowDarkness = 0.1;
float ShadowDarkness = 0.001;

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


// PBSky ------------------------------------------------------------//


vec4 GetPBSky(vec3 L, float spread)
{
	return texture(PBSkyCubeMap, L);
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
		float LightZ = SphlLights[lightId].DepthShadowZFar * texture(SphlLights[lightId].DepthShadowCubeMap, Lights[lightId].L).r;
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
					float LightZ = SphlLights[lightId].DepthShadowZFar * texture(SphlLights[lightId].DepthShadowCubeMap, L + vec3(x,y,z)).r;
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
			float LightZ = SphlLights[lightId].DepthShadowZFar *
						   texture(SphlLights[lightId].DepthShadowCubeMap,
						   	       L + sampleOffsetDirections[i] * diskRadius).r;
			shadow += Z - bias > LightZ ? 0.0 : 1.0;
		}

		Lights[lightId].shadow = shadow / float(samples);
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
		Lights[i].I = texture(SphlLights[i].LightProbeCubeMap, texcoord).rgb;// * 6.0;
		Lights[i].E0 = 1.0;
	}
	Lights[i].H = normalize(Fragment.V + Lights[i].L);
	Lights[i].NdotL = max(0.001, dot(Fragment.N, Lights[i].L));
	Lights[i].NdotV = max(0.001, dot(Fragment.N, Fragment.V));
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
		return texture(SphlLights[lightId].EnvironmentCubeMap, R).rgb;
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
		vec3 lightColor = texture(SphlLights[i].LightProbeCubeMap, Lights[i].L).rgb;

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

	return f_r * Material.Kd;
}


vec3 GetPbrBrdfSpecular(int i)
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
		z = texture(SphlLights[0].DepthShadowCubeMap, Fragment.R).z;
		if (z != 1.0) return vec3(0.0);
	}
	float C = (2.0 + Material.specularExponent) / (2.0 * 3.14159);
	vec3 F = GetPbrEnviroBrdfFresnel(i);
	return GetEnviroColor(i) * F * C * G2_GGX(i) / (4.0 * Fragment.NdotV);
}


vec3 GetPbrAmbient(int i)
{
	if (i < SunIndex)
		return texture(SphlLights[i].LightProbeCubeMap, Fragment.N).rgb;
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


float erf(float x)
{

	float x2 = x * x;
	float ax2 = 0.140012 * x2;
	float erf_result = sqrt(1.0 - exp(-x2 * (1.2732395 + ax2) / (1.0 + ax2)));
	if (x < 0.0) return -erf_result;
	return erf_result;
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


float ComputeBlinnPhong(float NdotH, float power)
{
	float t = pow(NdotH, power);
	return (power + 2.0) / 6.283185 * t;
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





// -------------------------------------------------------------------


// Main Function
void main(void)
{
	if (ToneMapScale > 0.0)
		toneMapScale = ToneMapScale;
	DoDeferredMain();
	DoDeferredLightAccum();

	// exposure and gamma
	vec3 finalColor = DeferredLightBuffer * ToneMapExposure;
	finalColor = pow(finalColor, vec3(1.0 / ToneMapGamma));	

	// vec3 finalColor = DeferredLightBuffer * ToneMapScale;

	FS_outputColor = vec4(finalColor, 1.0);
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
	// - 3  - Normal
	// - 4  - View
	// - 5  - Reflection
	// - 6  - Light
	// - 7  - Half Vector
	// - 8  - N.L
	// - 9  - Mobile Shader 1
	// - 10 - Mobile Shader 2
	// - 11 - PBR Diffuse
	// - 12 - PBR Specular
	// - 13 - PBR Environment
	// - 14 - Testing
	// - 15 - Testing
	// - 16 - Testing
	// - 17 - First lit SPHL only
	// - 18 - Ambient + all SPHLs
	// - 19 - Indirect Only
	// - 20 - Indirect Only using SPHL visibility

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
		debugChoice = 17;
	}

	if (debugChoice == 1)
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
