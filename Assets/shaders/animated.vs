#version 330 core

in vec4 Position;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;
in vec4 BoneWeights;
in uvec4 BoneIndices;

out vec4 position;
out vec3 normal;
out vec3 color;
out vec2 texCoord;
out vec4 shadow;

uniform mat4 mvp;
uniform mat4x3 bones[40]; // make sure this is more than the number of bones
uniform mat4 depth_bias_mvp;

void main()
{
	position = vec4(BoneWeights.x * (bones[BoneIndices.x] * Position) +
				    BoneWeights.y * (bones[BoneIndices.y] * Position) +
				    BoneWeights.z * (bones[BoneIndices.z] * Position) +
				    BoneWeights.w * (bones[BoneIndices.w] * Position), 1.0);
	gl_Position = mvp * position;
	normal = inverse(transpose(BoneWeights.x * mat3(bones[BoneIndices.x]) +
							   BoneWeights.y * mat3(bones[BoneIndices.y]) +
							   BoneWeights.z * mat3(bones[BoneIndices.z]) +
							   BoneWeights.w * mat3(bones[BoneIndices.w]))) * Normal;
	color = Color;
	texCoord = TexCoord;
	shadow = depth_bias_mvp * position;
}