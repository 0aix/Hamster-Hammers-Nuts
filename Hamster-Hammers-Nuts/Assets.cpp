#include "Assets.h"
#include "Graphics.h"
#include <fstream>

#define VERTEX_SIZE (3 * 4 + 3 * 4 + 3 * 4 + 2 * 4 + 4 * 4 + 4 * 4)
#define BONE_SIZE (4 + 4 * 3 * 4)
#define POSE_BONE_SIZE (3 * 4 + 4 * 4 + 3 * 4)

namespace Hamster
{
	namespace Assets
	{
		Asset* meshes;
		Asset* skeletons;
		Asset* anims;
		Bone* bones;
		PoseBone* posebones;

		bool LoadAssets(char* name)
		{
			static_assert(sizeof(Vertex) == VERTEX_SIZE, "Vertex is incorrectly packed");
			static_assert(sizeof(Bone) == BONE_SIZE, "Bone is incorrectly packed");
			static_assert(sizeof(PoseBone) == POSE_BONE_SIZE, "PoseBone is incorrectly packed");

			std::ifstream ifs(name, std::ios::binary);

			if (!ifs.is_open())
				return false;

			unsigned int version;
			ifs.read((char*)&version, 4);

			if (version != TOC::VERSION)
				return false;

			unsigned int size;

			// MESH_BUFFER
			ifs.read((char*)&size, 4);
			Vertex* vertices = new Vertex[size];
			ifs.read((char*)vertices, size * sizeof(Vertex));
			Graphics::LoadBufferData(vertices, size * sizeof(Vertex));
			delete[] vertices;

			// MESH
			ifs.read((char*)&size, 4);
			meshes = new Asset[size];
			ifs.read((char*)meshes, size * sizeof(Asset));

			// SKN_BUFFER
			ifs.read((char*)&size, 4);
			bones = new Bone[size];
			ifs.read((char*)bones, size * BONE_SIZE);

			// SKN
			ifs.read((char*)&size, 4);
			skeletons = new Asset[size];
			ifs.read((char*)skeletons, size * sizeof(Asset));

			// ANIM_BUFFER
			ifs.read((char*)&size, 4);
			posebones = new PoseBone[size];
			ifs.read((char*)posebones, size * POSE_BONE_SIZE);

			// ANIM
			ifs.read((char*)&size, 4);
			anims = new Asset[size];
			ifs.read((char*)anims, size * sizeof(Asset));

			ifs.close();
			return true;
		}
	}
}