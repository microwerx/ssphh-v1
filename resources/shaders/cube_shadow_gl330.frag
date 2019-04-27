#version 330 core

in vec4 FragPosition;

uniform vec3 LightPosition;
uniform float ZFar;
uniform float ZNear;

void main()
{	
	vec4 dP1 = dFdx(FragPosition);
	vec4 dP2 = dFdy(FragPosition);
	vec3 N = normalize(cross(dP1.xyz, dP2.xyz));
	vec3 L = LightPosition - FragPosition.xyz;
	float Z = length(L);
	L = normalize(L);
	float NdotL = max(0.0, dot(N, L));
	vec3 color = NdotL * vec3(1.0, 1.0, 1.0);
	float distance = Z / ZFar;
	gl_FragData[0] = vec4(color, 1.0);
	gl_FragDepth = Z / ZFar;
}