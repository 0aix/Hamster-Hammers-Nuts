#pragma once

#include "Assets.h"
#include "Object.h"
#include "GL.hpp"
#include <glm/glm.hpp>

namespace Hamster
{
	namespace Graphics
	{
		bool Initialize(char const* name, int width, int height);
		void Uninitialize();

		void LoadBufferData(void* data, int size);
		GLuint LoadTexture(char const* data, int size);

		void WorldTransforms(glm::mat4 const & world_to_camera, glm::mat4 const & world_to_clip, glm::mat4 const & world_to_light);

		void BeginShadow();
		void RenderShadow(const Object& object);

		void BeginScene(glm::vec3 const & to_light);
		void RenderScene(const Object& object, float alpha = 1.0f);
		void CompositeScene();

		void BeginSprite();
		void RenderSprite(unsigned int textureID, glm::vec4 const & pos, glm::vec4 const & uv = glm::vec4(0.0f, 1.0f, 1.0f, 0.0f), glm::vec4 const & color = glm::vec4(1.0f));

		void Present();
	}
}
