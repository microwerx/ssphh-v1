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
#version 100
#ifdef GL_ES
precision highp float;
#endif

// Input from OpenGL ES
uniform samplerCube uCubeTexture;
uniform samplerCube uHosekWilkieTexture;
uniform int uIsHosekWilkie;
uniform int uIsHemisphere;
uniform float ToneMapScale;
uniform float ToneMapGamma;
uniform float ToneMapExposure;

uniform vec3 SunDirTo;	// Direction of the light from the origin of the scene
uniform vec4 SunE0;		// Sun Disk Radiance - vec4(vec3(SunE0), SunMonoE0)
uniform vec4 GroundE0;  // Ground Radiance from the reflected sun.


// Input from Vertex Shader
varying vec4 fsColor;
varying vec3 fsTexCoord;
varying vec3 fsNormal;

vec4 GetPBSky(vec3 L, float spread)
{
	bool inDisc = false;
	bool atNight = false;

	if (SunDirTo.y >= 0.0)
	{
		atNight = false;
	}
	else
	{
		atNight = true;
	}

	float cosine = max(0.0, dot(normalize(L), normalize(SunDirTo)));
	if (cosine > 0.99995)
	{
		inDisc = true;
	} 

	vec4 outputColor = vec4(0.0);
	if (atNight)
	{	
		outputColor = vec4(0.0);
	}
	else
	{
		if (L.y >= 0.0)
		{
			if (inDisc)
			{
				outputColor = SunE0;
			}
			else
			{
				outputColor = textureCube(uCubeTexture, L);
			}
		}
		else
		{
			outputColor = textureCube(uCubeTexture, L);
		}
	}
	return outputColor;
}

void main()
{	
	// exposure and gamma
	vec3 finalColor = GetPBSky(fsTexCoord, 0.0).rgb * ToneMapExposure;
	finalColor = pow(finalColor, vec3(1.0 / ToneMapGamma));	

    gl_FragColor = vec4(finalColor, 1.0);
}
