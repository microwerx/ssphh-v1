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