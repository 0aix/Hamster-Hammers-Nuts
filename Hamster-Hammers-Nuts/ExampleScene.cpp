#include "Scene.h"
#include "Game.h"
#include "Graphics.h"

// temp
#include <string>
#include <iostream>
#include <vector>
#include "Meshes.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace Hamster
{
	// temp
	static GLuint compile_shader(GLenum type, std::string const &source);
	static GLuint link_program(GLuint fragment_shader, GLuint vertex_shader);

	ExampleScene::ExampleScene()
	{
		GLuint program = 0;
		GLint program_Position = 0;
		GLint program_Normal = 0;
		GLint program_Color = 0;
		// compile shader program:
		GLint vertex_shader = compile_shader(GL_VERTEX_SHADER,
			"#version 330\n"
			"uniform mat4 mvp;\n"
			"uniform mat3 itmv;\n"
			"in vec4 Position;\n"
			"in vec3 Normal;\n"
			"in vec3 Color;\n"
			"out vec3 normal;\n"
			"out vec3 color;\n"
			"void main() {\n"
			"	gl_Position = mvp * Position;\n"
			"	normal = itmv * Normal;\n"
			"	color = Color;\n"
			"}\n"
		);

		GLint fragment_shader = compile_shader(GL_FRAGMENT_SHADER,
			"#version 330\n"
			"uniform vec3 to_light;\n"
			"in vec3 normal;\n"
			"in vec3 color;\n"
			"out vec4 fragColor;\n"
			"void main() {\n"
			"	float nl = dot(normalize(normal), to_light);\n"
			"   vec3 ambience = color * 0.5f;\n"
			"	fragColor = vec4(ambience + (color / 3.1415926) * 1.5f * (smoothstep(0.0, 0.1, nl) * 0.6 + 0.4), 1.0);\n"
			"}\n"
		);

		program = link_program(fragment_shader, vertex_shader);

		//look up attribute locations:
		program_Position = glGetAttribLocation(program, "Position");
		if (program_Position == -1) throw std::runtime_error("no attribute named Position");
		program_Normal = glGetAttribLocation(program, "Normal");
		if (program_Normal == -1) throw std::runtime_error("no attribute named Normal");
		program_Color = glGetAttribLocation(program, "Color");
		if (program_Color == -1) throw std::runtime_error("no attribute named Color");

		//look up uniform locations:
		program_mvp = glGetUniformLocation(program, "mvp");
		if (program_mvp == -1) throw std::runtime_error("no uniform named mvp");
		program_itmv = glGetUniformLocation(program, "itmv");
		if (program_itmv == -1) throw std::runtime_error("no uniform named itmv");

		program_to_light = glGetUniformLocation(program, "to_light");
		if (program_to_light == -1) throw std::runtime_error("no uniform named to_light");

		glUseProgram(program);

		Meshes meshes;
		
		// add meshes to database:
		Meshes::Attributes attributes;
		attributes.Position = program_Position;
		attributes.Normal = program_Normal;
		attributes.Color = program_Color;

		meshes.load("meshes.blob", attributes);




		// actual
		current_time = std::chrono::high_resolution_clock::now();
		previous_time = current_time;
		elapsed = 0.0f;
		
		// kinda actual
		objects = new Object[1];
		const Mesh& mesh = meshes.get("Table");
		objects[0].start = mesh.start;
		objects[0].count = mesh.count;

		camera.set(5.0f, 1.2f, 0.5f * M_PI, glm::vec3(0.0f, 0.0f, 0.0f));
	}

	bool ExampleScene::HandleInput()
	{
		if (Game::event.type == SDL_MOUSEMOTION)
		{
			
		}
		return true;
	}

	bool ExampleScene::Update()
	{
		current_time = std::chrono::high_resolution_clock::now();
		elapsed += std::chrono::duration<float>(current_time - previous_time).count();
		previous_time = current_time;

		if (elapsed > 1.0f / 60.0f)
			elapsed = 1.0f / 60.0f;

		if (Game::KEYBD_STATE[SDL_SCANCODE_ESCAPE])
		{
			Game::NextScene(0);
			return false;
		}
		return true;
	}

	void ExampleScene::Render()
	{
		glClearColor(0.5, 0.5, 0.5, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glm::mat4 world_to_camera = camera.transform.make_world_to_local();
		glm::mat4 world_to_clip = camera.make_projection() * world_to_camera;
		glm::vec3 light_in_camera = glm::mat3(world_to_camera) * glm::vec3(0.0f, 0.0f, 1.0f);

		glUniform3fv(program_to_light, 1, glm::value_ptr(glm::normalize(light_in_camera)));

		for (int i = 0; i < 1; i++)
		{
			glm::mat4 local_to_world = objects[i].transform.make_local_to_world();

			//compute modelview+projection (object space to clip space) matrix for this object:
			glm::mat4 mvp = world_to_clip * local_to_world;

			//compute modelview (object space to camera local space) matrix for this object:
			glm::mat4 mv = world_to_camera * local_to_world;

			//NOTE: inverse cancels out transpose unless there is scale involved
			glm::mat3 itmv = glm::inverse(glm::transpose(glm::mat3(mv)));

			//set up program uniforms:
			glUniformMatrix4fv(program_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
			glUniformMatrix3fv(program_itmv, 1, GL_FALSE, glm::value_ptr(itmv));

			//draw the object:
			glDrawArrays(GL_TRIANGLES, objects[i].start, objects[i].count);
		}

		Graphics::Present();
	}

	static GLuint compile_shader(GLenum type, std::string const &source) {
		GLuint shader = glCreateShader(type);
		GLchar const *str = source.c_str();
		GLint length = source.size();
		glShaderSource(shader, 1, &str, &length);
		glCompileShader(shader);
		GLint compile_status = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
		if (compile_status != GL_TRUE) {
			std::cerr << "Failed to compile shader." << std::endl;
			GLint info_log_length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
			std::vector< GLchar > info_log(info_log_length, 0);
			GLsizei length = 0;
			glGetShaderInfoLog(shader, info_log.size(), &length, &info_log[0]);
			std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
			glDeleteShader(shader);
			throw std::runtime_error("Failed to compile shader.");
		}
		return shader;
	}

	static GLuint link_program(GLuint fragment_shader, GLuint vertex_shader) {
		GLuint program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);
		GLint link_status = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &link_status);
		if (link_status != GL_TRUE) {
			std::cerr << "Failed to link shader program." << std::endl;
			GLint info_log_length = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
			std::vector< GLchar > info_log(info_log_length, 0);
			GLsizei length = 0;
			glGetProgramInfoLog(program, info_log.size(), &length, &info_log[0]);
			std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
			throw std::runtime_error("Failed to link program");
		}
		return program;
	}
}