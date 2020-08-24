#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in mat4 transformation;

void main()
{
	gl_Position = transformation * vec4(position, 0.0f 1.0f);
}

