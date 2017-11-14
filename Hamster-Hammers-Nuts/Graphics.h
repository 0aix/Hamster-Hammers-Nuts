#pragma once

#include "GL.hpp"

struct Vertex;

namespace Hamster
{
	namespace Graphics
	{
		extern GLuint basic;
		extern GLuint animated;
		extern GLuint shadow;
		extern GLuint shadow_animated;
		extern GLuint FramebufferName;
		extern GLuint depthTexture;

		bool Initialize(char* name, int width, int height);
		void Uninitialize();

		void LoadBufferData(void* data, int size);

		void Begin();
		void Present();
	}
}