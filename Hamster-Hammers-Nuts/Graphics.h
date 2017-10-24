#pragma once

#include "GL.hpp"

namespace Hamster
{
	namespace Graphics
	{
		bool Initialize(char* name, int width, int height);
		void Uninitialize();
		void Present();

		GLuint LoadProgram(int programID);
	}
}