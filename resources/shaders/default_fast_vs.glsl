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
#version 400

// Uniforms
uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 CameraMatrix;

// Inputs

in vec3 vtxPosition;

// Outputs

out vec4 VS_Position;

// Main Function

void main(void)
{
//	mat4 ModelViewProjectionMatrix = mat4(
//		1.0, 0.0, 0.0, 0.0,
//		0.0, 1.0, 0.0, 0.0,
//		0.0, 0.0, -1.0, -2.0,
//		0.0, 0.0, -1.0, 0.0);
	VS_Position = ModelViewMatrix * vec4(vtxPosition, 1);
	gl_Position = ProjectionMatrix * CameraMatrix * ModelViewMatrix * vec4(vtxPosition, 1);
}
