#include "Scene.h"
#include "Game.h"
#include "Graphics.h"

// temp
#define GLM_ENABLE_EXPERIMENTAL
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include <random>
#include <algorithm>
#include <string>
#include "Meshes.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

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

		scene_meshes = meshes;


		// actual
		current_time = std::chrono::high_resolution_clock::now();
		previous_time = current_time;
		elapsed = 0.0f;
		


		// kinda actual
		objects = std::unordered_map<std::string,Object>();
		glm::vec3 hammer_offset = glm::vec3(-1.0f, -1.0f, -0.75f);

		auto add_object = [&](std::string const &name, glm::vec3 const &position, glm::quat const &rotation, glm::vec3 const &scale) -> Object & {
			Mesh const &mesh = meshes.get(name);
			Object object;
			object.transform.position = position;
			object.transform.rotation = rotation;
			object.transform.scale = scale;
			object.start = mesh.start;
			object.count = mesh.count;
			objects[name] = object;
			return objects[name];
		};

		auto rotateObj = [&](const std::string &name, float degrees, glm::vec3 axis) {
			if (degrees == 0.0f) {
				return;
			}
			auto quart = objects[name].transform.rotation;
			quart = glm::rotate(quart, degrees, axis);
			objects[name].transform.rotation = quart;
		};

		add_object("Hamster", glm::vec3(0.0f,0.0f,1.5f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f,1.0f,1.5f));
		rotateObj("Hamster", 0.5f*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
		add_object("Hammer", glm::vec3(0.0f, 0.0f, 1.5f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 2.0f));
		rotateObj("Hammer", -0.5f*M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
		objects["Hammer"].transform.position += hammer_offset;
		add_object("Ground", glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(40.0f,40.0f,0.0f));
		Object* hamster = &objects["Hamster"];
		Object* hammer = &objects["Hammer"];
		
		//rotateHamster("Up");
		//rotateHamster("Left");
		//rotateObj("Hamster", 0.5f*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
		camera.set(40.0f, 0.3f * M_PI, 1.5f * M_PI, glm::vec3(0.0f, 0.0f, 0.0f));
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
		glm::vec3 hammer_offset = glm::vec3(-1.0f, -1.0f, -0.75f);
		Object* hamster = &objects["Hamster"];
		Object* hammer = &objects["Hammer"];
		auto add_object = [&](std::string const &name, std::string const &mesh_name,
			glm::vec3 const &position, glm::quat const &rotation, glm::vec3 const &scale) -> Object & {
			Mesh const &mesh = scene_meshes.get(mesh_name);
			Object object;
			object.transform.position = position;
			object.transform.rotation = rotation;
			object.transform.scale = scale;
			object.start = mesh.start;
			object.count = mesh.count;
			objects[name] = object;
			return objects[name];
		};

		auto rotateObj = [&](const std::string &name, float degrees, glm::vec3 axis) {
			if (degrees == 0.0f) {
				return;
			}
			auto quart = objects[name].transform.rotation;
			quart = glm::rotate(quart, degrees, axis);
			objects[name].transform.rotation = quart;
		};

		auto rotateHamster = [&](std::string direction) {
			hammer_offset = glm::vec3(-1.0f, -1.0f, -0.75f);
			hamster->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			rotateObj("Hamster", 0.5f*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
			hammer->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			rotateObj("Hammer", -0.5f*M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
			float degrees;
			if (direction == "Left") {
				degrees = 1.5f*M_PI;
			}
			else if (direction == "Up") {
				degrees = 1.0f*M_PI;
			}
			else if (direction == "Right") {
				degrees = 0.5f*M_PI;
			}
			else if (direction == "Down") {
				degrees = 0.0f;
			}
			hammer_offset = glm::rotate(hammer_offset, degrees, glm::vec3(0.0f, 0.0f, 1.0f));
			objects["Hammer"].transform.position = hammer_offset + objects["Hamster"].transform.position;
			rotateObj("Hammer", -degrees, glm::vec3(0.0f, 1.0f, 0.0f));
			rotateObj("Hamster", degrees, glm::vec3(0.0f, 0.0f, 1.0f));
		};

		

		current_time = std::chrono::high_resolution_clock::now();
		elapsed = std::chrono::duration<float>(current_time - previous_time).count();
		previous_time = current_time;
		if (elapsed > 1.0f / 60.0f)
			elapsed = 1.0f / 60.0f;

		if (Game::KEYBD_STATE[SDL_SCANCODE_ESCAPE])
		{
			Game::NextScene(0);
			return false;
		}
		if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S]) {
			yv = 5.0f;
			hv = 0.0f;
			hd = 0;
			direction = "Up";
		}
		else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S]) {
			yv = -5.0f;
			hv = 0.0f;
			hd = 0;
			direction = "Down";
		}
		else {
			yv = 0.0f;
		}
		if (Game::KEYBD_STATE[SDL_SCANCODE_A] && !Game::KEYBD_STATE[SDL_SCANCODE_D]) {
			xv = -5.0f;
			hv = 0.0f;
			hd = 0;
			direction = "Left";
		}
		else if (!Game::KEYBD_STATE[SDL_SCANCODE_A] && Game::KEYBD_STATE[SDL_SCANCODE_D]) {
			xv = 5.0f;
			hv = 0.0f;
			hd = 0;
			direction = "Right";
		}
		else {
			xv = 0.0f;
		}
		if (Game::KEYBD_STATE[SDL_SCANCODE_SPACE]) {
			hd = 0;
			hv = 20.0f*M_PI;
		}
		if (hv != 0.0f) {
			hd += elapsed*hv;
			if (hd >= 90.0f) {
				hd = 90.0f;
				hv = -20.0*M_PI;
			}
			if (hd <= 0.0f) {
				hd = 0.0f;
				hv = 0.0f;
				for (auto kv : objects) {
					if (kv.first.substr(0, 3) == "Nut") {
						auto nut_pos = objects[kv.first].transform.position;
						auto ham_pos = objects["Hammer"].transform.position;
						if (direction == "Up" && abs(ham_pos.x-nut_pos.x) <= 0.75f && abs(ham_pos.y+1-nut_pos.y)<=0.75f) {
							objects.erase(objects.find(kv.first));
							break;
						}
						if (direction == "Down" && abs(ham_pos.x - nut_pos.x) <= 0.75f && abs(ham_pos.y - 1 - nut_pos.y) <= 0.75f) {
							objects.erase(objects.find(kv.first));
							break;
						}
						if (direction == "Left" && abs(ham_pos.x - nut_pos.x - 1) <= 0.75f && abs(ham_pos.y - nut_pos.y) <= 0.75f) {
							objects.erase(objects.find(kv.first));
							break;
						}
						if (direction == "Right" && abs(ham_pos.x - nut_pos.x + 1) <= 0.75f && abs(ham_pos.y - nut_pos.y) <= 0.75f) {
							objects.erase(objects.find(kv.first));
							break;
						}
					}
				}
			}
		}
		rotateHamster(direction);
		rotateObj("Hammer", -hd/180.0*M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
		bool can_move = true;
		for (auto kv : objects) {
			if (kv.first.substr(0,3) == "Log" && objects[kv.first].transform.position.z == 1.0f) {
				float x1 = hamster->transform.position.x+elapsed*xv;
				float x2 = kv.second.transform.position.x;
				float y1 = hamster->transform.position.y+elapsed*yv;
				float y2 = kv.second.transform.position.y;
				if (kv.second.transform.rotation.x <= 0.6f && kv.second.transform.rotation.x >= 0.4f) {
					if (abs(y1 - y2) <= 5.0f && abs(x1 - x2) <= 3.0f) {
						can_move = false;
						break;
					}
				}
				else {
					if (abs(y1 - y2) <= 3.0f && abs(x1 - x2) <= 5.0) {
						can_move = false;
						break;
					}
				}
			}
			else if ((kv.first.substr(0, 3) == "Log"|| kv.first.substr(0, 3) == "Nut")&&
				objects[kv.first].transform.position.z >= 1.5f && objects[kv.first].transform.position.z <= 2.0f) {
				float x1 = hamster->transform.position.x;
				float x2 = kv.second.transform.position.x;
				float y1 = hamster->transform.position.y;
				float y2 = kv.second.transform.position.y;
				xv = 10.0f;
				yv = 10.0f;
				if (y2 - y1 > 0.0f)
					xv = -10.0;
				if (x2 - x1 > 0.0f)
					yv = -10.0f;
			}
		}
		if (can_move) {
			hamster->transform.position += glm::vec3(xv*elapsed, yv*elapsed, 0.0f);
			hammer->transform.position += glm::vec3(xv*elapsed, yv*elapsed, 0.0f);
		}
		next_drop -= elapsed;
		if (next_drop <= 0.0f) {
			next_drop = 5.0f;
			std::mt19937 mt_rand((unsigned int)time(NULL));
			int drop_type = mt_rand() % 3;
			glm::vec3 pos = glm::vec3((float)(mt_rand() % 40) - 20.0f, (float)(mt_rand() % 40) - 20.0f, 20.0f);
			bool can_drop = true;
			for (auto kv : objects) {
				if (kv.first.substr(0,3)=="Log") {
					if (abs(pos.x - kv.second.transform.position.x) <= 3.0f || abs(pos.y - kv.second.transform.position.y)<=3.0f) {
						can_drop = false;
						break;
					}
				}
				else if (kv.first.substr(0, 3) == "Nut") {
					if (abs(pos.x - kv.second.transform.position.x) <= 1.0f || abs(pos.y - kv.second.transform.position.y) <= 1.0f) {
						can_drop = false;
						break;
					}
				}
			}
			if (can_drop) {
				if (drop_type != 0) {
					std::string name = "Nut" + std::to_string(nut_count);
					add_object(name, "Nut", pos,
						glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.4f, 0.4f, 0.5f));
					nut_count++;
				}
				else {
					std::string name = "Log" + std::to_string(log_count);
					add_object(name, "Log", pos,
						glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0, 3.0f));
					rotateObj(name, 0.5*M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
					int rotation = mt_rand() % 4;
					rotateObj(name, (float)rotation*0.5*M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
					log_count++;
				}
			}
		}
		for (auto kv : objects) {
			if (kv.first != "Hamster" && kv.first != "Hammer" && kv.first != "Ground") {
				objects[kv.first].transform.position.z -= 5.0f*elapsed;
				if (objects[kv.first].transform.position.z <= 0.5f && kv.first.substr(0,3)=="Nut") {
					objects[kv.first].transform.position.z = 0.5f;
				}
				if (objects[kv.first].transform.position.z <= 1.0f && kv.first.substr(0, 3) == "Log") {
					objects[kv.first].transform.position.z = 1.0f;
				}
			}
		}
		camera.set(40.0f, 0.3f * M_PI, 1.5f * M_PI, hamster->transform.position);
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

		for (auto kv : objects)
		{
			Object* object = &(kv.second);
			glm::mat4 local_to_world = object->transform.make_local_to_world();

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
			glDrawArrays(GL_TRIANGLES, object->start, object->count);
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