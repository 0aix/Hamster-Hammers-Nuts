#version 330 core

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Color;

out vec2 texCoord;
out vec4 color;

void main()
{
	gl_Position = Position;
	texCoord = TexCoord;
	color = Color;
}