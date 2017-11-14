#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace Hamster
{
	struct Mesh
	{
		Mesh() {}
		Mesh(unsigned int meshID);
		Mesh(unsigned int meshID, unsigned int sknID, unsigned int animID);

		void Play(unsigned int animID);
		void Update(float elapsed);

		// Mesh
		int vertex_start;
		int vertex_count;
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
		bool animated;
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec4 weights;
		glm::uvec4 indices;
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