#pragma once

#include "Mesh.h"
#include "TOC.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Hamster
{
	// Not really an asset :S
	struct Asset
	{
		int start;
		int count;
	};

	// Not actually necessary :S
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec4 weights;
		glm::uvec4 indices;
	};

	namespace Assets
	{
		extern Asset* meshes;
		extern Asset* skeletons;
		extern Asset* anims;
		extern Asset* textures;
		extern Bone* bones;
		extern PoseBone* posebones;

		bool LoadAssets(char* name);
	}
}