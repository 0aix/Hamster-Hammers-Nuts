#include "Assets.h"
#include "Mesh.h"

namespace Hamster
{
	Mesh::Mesh(unsigned int meshID)
	{
		vertex_start = Assets::meshes[meshID].start;
		vertex_count = Assets::meshes[meshID].count;
		animated = false;
	}

	Mesh::Mesh(unsigned int meshID, unsigned int sknID, unsigned int animID)
	{
		vertex_start = Assets::meshes[meshID].start;
		vertex_count = Assets::meshes[meshID].count;
		bone_start = Assets::skeletons[sknID].start;
		bone_count = Assets::skeletons[sknID].count;
		animated = true;
		Play(animID);
	}

	void Mesh::Play(unsigned int animID)
	{
		if (animated)
		{
			frame_start = Assets::anims[animID].start;
			frame_count = Assets::anims[animID].count;
			frame_number = 0;
			frame_time = 0.0f;
			bone_to_world = std::vector<glm::mat4x3>(bone_count);
			bind_to_world = std::vector<glm::mat4x3>(bone_count);

			for (int i = 0; i < bone_count; i++)
			{
				const PoseBone& pose_bone = Assets::posebones[frame_start + i];
				const Bone& bone = Assets::bones[bone_start + i];

				glm::mat3 r = glm::mat3_cast(pose_bone.rotation);
				glm::mat3 rs = glm::mat3(r[0] * pose_bone.scale.x, r[1] * pose_bone.scale.y, r[2] * pose_bone.scale.z);
				glm::mat4x3 trs = glm::mat4x3(rs[0], rs[1], rs[2], pose_bone.position);

				if (bone.parent == -1)
					bone_to_world[i] = trs;
				else
					bone_to_world[i] = bone_to_world[bone.parent] * glm::mat4(trs);

				bind_to_world[i] = bone_to_world[i] * glm::mat4(bone.inverse_bind_matrix);
			}
		}
	}

	void Mesh::Update(float elapsed)
	{
		bool update = false;

		frame_time += elapsed;
		while (frame_time >= 1.0f / 24.0f) // 24 FPS (?)
		{
			if (frame_number < frame_count - 1)
				frame_number++;
			else
				frame_number = 0;

			frame_time -= 1.0f / 24.0f;
			//update = true;
		}

		//if (!update)
		//	return;

		int offset = frame_number * bone_count;
		int offset2 = (frame_number < frame_count - 1 ? frame_number + 1 : 0) * bone_count;
		bone_to_world = std::vector<glm::mat4x3>(bone_count);
		bind_to_world = std::vector<glm::mat4x3>(bone_count);
		for (int i = 0; i < bone_count; i++)
		{
			const PoseBone& pose_bone = Assets::posebones[frame_start + offset + i];
			const PoseBone& next_bone = Assets::posebones[frame_start + offset2 + i];
			const Bone& bone = Assets::bones[bone_start + i];
			
			// Linear interpolation
			float factor = frame_time / (1.0f / 24.0f);
			auto rotation = glm::slerp(pose_bone.rotation, next_bone.rotation, factor);
			auto scale = glm::mix(pose_bone.scale, next_bone.scale, factor);
			auto position = glm::mix(pose_bone.position, next_bone.position, factor);

			glm::mat3 r = glm::mat3_cast(rotation);
			glm::mat3 rs = glm::mat3(r[0] * scale.x, r[1] * scale.y, r[2] * scale.z);
			glm::mat4x3 trs = glm::mat4x3(rs[0], rs[1], rs[2], position);

			//glm::mat3 r = glm::mat3_cast(pose_bone.rotation);
			//glm::mat3 rs = glm::mat3(r[0] * pose_bone.scale.x, r[1] * pose_bone.scale.y, r[2] * pose_bone.scale.z);
			//glm::mat4x3 trs = glm::mat4x3(rs[0], rs[1], rs[2], pose_bone.position);

			if (bone.parent == -1)
				bone_to_world[i] = trs;
			else
				bone_to_world[i] = bone_to_world[bone.parent] * glm::mat4(trs);

			bind_to_world[i] = bone_to_world[i] * glm::mat4(bone.inverse_bind_matrix);
		}
	}
}