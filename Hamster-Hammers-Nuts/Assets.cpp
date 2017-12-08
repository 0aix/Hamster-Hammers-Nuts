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
		Entry* meshes;
		Entry* skeletons;
		Entry* anims;
		Bone* bones;
		PoseBone* posebones;
		GLuint* textures;
		Sound* sounds;

		bool LoadAssets(char const * name)
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
			meshes = new Entry[size];
			ifs.read((char*)meshes, size * sizeof(Entry));

			// SKN_BUFFER
			ifs.read((char*)&size, 4);
			bones = new Bone[size];
			ifs.read((char*)bones, size * BONE_SIZE);

			// SKN
			ifs.read((char*)&size, 4);
			skeletons = new Entry[size];
			ifs.read((char*)skeletons, size * sizeof(Entry));

			// ANIM_BUFFER
			ifs.read((char*)&size, 4);
			posebones = new PoseBone[size];
			ifs.read((char*)posebones, size * POSE_BONE_SIZE);

			// ANIM
			ifs.read((char*)&size, 4);
			anims = new Entry[size];
			ifs.read((char*)anims, size * sizeof(Entry));

			// TEXTURE_BUFFER
			ifs.read((char*)&size, 4);
			char* buffer = new char[size];
			ifs.read(buffer, size);

			// TEXTURE
			ifs.read((char*)&size, 4);
			Entry* pngs = new Entry[size];
			ifs.read((char*)pngs, size * sizeof(Entry));
			textures = new GLuint[size];
			for (uint32_t i = 0; i < size; i++)
				textures[i] = Graphics::LoadTexture(&buffer[pngs[i].start], pngs[i].count);
			delete[] buffer;
			delete[] pngs;

			// SOUND_BUFFER
			ifs.read((char*)&size, 4);
			buffer = new char[size];
			ifs.read(buffer, size);

			// SOUND
			ifs.read((char*)&size, 4);
			Entry* oggs = new Entry[size];
			ifs.read((char*)oggs, size * sizeof(Entry));
			sounds = new Sound[size];
			for (uint32_t i = 0; i < size; i++)
			{
				if (i == TOC::BGM_OGG)
					Audio::LoadMusic(&sounds[i], &buffer[oggs[i].start], oggs[i].count);
				else
					Audio::LoadChunk(&sounds[i], &buffer[oggs[i].start], oggs[i].count);
			}

			ifs.close();
			return true;
		}
	}
}
