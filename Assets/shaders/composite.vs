#version 330 core

layout(location = 0) in vec2 position;

out vec2 uv;

void main()
{
    gl_Position = vec4(vec3(position, 0.0), 1.0);
    uv = (position + vec2(1.0, 1.0)) / 2.0;
}