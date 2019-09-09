#version 300 es
precision highp float;

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

// Input from OpenGL ES
uniform mat4 CameraMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 WorldMatrix;
uniform vec4 CameraPosition;

// Input from OpenGL ES
in vec4 aPosition;
in vec3 aNormal;
in vec4 aColor;
in vec3 aTexCoord;

// Output to Fragment Shader
out vec4 vColor;
out vec3 vNormal;
out vec3 vTexCoord;
out vec3 vViewVector;
out vec4 vPosition;

void main()
{
    vColor = aColor;
    vTexCoord = aTexCoord;
    vNormal = (WorldMatrix * vec4(aNormal, 0.0)).xyz;
    vec4 worldPosition = WorldMatrix * aPosition;
    vPosition = worldPosition;
    vViewVector = CameraPosition.xyz - worldPosition.xyz;
    gl_Position = (ProjectionMatrix * CameraMatrix * worldPosition);
}
