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
uniform mat4 CameraMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 WorldMatrix;
uniform mat4 ModelViewProjectionMatrix;

// Input from OpenGL ES
attribute vec4 aPosition;
attribute vec4 aColor;
attribute vec3 aTexCoord;
attribute vec3 aNormal;

// Output to Fragment Shader
varying vec4 fsColor;
varying vec3 fsNormal;
varying vec3 fsTexCoord;

void main()
{
        fsColor = aColor;
        fsTexCoord = aTexCoord;
        fsNormal = (WorldMatrix * vec4(aNormal, 0.0)).xyz;
        gl_Position = (ProjectionMatrix * CameraMatrix * WorldMatrix * aPosition);
}
