#pragma once

#include "GL.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace Hamster
{
	struct Mesh
	{
		Mesh() {}
		Mesh(unsigned int meshID);

		int vertex_start;
		int vertex_count;
		GLuint texture;
	};

	enum class AnimationState
	{
		PLAYING,
		REPEAT,
		FINISHED
	};

	struct Animation
	{
		Animation() {};
		Animation(unsigned int sknID, unsigned int animID, bool repeat = true);

		void Play(unsigned int animID, bool repeat = true, bool cont = true);
		void Update(float elapsed);

		// Meshes
		std::vector<Mesh> mesh;

		// Skeleton
		int bone_start;
		int bone_count;

		// Animation
		int frame_start;
		int frame_count;
		int frame_number;
		float frame_time;

		std::vector<glm::mat4x3> bone_to_world;
		std::vector<glm::mat4x3> bind_to_world;

		AnimationState state;
		unsigned int anim;
	};

	struct Bone
	{
		int parent = -1;
		glm::mat4x3 inverse_bind_matrix;
	};

	struct PoseBone
	{
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
	};
}