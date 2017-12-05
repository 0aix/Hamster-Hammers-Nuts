#pragma once

#include "Mesh.h"
#include "TOC.h"
#include "GL.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Hamster
{
	struct Entry
	{
		int start;
		int count;
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

	namespace Assets
	{
		extern Entry* meshes;
		extern Entry* skeletons;
		extern Entry* anims;
		extern Bone* bones;
		extern PoseBone* posebones;
		extern GLuint* textures;

		bool LoadAssets(char* name);
	}
}