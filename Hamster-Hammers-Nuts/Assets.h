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

	namespace Assets
	{
		extern Asset* meshes;
		extern Asset* skeletons;
		extern Asset* anims;
		extern Bone* bones;
		extern PoseBone* posebones;

		bool LoadAssets(char* name);
	}
}