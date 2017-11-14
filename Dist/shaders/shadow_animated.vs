#version 330 core

in vec4 Position;
in vec4 BoneWeights;
in uvec4 BoneIndices;

uniform mat4 depthMVP;
uniform mat4x3 bones[40]; // make sure this is more than the number of bones

void main()
{
	gl_Position = depthMVP * vec4(BoneWeights.x * (bones[BoneIndices.x] * Position) +
								  BoneWeights.y * (bones[BoneIndices.y] * Position) +
								  BoneWeights.z * (bones[BoneIndices.z] * Position) +
								  BoneWeights.w * (bones[BoneIndices.w] * Position), 1.0);
}