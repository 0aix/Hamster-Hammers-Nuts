#include "Graphics.h"
#include <SDL.h>
#include <iostream>

namespace Hamster
{
	namespace Graphics
	{
		SDL_Window* window;
		SDL_GLContext context;

		bool Initialize(char* name, int width, int height)
		{
			// Initialize SDL library:
			SDL_Init(SDL_INIT_VIDEO);

			// Ask for an OpenGL context version 3.3, core profile, and enable debugging
			SDL_GL_ResetAttributes();
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

			// Create window
			if (!(window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL)))
			{
				std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
				return false;
			}

			// Create OpenGL context
			if (!(context = SDL_GL_CreateContext(window)))
			{
				SDL_DestroyWindow(window);
				std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
				return false;
			}

#ifdef _WIN32
			// On Windows, load OpenGL extensions
			if (!init_gl_shims())
			{
				std::cerr << "Error initializing shims" << std::endl;
				return false;
			}
#endif

			// Set VSYNC + Late Swap (prevents crazy FPS)
			if (SDL_GL_SetSwapInterval(-1) != 0)
			{
				std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
				if (SDL_GL_SetSwapInterval(1) != 0)
					std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
			}

			return true;
		}

		void Uninitialize()
		{
			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(window);
		}

		void Present()
		{
			SDL_GL_SwapWindow(window);
		}
	}
}