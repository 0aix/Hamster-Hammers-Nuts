#version 330 core

layout(location = 0) in vec4 Position;
layout(location = 4) in vec4 BoneWeights;
layout(location = 5) in uvec4 BoneIndices;

uniform mat4 depth_mvp;
uniform mat4x3 bones[40]; // make sure this is more than the number of bones
uniform bool animated;

void main()
{
	if (animated)
		gl_Position = depth_mvp * vec4(BoneWeights.x * (bones[BoneIndices.x] * Position) +
									   BoneWeights.y * (bones[BoneIndices.y] * Position) +
									   BoneWeights.z * (bones[BoneIndices.z] * Position) +
									   BoneWeights.w * (bones[BoneIndices.w] * Position), 1.0);
	else
		gl_Position = depth_mvp * Position;
}