#include "Graphics.h"
#include "Assets.h"
#include "Mesh.h"
#include <SDL.h>
#include <glm/gtc/type_ptr.hpp>
#include <png.h>
#include <fstream>
#include <iostream>
#include <vector>

namespace Hamster
{
	namespace Graphics
	{
		struct Vert
		{
			Vert(glm::vec2& Position, glm::vec2& TexCoord, glm::vec4& Color) : Position(Position), TexCoord(TexCoord), Color(Color) {}
			glm::vec2 Position;
			glm::vec2 TexCoord;
			glm::vec4 Color;
		};

		struct
		{
			int width;
			int height;
		} viewport;

		SDL_Window* window;
		SDL_GLContext context;

		// programs
		GLuint scene;
		GLuint shadow;
		GLuint composite;
		GLuint sprite;

		GLuint shadowbuffer;
		GLuint shadowmap;

		GLuint scenebuffer;
		GLuint diffusemap;
		GLuint normalmap;
		GLuint depthmap;

		GLuint spritebuffer;

		GLuint vao;
		GLuint compvao;
		GLuint spritevao;

		glm::mat4 m_world_to_camera;
		glm::mat4 m_world_to_clip;
		glm::mat4 m_world_to_light;

		// modified https://github.com/opengl-tutorials/ogl/tree/master/common/shader.cpp
		GLuint LoadShaders(char* vertex_file_path, char* fragment_file_path);

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

			scene = LoadShaders("shaders\\scene.vs", "shaders\\scene.fs");
			shadow = LoadShaders("shaders\\shadow.vs", "shaders\\shadow.fs");
			composite = LoadShaders("shaders\\composite.vs", "shaders\\composite.fs");
			//sprite = LoadShaders("shaders\\sprite.vs", "shaders\\sprite.fs");

			// some code from http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
			// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
			glGenFramebuffers(1, &shadowbuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, shadowbuffer);

			// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
			glGenTextures(1, &shadowmap);
			glBindTexture(GL_TEXTURE_2D, shadowmap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmap, 0);

			// No color buffer is drawn to
			glDrawBuffer(GL_NONE);

			// Always check that our framebuffer is ok
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				return false;
			
			// Scene stuff
			glGenFramebuffers(1, &scenebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, scenebuffer);
			
			glGenTextures(1, &diffusemap);
			glBindTexture(GL_TEXTURE_2D, diffusemap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.width, viewport.height, 0, GL_RGBA, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, diffusemap, 0);
			
			glGenTextures(1, &normalmap);
			glBindTexture(GL_TEXTURE_2D, normalmap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, viewport.width, viewport.height, 0, GL_RGBA, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normalmap, 0);
			
			glGenTextures(1, &depthmap);
			glBindTexture(GL_TEXTURE_2D, depthmap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, viewport.width, viewport.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthmap, 0);
			
			GLenum drawbuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, drawbuffers);
			
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				return false;

			float vertices[] =
			{
				-1.0f, -1.0f,
				-1.0f, 1.0f,
				1.0f, -1.0f,
				1.0f, 1.0f
			};

			// Composite stuff
			GLuint buffer;
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glGenVertexArrays(1, &compvao);
			glBindVertexArray(compvao);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			// Sprite stuff
			//glGenBuffers(1, &spritebuffer);
			//glGenVertexArrays(1, &spritevao);
			//glBindVertexArray(spritevao);
			//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), 0);
			//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (GLbyte*)8);
			//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vert), (GLbyte*)16);
			//glEnableVertexAttribArray(0);
			//glEnableVertexAttribArray(1);
			//glEnableVertexAttribArray(2);

			// Enable/set some GL defaults
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			return true;
		}

		void Uninitialize()
		{
			glDeleteProgram(scene);
			glDeleteProgram(shadow);

			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(window);
		}

		void LoadBufferData(void* data, int size)
		{
			GLuint buffer;
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
			GLenum a = glGetError();
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

		uint32_t m_offset = 0;
		int m_size = 0;
		char* m_data;
		void read_data(png_struct* png_ptr, png_byte* png_data, png_size_t length)
		{
			if (m_offset + length > m_size)
				png_error(png_ptr, "Error reading");
			std::memcpy(png_data, m_data + m_offset, length);
			m_offset += length;
		};

		GLuint LoadTexture(char* data, int size)
		{
			png_struct* png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			png_info* info = png_create_info_struct(png);

			m_offset = 0;
			m_size = size;
			m_data = data;
			png_set_read_fn(png, NULL, read_data);
			png_read_info(png, info);

			// 
			if (png_get_color_type(png, info) == PNG_COLOR_TYPE_PALETTE)
				png_set_palette_to_rgb(png);
			if (png_get_color_type(png, info) == PNG_COLOR_TYPE_GRAY || png_get_color_type(png, info) == PNG_COLOR_TYPE_GRAY_ALPHA)
				png_set_gray_to_rgb(png);
			if (!(png_get_color_type(png, info) & PNG_COLOR_MASK_ALPHA))
				png_set_add_alpha(png, 0xFF, PNG_FILLER_AFTER);
			if (png_get_bit_depth(png, info) < 8)
				png_set_packing(png);
			if (png_get_bit_depth(png, info) == 16)
				png_set_strip_16(png);

			png_read_update_info(png, info);
			
			uint32_t width = png_get_image_width(png, info);
			uint32_t height = png_get_image_height(png, info);

			std::vector<unsigned int*> buffer(width * height);

			png_byte** row_pointers = new png_byte*[height];
			for (int i = 0; i < height; i++)
				//row_pointers[height - i - 1] = (png_byte*)(&buffer[i * width]);
				row_pointers[i] = (png_byte*)(&buffer[i * width]);

			png_read_image(png, row_pointers);
			png_destroy_read_struct(&png, &info, NULL);
			delete[] row_pointers;

			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			return texture;
		}

		void WorldTransforms(glm::mat4& world_to_camera, glm::mat4& world_to_clip, glm::mat4& world_to_light)
		{
			m_world_to_camera = world_to_camera;
			m_world_to_clip = world_to_clip;
			m_world_to_light = world_to_light;
		}

		void BeginShadow()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, shadowbuffer);
			glClear(GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, 1024, 1024);
			glUseProgram(shadow);
			glBindVertexArray(vao);
			//
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

		void RenderShadow(const Object& object)
		{
			static GLint shadow_depth_mvp = glGetUniformLocation(shadow, "depth_mvp");
			static GLint shadow_bones = glGetUniformLocation(shadow, "bones");
			static GLint shadow_animated = glGetUniformLocation(shadow, "animated");

			// compute depth mvp
			glm::mat4 depth_mvp = m_world_to_light * object.transform.make_local_to_world();
			glUniformMatrix4fv(shadow_depth_mvp, 1, GL_FALSE, glm::value_ptr(depth_mvp));

			if (object.animated)
			{
				glUniformMatrix4x3fv(shadow_bones, object.anim.bind_to_world.size(), GL_FALSE, glm::value_ptr(object.anim.bind_to_world[0]));
				glUniform1i(shadow_animated, 1);
				for (auto it = object.anim.mesh.begin(); it != object.anim.mesh.end(); it++)
					glDrawArrays(GL_TRIANGLES, it->vertex_start, it->vertex_count);
			}
			else
			{
				glUniform1i(shadow_animated, 0);
				glDrawArrays(GL_TRIANGLES, object.mesh.vertex_start, object.mesh.vertex_count);
			}
		}

		void BeginScene(glm::vec3& to_light)
		{
			static GLint scene_shadowmap = glGetUniformLocation(scene, "shadowmap");
			static GLint scene_to_light = glGetUniformLocation(scene, "to_light");

			glBindFramebuffer(GL_FRAMEBUFFER, scenebuffer);
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0.5f, 0.5f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, viewport.width, viewport.height);
			glUseProgram(scene);
			glBindVertexArray(vao);
			//
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
			//
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowmap);
			glUniform1i(scene_shadowmap, 0);
			glUniform3fv(scene_to_light, 1, glm::value_ptr(to_light));
		}

		void RenderScene(const Object& object)
		{
			static glm::mat4 bias(0.5f, 0.0f, 0.0f, 0.0f,
								  0.0f, 0.5f, 0.0f, 0.0f,
								  0.0f, 0.0f, 0.5f, 0.0f,
								  0.5f, 0.5f, 0.5f, 1.0f);
			static GLint scene_mvp = glGetUniformLocation(scene, "mvp");
			static GLint scene_depth_bias_mvp = glGetUniformLocation(scene, "depth_bias_mvp");
			static GLint scene_bones = glGetUniformLocation(scene, "bones");
			static GLint scene_itmv = glGetUniformLocation(scene, "itmv");
			static GLint scene_animated = glGetUniformLocation(scene, "animated");

			// compute model + view + projection (object space to clip space) matrix
			glm::mat4 local_to_world = object.transform.make_local_to_world();
			glm::mat4 mvp = m_world_to_clip * local_to_world;

			// compute depth bias mvp
			glm::mat4 depth_bias_mvp = bias * m_world_to_light * local_to_world;

			glUniformMatrix4fv(scene_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
			glUniformMatrix4fv(scene_depth_bias_mvp, 1, GL_FALSE, glm::value_ptr(depth_bias_mvp));

			if (object.animated)
			{
				glUniformMatrix4x3fv(scene_bones, object.anim.bind_to_world.size(), GL_FALSE, glm::value_ptr(object.anim.bind_to_world[0]));
				glUniform1i(scene_animated, 1);
				for (auto it = object.anim.mesh.begin(); it != object.anim.mesh.end(); it++)
					glDrawArrays(GL_TRIANGLES, it->vertex_start, it->vertex_count);
			}
			else
			{
				// compute model view (object space to camera local space) matrix
				glm::mat4 mv = m_world_to_camera * local_to_world;
				// NOTE: inverse cancels out transpose unless there is scale involved
				glm::mat3 itmv = glm::inverse(glm::transpose(glm::mat3(mv)));

				glUniformMatrix3fv(scene_itmv, 1, GL_FALSE, glm::value_ptr(itmv));
				glUniform1i(scene_animated, 0);
				glDrawArrays(GL_TRIANGLES, object.mesh.vertex_start, object.mesh.vertex_count);
			}
		}

		void CompositeScene()
		{
			static GLint m_diffusemap = glGetUniformLocation(composite, "diffusemap");
			static GLint m_normalmap = glGetUniformLocation(composite, "normalmap");
			static GLint m_depthmap = glGetUniformLocation(composite, "depthmap");

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(176.0f / 255.0f, 226.0f / 255.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, viewport.width, viewport.height);
			glUseProgram(composite);
			glBindVertexArray(compvao);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffusemap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalmap);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, depthmap);
			glUniform1i(m_diffusemap, 0);
			glUniform1i(m_normalmap, 1);
			glUniform1i(m_depthmap, 2);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		//void BeginSprite()
		//{
		//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//	glClear(GL_DEPTH_BUFFER_BIT);
		//	glViewport(0, 0, viewport.width, viewport.height);
		//	glUseProgram(sprite);
		//	glBindBuffer(GL_ARRAY_BUFFER, spritebuffer);
		//	glBindVertexArray(spritevao);
		//}
		//
		//void RenderSprite(unsigned int textureID, glm::vec2& center, glm::vec2& size, glm::vec4& color)
		//{
		//	static GLint sprite_tex = glGetUniformLocation(sprite, "tex");
		//	Vert vertices[4] =
		//	{
		//		{ glm::vec2(center.x - 0.5 * size.x, center.y + 0.5 * size.y), glm::vec2(0.0f, 1.0f), color },
		//		{ glm::vec2(center.x + 0.5 * size.x, center.y + 0.5 * size.y), glm::vec2(1.0f, 1.0f), color },
		//		{ glm::vec2(center.x - 0.5 * size.x, center.y - 0.5 * size.y), glm::vec2(0.0f, 0.0f), color },
		//		{ glm::vec2(center.x + 0.5 * size.x, center.y - 0.5 * size.y), glm::vec2(1.0f, 0.0f), color }
		//	};
		//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
		//	glActiveTexture(GL_TEXTURE0);
		//	glBindTexture(GL_TEXTURE_2D, Assets::textures[textureID]);
		//	glUniform1i(sprite_tex, 0);
		//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//}

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