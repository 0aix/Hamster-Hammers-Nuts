#version 330 core

in vec4 Position;
in vec3 Normal;
in vec3 Color;

out vec4 position;
out vec3 normal;
out vec3 color;
out vec4 shadow;

uniform mat4 mvp;
uniform mat3 itmv;
uniform mat4 depth_bias_mvp;

void main()
{
	gl_Position = mvp * Position;
	position = Position;
	normal = itmv * Normal;
	color = Color;
	shadow = depth_bias_mvp * Position;
}