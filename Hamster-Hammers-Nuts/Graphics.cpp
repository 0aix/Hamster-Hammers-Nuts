#include "Graphics.h"
#include "Mesh.h"
#include <SDL.h>
#include <fstream>
#include <iostream>
#include <vector>

namespace Hamster
{
	namespace Graphics
	{
		SDL_Window* window;
		SDL_GLContext context;

		GLuint basic;
		GLuint animated;
		GLuint shadow;
		GLuint shadow_animated;
		GLuint FramebufferName;
		GLuint depthTexture;

		// modified https://github.com/opengl-tutorials/ogl/tree/master/common/shader.cpp
		GLuint LoadShaders(char* vertex_file_path, char* fragment_file_path);

		struct
		{
			int width;
			int height;
		}	viewport;

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

			viewport.width = width;
			viewport.height = height;

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

			basic = LoadShaders("shaders\\basic.vs", "shaders\\basic.fs");
			animated = LoadShaders("shaders\\animated.vs", "shaders\\basic.fs");
			shadow = LoadShaders("shaders\\shadow.vs", "shaders\\shadow.fs");
			shadow_animated = LoadShaders("shaders\\shadow_animated.vs", "shaders\\shadow.fs");

			// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
			glGenFramebuffers(1, &FramebufferName);
			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

			// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
			glGenTextures(1, &depthTexture);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL); //
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); //

			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

			// No color buffer is drawn to.
			glDrawBuffer(GL_NONE);

			// Always check that our framebuffer is ok
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				return false;

			return true;
		}

		void Uninitialize()
		{
			glDeleteProgram(basic);
			glDeleteProgram(animated);

			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(window);
		}

		void LoadBufferData(void* data, int size)
		{
			GLuint buffer;
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

			GLuint vao;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte*)0); // Position
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte*)12); // Normal
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte*)24); // Color
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte*)36); // TexCoord
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte*)44); // BoneWeights
			glVertexAttribIPointer(5, 4, GL_UNSIGNED_INT, sizeof(Vertex), (GLbyte*)60); // BoneIndices
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);
			glEnableVertexAttribArray(5);
		}

		void Begin()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}

		void Present()
		{
			SDL_GL_SwapWindow(window);
		}
		
		// modified https://github.com/opengl-tutorials/ogl/tree/master/common/shader.cpp
		GLuint LoadShaders(char* vertex_file_path, char* fragment_file_path)
		{
			// Create the shaders
			GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
			GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

			// Read the Vertex Shader code from the file
			std::ifstream VertexShaderStream(vertex_file_path, std::ios::in | std::ios::binary);
			std::string VertexShaderCode((std::istreambuf_iterator<char>(VertexShaderStream)), std::istreambuf_iterator<char>());

			// Read the Fragment Shader code from the file
			std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in | std::ios::binary);
			std::string FragmentShaderCode((std::istreambuf_iterator<char>(FragmentShaderStream)), std::istreambuf_iterator<char>());

			GLint Result = GL_FALSE;
			int InfoLogLength;

			// Compile Vertex Shader
			const char * pVertexSource = VertexShaderCode.c_str();
			glShaderSource(VertexShaderID, 1, &pVertexSource, NULL);
			glCompileShader(VertexShaderID);

			// Check Vertex Shader
			glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (Result != GL_TRUE)
			{
				std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
				glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
				printf("%s\n", &VertexShaderErrorMessage[0]);
				throw std::runtime_error("Failed to compile vertex shader.");
			}

			// Compile Fragment Shader
			const char * pFragmentSource = FragmentShaderCode.c_str();
			glShaderSource(FragmentShaderID, 1, &pFragmentSource, NULL);
			glCompileShader(FragmentShaderID);

			// Check Fragment Shader
			glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (Result != GL_TRUE)
			{
				std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
				glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
				printf("%s\n", &FragmentShaderErrorMessage[0]);
				throw std::runtime_error("Failed to compile fragment shader.");
			}

			// Link the program
			GLuint ProgramID = glCreateProgram();
			glAttachShader(ProgramID, VertexShaderID);
			glAttachShader(ProgramID, FragmentShaderID);
			glBindAttribLocation(ProgramID, 0, "Position");
			glBindAttribLocation(ProgramID, 1, "Normal");
			glBindAttribLocation(ProgramID, 2, "Color");
			glBindAttribLocation(ProgramID, 3, "TexCoord");
			glBindAttribLocation(ProgramID, 4, "BoneWeights");
			glBindAttribLocation(ProgramID, 5, "BoneIndices");
			glLinkProgram(ProgramID);

			// Check the program
			glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
			glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (Result != GL_TRUE)
			{
				std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
				glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
				printf("%s\n", &ProgramErrorMessage[0]);
				throw std::runtime_error("Failed to link program");
			}

			glDetachShader(ProgramID, VertexShaderID);
			glDetachShader(ProgramID, FragmentShaderID);

			glDeleteShader(VertexShaderID);
			glDeleteShader(FragmentShaderID);

			return ProgramID;
		}
	}
}