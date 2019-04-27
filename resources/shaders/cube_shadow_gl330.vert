#version 330 core

layout(location = 0) in vec3 aPosition;

out vec4 FragPosition;

uniform mat4 WorldMatrix;

void main()
{
	FragPosition = WorldMatrix * vec4(aPosition, 1.0);
}