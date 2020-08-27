#version 330 core

layout (location = 0) in vec2 position;
uniform mat4 projection;
uniform mat4 transformation;

void main()
{
	gl_Position = projection * transformation * vec4(position, 0.0f, 1.0f);
}

