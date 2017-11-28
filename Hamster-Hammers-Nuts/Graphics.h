#pragma once

#include "Object.h"
#include "GL.hpp"
#include <glm/glm.hpp>

namespace Hamster
{
	namespace Graphics
	{
		bool Initialize(char* name, int width, int height);
		void Uninitialize();
		void LoadBufferData(void* data, int size);

		void WorldTransforms(glm::mat4& world_to_camera, glm::mat4& world_to_clip, glm::mat4& world_to_light);

		void BeginShadow();
		void RenderShadow(const Object& object);

		void BeginScene(glm::vec3& to_light);
		void RenderScene(const Object& object);
		
		void Present();
	}
}