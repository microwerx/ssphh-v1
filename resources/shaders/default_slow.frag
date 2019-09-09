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
#version 150

// Uniforms ----------------------------------------------------------

uniform highp float UScreenWidth;
uniform highp float UScreenHeight;
uniform highp vec4 CameraPosition;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 Ke;
uniform highp float Tf;
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


highp float pi = 3.1415926;
float invPi = 0.31831;
float epsilon = 0.00001;
float fragmentArea = 1.0;

float lambda_r = 650;
float lambda_g = 532;
float lambda_b = 473;

vec3 gold_ior = vec3(0.166, 0.402, 1.242);
vec3 gold_coef = vec3(3.15, 2.54, 1.796);


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
	sunZsamples[0] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, offset)).z ? 0 : 1;
	sunZsamples[1] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, offset)).z ? 0 : 1;
	sunZsamples[2] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, -offset)).z ? 0 : 1;
	sunZsamples[3] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, -offset)).z ? 0 : 1;
	sunZsamples[4] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, 0)).z ? 0 : 1;
	sunZsamples[5] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, 0)).z ? 0 : 1;
	sunZsamples[6] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(0, -offset)).z ? 0 : 1;
	sunZsamples[7] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy + vec2(-0, -offset)).z ? 0 : 1;
	sunZsamples[8] = sunCoord.z - bias > texture(SunShadowMap, sunCoord.xy).z ? 0 : 1;

	float sampleSum = 0.0;
	for (int i = 0; i < 9; i++) {
		sampleSum += sunZsamples[i];
	}
	sampleSum /= 9.0;

	//sunTexMapColor = sunCoord.xyz;	

	float shadowAmount = 1.0;
	if (sunCoord.z < 0 || sunCoord.z > 1)
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
	vec3 map = 2*texture(map_normal, texcoord).rgb-1;
	mat3 TBN = MakeCotangentFrame(N, -V, texcoord);
	return normalize(TBN * map);
}


// Physically Based Lighting Code ------------------------------------


float FresnelDielectric(in float n_1, in float n_2, in float cos_theta_i, in float sin_theta_i)
{
	float n1c = n_1 * cos_theta_i;
	float n2c = n_2 * cos_theta_i;
	float n1n2s = pow(1 - pow((n_1 / n_2) * sin_theta_i, 2.0), 0.5);
	float n1s = n_1 * n1n2s;
	float n2s = n_2 * n1n2s;

	float Rs = 1;
	float Rp = 1;
	float den = n1c + n1s;
	Rs = pow((n1c - n2s)/(n1c + n2s), 2.0);
	den = n1s + n1c;
	Rp = pow((n1s - n2c)/(n1s + n2c), 2.0);
	float R = (Rs + Rp) * 0.5;

	return clamp(R, 0, 1);
}

float FresnelMetal(in float n, in float k, in float cos_theta_i)
{
	float Rs;
	float Rp;
	float n2 = n*n;
	float k2 = k*k;
	float cos2 = cos_theta_i * cos_theta_i;
	float n2k2cos2 = (n2 + k2) * cos2;
	float n2cos = 2 * n * cos_theta_i;
	float n_minus_cos_2 = (n - cos_theta_i) * (n - cos_theta_i);
	float n_plus_cos_2 = (n + cos_theta_i) * (n + cos_theta_i);


	Rs = (n_minus_cos_2 + k2) / (n_plus_cos_2 + k2);
	Rp = (n2k2cos2 - n2cos + 1) / (n2k2cos2 + n2cos + 1);

	float R = (Rs + Rp) * 0.5;
	return clamp(R, 0, 1);
}


float FresnelMetal2(in float n_1, in float n_2, in float k_2, in float cos_theta_i, in float sin_theta_i)
{
	if (cos_theta_i <= 0.0) return 1.0;
	float k_2Squared = k_2 * k_2;
	if (cos_theta_i >= 1.0) {
		float t1 = 1 - n_2;
		float t2 = 1 + n_2;
		return (t1*t1 + k_2Squared) / (t2*t2 + k_2Squared);
	}
	float n_2Squared = n_2 * n_2;
	float n_1Squared = n_1 * n_1;
	float NcrossLSquared = sin_theta_i * sin_theta_i;
	float a = n_2Squared - k_2Squared - n_1Squared * NcrossLSquared;
	float aSquared = a * a;
	float b = 4.0 * n_2Squared * k_2Squared;
	float c = sqrt(aSquared - b);
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
	return clamp(R, 0, 1);
}


float ComputePhong(float NdotH, float power)
{
	return pow(max(0, NdotH), power);
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
	// tan2alpha / m2 = ((1 - cos2alpha)/(cos2alpha * m2))
	//float beckmann = exp(-((1 - cos2alpha)/(cos2alpha * m2))) / (pi * m2 * cos2alpha * cos2alpha);

	// These two are exactly the same...
	//float beckmann = 1.0 / (3.14159 * m2 * cos2alpha*cos2alpha) * exp(-tan2alpha/m2);
	float beckmann = 1.0 / (3.14159 * m2 * cos2alpha*cos2alpha) * exp((cos2alpha-1.0)/(cos2alpha*m2));
	float pixarD = 100.0 * exp(-(alpha*alpha)/m2);

	float fKs = 0;

	float D = beckmann;
	float G = min(1.0, min(2.0*NdotH*NdotV/VdotH, 2.0*NdotH*NdotL/VdotH));
	float Ks_den = 4.0 * 3.14159 * NdotV * NdotL;
	fKs = D * F * G / Ks_den;

	// A BRDF with a lower value than 0 is not physically plausible
	return max(0.0, fKs);
}

float ComputeOrenNayer(vec3 N, vec3 L, vec3 V, float m)
{
	float NdotL = max(dot(N, L), 0.0);
	if (m <= 0.0) return NdotL;
	if (NdotL == 0.0) return 0.0;

	float NdotV = max(dot(N, V), 0);
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
	if (NdotL == 0) return vec3(0.0, 0.0, 0.0);

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

float ComputeE0(float Ldistance)
{
	float Ld2 = Ldistance * Ldistance;
	float E0;
	if (Ldistance > 0)
		E0 = 15.0 * 4 * pi / Ld2;
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
	float F = 0;

	float R_0 = (n_1 - n_2)/(n_1 + n_2); R_0 = R_0 * R_0; // for schlick's approximation
	float schlicks = R_0 + (1 - R_0) * pow(1 - VdotH, 5.0);
	float fresnel = FresnelDielectric(n_1, n_2, VdotH, sin_theta_i);
	float fresnelFace = FresnelDielectric(n_1, n_2, NdotL, sin(acos(NdotL)));

	if (k > 0)
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
	float power = Ns < 1 ? 1 : Ns;
	if (m > 0)
		fKs = vec3(
			ComputeCookTorrance(N, L, V, H, F.r, m),
			ComputeCookTorrance(N, L, V, H, F.g, m),
			ComputeCookTorrance(N, L, V, H, F.b, m)
			);
	else
		fKs = F * ComputePhong(NdotH, power);

	return E0 * fKs * myKs;
}

vec3 ComputePBSpecular(vec3 N, vec3 L, vec3 V, vec3 myKs, float m, float F, float E0)
{
	vec3 H = normalize(V + L);
	float NdotH = max(dot(N,H), 0.0);

	float fKs = 0.0;
	float power = Ns < 1 ? 1 : Ns;
	if (m > 0)
		fKs = ComputeCookTorrance(N, L, V, H, F, m);
	else
		fKs = ComputePhong(NdotH, power);//ComputeCookTorrance(N, L, V, H, F, 0.99);//ComputePhong(NdotH, power);

	return E0 * fKs * myKs;
}

vec3 ComputePBLighting(vec3 N, vec3 L, vec3 V, vec3 H, vec3 myKd, vec3 myKs, vec3 reflectColor, float E0)
{
	vec3 finalColor = vec3(1,1,1);

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
	float schlicks = R_0 + (1 - R_0) * pow(1 - VdotH, 5.0);
	float fresnel = FresnelDielectric(n_1, n_2, VdotH, sin(acos(VdotH)));
	float fresnelFace = FresnelDielectric(n_1, n_2, NdotL, sin(acos(NdotL)));
	float fresnelmetal = FresnelMetal(n_1, 5, VdotH);

	if (PBk > 0)
		F = fresnelmetal;
	else
		F = schlicks;
	*/

	// T is the amount of transmitted light
	float F = ComputeFresnel(N, L, V, H, 1.33, 0);
	float T = 1 - F;

	vec3 diffuseColor = ComputePBDiffuse(N, L, V, myKd, T, E0);

	//vec3 wo = V;
	//vec3 wi = L;
	//vec3 wh = V + L;
	//vec wg = N;
	//float wm_dot_wg;
	//float wo_dot_wg;
	//float wo_dot_wm;

	float fKs = 0.0;
	float power = Ns < 1 ? 1 : Ns;
	if (PBk > 0)
		fKs = ComputeCookTorrance(N, L, V, H, F, PBm);
	if (PBKsm > 0)
		fKs = ComputeCookTorrance(N, L, V, H, F, PBKsm);
	else
		fKs = ComputeCookTorrance(N, L, V, H, F, 0.99);//ComputePhong(NdotH, power);

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
		Nmap = PerturbNormal(N, V, 1 - VS_TexCoord);
	}

	vec4 dP1 = dFdx(VS_Position);
	vec4 dP2 = dFdy(VS_Position);
	vec3 fragN = normalize(cross(dP1.xyz, dP2.xyz));
	vec3 fragNmap = fragN;
	if (map_normal_mix > 0.5) {
		fragNmap = PerturbNormal(fragN, V, 1 - VS_TexCoord);
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
		vec3 map = texture(map_Kd, 1 - VS_TexCoord).rgb;
		myKd = (Kd * map);
	}

	vec3 myKs = Ks;
	if (map_Ks_mix > 0.5) 
	{
		vec3 map = texture(map_Ks, 1 - VS_TexCoord).rgb;
		myKs = Ks * map;
	}

	float kspec = pow(max(0.0, dot(N, H)), Ns);

	myKs += spec * vec3(1, 1, 1) * lightArea / (lightDistance2);
	kspec = 0;

	if (SpheresCount > 0)
		myKs = SpheresKe[0].rgb;

	float m;
	if (PBk > 0)
		m = PBm;
	else {
		if (PBmap_mix > 0)
			m = texture(PBmap, 1 - VS_TexCoord).r;
		else
			m = PBKsm;
	}

	float pb_ks_m = 0;
	float pb_kd_m = 0;
	float pb_ior = 0;
	float pb_metalior = 0;
	vec3 pb_metalF;

	if (PBmap_mix > 0)
	{
		vec4 pb = texture(PBmap, 1 - VS_TexCoord);
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

	if (PBk > 0)
	{
		float cos_theta_i = max(0, dot(V, H));
		pb_metalF = vec3(
			FresnelMetal(gold_ior.x, gold_coef.x, cos_theta_i),
			FresnelMetal(gold_ior.y, gold_coef.y, cos_theta_i),
			FresnelMetal(gold_ior.z, gold_coef.z, cos_theta_i)
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
	sunZsamples[0] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, offset)).z ? 0 : 1;
	sunZsamples[1] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, offset)).z ? 0 : 1;
	sunZsamples[2] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, -offset)).z ? 0 : 1;
	sunZsamples[3] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, -offset)).z ? 0 : 1;
	sunZsamples[4] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(offset, 0)).z ? 0 : 1;
	sunZsamples[5] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(-offset, 0)).z ? 0 : 1;
	sunZsamples[6] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(0, -offset)).z ? 0 : 1;
	sunZsamples[7] = sunZ + bias > texture(SunShadowMap, sunCoord.xy + vec2(-0, -offset)).z ? 0 : 1;

	//sunTexMapColor = sunCoord.xyz;	

	float shadowAmount = 1.0;
	if ((sunCoord.z - biasW) > sunZ || sunCoord.z < 0 || sunCoord.z > 1)
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
	float tempDiffuseMix = 1 - tempSpecularMix;

	vec3 enviroN = normalize(tempN);
	vec3 enviroL = normalize(reflect(tempV, enviroN));
	vec3 enviroH = normalize(enviroL + tempV);
	vec3 enviroCubeMapColor = texture(EnviroCubeMap, enviroL).rgb;

	//vec3 sunTexMapColor = sunCoord;//texture(SunColorMap, sunCoord.xy).rgb;//texture(map_Kd, sunCoord.xy).rgb;
	float sunF = ComputeFresnel(Nmap, sunL, V, sunH, pb_ior, pb_metalior);
	float sunT = 1 - sunF;
	float sunShadowR = GetSunShadow();
	

	//L = sunL;

	float E0 = tempE0 * ComputeE0(lightDistance);
	float F = ComputeFresnel(enviroN, L, V, H, pb_ior, pb_ks_m);
	float T = 1 - F;
	// vec3 enviroL = reflect(Nmap, V);
	//float enviroF = ComputeFresnel(Nmap, enviroL, V, enviroH, pb_ior, pb_metalior);
	float enviroF = ComputeFresnel(enviroN, -enviroL, V, enviroH, tempIOR, tempKappa);
	float enviroT = 1 - enviroF;
	vec3 enviroMapColor = texture(EnviroCubeMap, enviroL).rgb;
	float enviroKspec = ComputeCookTorrance(enviroN, enviroL, V, enviroH, enviroF, tempSpecularRoughness);
	float enviroKdiff = ComputeOrenNayer(enviroN, enviroL, V, tempDiffuseRoughness);

	float lightF = ComputeFresnel(tempN, L, V, normalize(L+V), tempIOR, tempKappa);
	float lightT = 1;// - lightF;

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
	vec3 sunColor = ComputePBDiffuse(Nmap, sunL, V, myKd, T, SunSize/4) + ComputePBSpecular(Nmap, sunL, V, SunColor, pb_ks_m, sunF, SunSize/4);
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
	if (PBk > 0)
	{
		float spec_cos_theta_i = max(0.0, dot(V, normalize(L+V)));
		float spec_sin_theta_i = sin(acos(spec_cos_theta_i));

		vec3 pb_metalSpecularF = vec3(
			FresnelMetal(gold_ior.x, gold_coef.x, spec_cos_theta_i),
			FresnelMetal(gold_ior.y, gold_coef.y, spec_cos_theta_i),
			FresnelMetal(gold_ior.z, gold_coef.z, spec_cos_theta_i)
			);
		// vec3 pb_metalSpecularF = vec3(
		// 	FresnelMetal2(1.0, gold_ior.x, gold_coef.x, spec_cos_theta_i, spec_sin_theta_i),
		// 	FresnelMetal2(1.0, gold_ior.y, gold_coef.y, spec_cos_theta_i, spec_sin_theta_i),
		// 	FresnelMetal2(1.0, gold_ior.z, gold_coef.z, spec_cos_theta_i, spec_sin_theta_i)
		// 	);
		float enviro_cos_theta_i = max(0.0, dot(V, normalize(enviroL+V)));
		vec3 pb_metalEnviroF = vec3(
			FresnelMetal(gold_ior.x, gold_coef.x, enviro_cos_theta_i),
			FresnelMetal(gold_ior.y, gold_coef.y, enviro_cos_theta_i),
			FresnelMetal(gold_ior.z, gold_coef.z, enviro_cos_theta_i)
			);
		vec3 metalSpecular = ComputePBMetalSpecular(tempN, L, V, myKs, tempSpecularRoughness, pb_metalSpecularF, tempE0);
		vec3 metalEnviro = ComputePBMetalSpecular(tempN, enviroL, V, enviroCubeMapColor, tempMetalRoughness, pb_metalEnviroF, tempE0);//pb_metalF * E0 * 0.1 * dot(Nmap, L);
		FS_outputColor = vec4(metalSpecular, 1.0);
		//FS_outputColor = vec4(enviroCubeMapColor, 1.0);
		//FS_outputColor = vec4(metalSpecular, 1.0);
		FS_outputColor = vec4(diffuseColor * (1 - aluminumF) + E0*enviroCubeMapColor * aluminumF * NdotL, 1.0);
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
	int debugChoice = 7;

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

		// Sun illumination
		bool inDisc = false;
		if (NdotL > 0.99999) inDisc = true;
		float sunRadiance = length(SunE0.rgb);
		float sunScaleFactor = 1.0 / length(SunE0.rgb);
		float sunF = ComputeFresnel(N, L, V, H, PBior, PBk);
		float sunT = 1.0 - sunF;

		highp vec3 sunE0;
		if (SunDirTo.y < 0.0) {
			sunE0 = vec3(0.0, 0.0, 0.0);
			sunF = 0.0;
			sunT = 0.0;
		}
		else {
			sunE0 = sunScaleFactor * SunE0.rgb;
		}

		vec4 sunDiffuseColor;
		vec4 sunSpecularColor;
		if (inDisc) {
			sunSpecularColor = vec4(ComputePBSpecular(N, L, V, sunE0, PBKsm, sunF, 1.0), 1.0);
		}
		sunDiffuseColor = vec4(sunE0.rgb * ComputePBDiffuse(N, L, V, DeferredPB3.rgb, sunT, 1.0), 1.0);

		// Skybox illumination
		highp vec3 skyboxL = normalize(reflect(-V, N));
		highp vec3 skyboxTexel;
		if (skyboxL.y < 0.0) {
			skyboxTexel = sunScaleFactor * GroundE0.rgb;
		}
		else {
			skyboxTexel = texture(PBSkyCubeMap, skyboxL).rgb;
		}

		float skyboxF = ComputeFresnel(N, skyboxL, V, H, PBior, PBk);
		float skyboxT = 1.0 - skyboxF;

		highp vec4 specularColor;
		highp vec4 diffuseColor;
		if (PBk > 0) {
			float spec_cos_theta_i = max(0.0, dot(V, normalize(skyboxL+V)));
			float spec_sin_theta_i = sin(acos(spec_cos_theta_i));

			vec3 pb_metalSpecularF = vec3(
				FresnelMetal(gold_ior.x, gold_coef.x, spec_cos_theta_i),
				FresnelMetal(gold_ior.y, gold_coef.y, spec_cos_theta_i),
				FresnelMetal(gold_ior.z, gold_coef.z, spec_cos_theta_i)
				);
			specularColor = vec4(ComputePBMetalSpecular(N, skyboxL, V, skyboxTexel, 0.0, pb_metalSpecularF, 1.0), 1.0);
			diffuseColor = vec4(sunE0.rgb * DeferredPB3.rgb * (1.0 - pb_metalSpecularF) * max(0.0, dot(N, skyboxL)), 0.0);// vec4(sunE0.rgb * ComputePBDiffuse(N, L, V, DeferredPB3.rgb, skyboxT, 1.0), 1.0);
		}
		else {
			specularColor =  vec4(ComputePBSpecular(N, skyboxL, V, skyboxTexel, PBKsm, skyboxF, 1.0), 1.0);
			diffuseColor = vec4(sunE0.rgb * ComputePBDiffuse(N, L, V, DeferredPB3.rgb, skyboxT, 1.0), 1.0);
		}		

		DeferredLightBuffer += GetSunShadow() * log(1 + diffuseColor + specularColor + sunDiffuseColor + sunSpecularColor);
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
}
