#include "Assets.h"
#include "Mesh.h"

namespace Hamster
{
	Mesh::Mesh(unsigned int meshID)
	{
		vertex_start = Assets::meshes[meshID].start;
		vertex_count = Assets::meshes[meshID].count;
	}
}