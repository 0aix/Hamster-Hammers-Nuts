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

	StoryScene::StoryScene()
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
		center = glm::vec3(0.0f);
		score = 0;
		add_object("Hamster", "Hamster", glm::vec3(0.0f,0.0f,1.5f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f,1.0f,1.0f));
		direction = "Down";
		//rotateObj("Hamster", 0.5f*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
		add_object("Hammer", "Hammer", glm::vec3(0.0f, 0.0f, 1.5f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		//rotateObj("Hammer", -0.5f*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
		objects["Hammer"].transform.position += hammer_offset;
		add_object("Ground" + std::to_string(ground_count), "Ground", glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f,1.0f,1.0f));
		ground_count++;
		//rotateObj("Ground", 0.5f*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
		add_object("Wall" + std::to_string(wall_count), "Wall", glm::vec3(30.0f, 0.0f, 20.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		wall_count++;
		add_object("Wall" + std::to_string(wall_count), "Wall", glm::vec3(-30.0f, 0.0f, -20.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		wall_count++;
		Object* hamster = &objects["Hamster"];
		Object* hammer = &objects["Hammer"];
		
		//rotateHamster("Up");
		//rotateHamster("Left");
		//rotateObj("Hamster", 0.5f*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
		camera.set(60.0f, 0.2f * M_PI, 1.0f * M_PI, glm::vec3(0.0f, 0.0f, 0.0f));
	}

	bool StoryScene::HandleInput()
	{
		if (Game::event.type == SDL_MOUSEMOTION)
		{
			
		}
		if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_SPACE && stun == 0.0f && !on_ladder)
		{
			hd = 0;
			hv = 20.0f*M_PI;
		}
		return true;
	}

	bool StoryScene::Update()
	{
		std::mt19937 mt_rand((unsigned int)time(NULL));
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
		auto clearAll = [&]() {
			auto kv = objects.begin();
			while (kv != objects.end()) {
				if (kv->first.substr(0, 3) == "Nut" || kv->first.substr(0, 3) == "Log") {
					kv = objects.erase(kv);
				}
				else {
					kv++;
				}
			}
		};
		auto rotateHamster = [&](std::string direction) {
			hamster->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			//rotateObj("Hamster", 0.5f*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
			hammer->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			//rotateObj("Hammer", -0.5f*M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
			float degrees;
			if (direction == "Left") {
				hammer_offset = glm::vec3(-1.0f, 1.0f, 0.5f);
				degrees = 1.5f*M_PI;
				rotateObj("Hammer", hd/180.0*M_PI, glm::vec3(-1.0f, 0.0f, 0.0f));
			}
			else if (direction == "Up") {
				hammer_offset = glm::vec3(1.0f, 1.0f, 0.5f);
				degrees = 1.0f*M_PI;
				rotateObj("Hammer", hd / 180.0*M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
			}
			else if (direction == "Right") {
				hammer_offset = glm::vec3(1.0f, -1.0f, 0.5f);
				degrees = 0.5f*M_PI;
				rotateObj("Hammer", hd / 180.0*M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			else if (direction == "Down") {
				hammer_offset = glm::vec3(-1.0f, -1.0f, 0.5f);
				degrees = 0.0f;
				rotateObj("Hammer", hd / 180.0*M_PI, glm::vec3(0.0f, -1.0f, 0.0f));
			}
			objects["Hammer"].transform.position = hammer_offset + objects["Hamster"].transform.position;
			rotateObj("Hammer", degrees, glm::vec3(0.0f, 0.0f, 1.0f));
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
		//Move
		if (stun == 0.0f && !on_ladder) {
			if (Game::KEYBD_STATE[SDL_SCANCODE_A] && !Game::KEYBD_STATE[SDL_SCANCODE_D] && stun == 0.0f) {
				yv = 5.0f;
				hv = 0.0f;
				hd = 0;
				direction = "Left";
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_A] && Game::KEYBD_STATE[SDL_SCANCODE_D] && stun == 0.0f) {
				yv = -5.0f;
				hv = 0.0f;
				hd = 0;
				direction = "Right";
			}
			else {
				yv = 0.0f;
			}
			if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S] && stun == 0.0f) {
				xv = 5.0f;
				hv = 0.0f;
				hd = 0;
				direction = "Up";
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S] && stun == 0.0f) {
				xv = -5.0f;
				hv = 0.0f;
				hd = 0;
				direction = "Down";
			}
			else {
				xv = 0.0f;
			}
		}
		//Hammering
		if (hv != 0.0f) {
			hd += elapsed*hv;
			if (hd >= 120.0f) {	
				for (auto kv : objects) {
					if (kv.first.substr(0, 3) == "Nut") {
						auto nut_pos = objects[kv.first].transform.position;
						auto ham_pos = objects["Hammer"].transform.position;
						if (direction == "Up" && abs(ham_pos.x-nut_pos.x + 2.0f) <= 1.0f && abs(ham_pos.y - nut_pos.y)<= 1.0f) {
							objects.erase(objects.find(kv.first));
							if (score < max)
								score++;
							break;
						}
						if (direction == "Down" && abs(ham_pos.x - nut_pos.x - 2.0f) <= 1.0f && abs(ham_pos.y - nut_pos.y) <= 1.0f) {
							objects.erase(objects.find(kv.first));
							if (score < max)
								score++;
							break;
						}
						if (direction == "Left" && abs(ham_pos.x - nut_pos.x) <= 1.0f && abs(ham_pos.y - nut_pos.y + 2.0f) <= 1.0f) {
							objects.erase(objects.find(kv.first));
							if (score < max)
								score++;
							break;
						}
						if (direction == "Right" && abs(ham_pos.x - nut_pos.x) <= 1.0f && abs(ham_pos.y - nut_pos.y - 2.0f) <= 1.0f) {
							objects.erase(objects.find(kv.first));
							if (score < max)
								score++;
							break;
						}
					}
				}
				hd = 0.0f;
				hv = 0.0f;
			}
		}
		rotateHamster(direction);
		if (stun != 0.0f) {
			stun -= elapsed;
			if (stun < 0.0f)
				stun = 0.0f;
		}
		for (auto kv : objects) {
			if (kv.first.substr(0,3) == "Log" && objects[kv.first].transform.position.z == 1.0f) {
				float x1 = hamster->transform.position.x+elapsed*xv;
				float x2 = kv.second.transform.position.x;
				float y1 = hamster->transform.position.y+elapsed*yv;
				float y2 = kv.second.transform.position.y;
				if (kv.second.transform.rotation.z <= 0.9f && kv.second.transform.rotation.z >= 0.4f) {
					if (abs(y1 - y2) <= 4.0f && abs(x1 - x2) <= 2.0f) {
						if (windv > 0.0f && hamster->transform.position.y-y2>0.0f) {
							yv = windv;
						}
						else if (windv < 0.0f && hamster->transform.position.y - y2<0.0f) {
							yv = windv;
						}
						else {
							xv = 0.0f;
							yv = 0.0f;
						}
						break;
					}
				}
				else {
					if (abs(y1 - y2) <= 2.0f && abs(x1 - x2) <= 4.0f) {
						if (windv > 0.0f && hamster->transform.position.y - y2>0.0f) {
							yv = windv;
						}
						else if (windv < 0.0f && hamster->transform.position.y - y2<0.0f) {
							yv = windv;
						}
						else {
							xv = 0.0f;
							yv = 0.0f;
						}
						break;
					}
				}
			}
			else if (kv.first.substr(0, 3) == "Log" && objects[kv.first].transform.position.z > 1.0f && objects[kv.first].transform.position.z <= 2.0f) {
				float x1 = hamster->transform.position.x + elapsed*xv;
				float x2 = kv.second.transform.position.x;
				float y1 = hamster->transform.position.y + elapsed*yv;
				float y2 = kv.second.transform.position.y;
				if (kv.second.transform.rotation.z <= 0.9f && kv.second.transform.rotation.z >= 0.4f) {
					if (abs(y1 - y2) <= 4.0f && abs(x1 - x2) <= 2.0f) {
						stun = 1.0f;
						xv = 5.0f;
						yv = 5.0f;
						if (y2 - y1 > 0.0f)
							yv = -5.0;
						if (x2 - x1 > 0.0f)
							xv = -5.0f;
						if (score > 0)
							score--;
						break;
					}
				}
				else {
					if (abs(y1 - y2) <= 2.0f && abs(x1 - x2) <= 4.0f) {
						stun = 1.0f;
						xv = 5.0f;
						yv = 5.0f;
						if (y2 - y1 > 0.0f)
							yv = -5.0;
						if (x2 - x1 > 0.0f)
							xv = -5.0f;
						if (score > 0)
							score--;
						break;
					}
				}
			}
			else if (kv.first.substr(0, 3) == "Nut" &&
				objects[kv.first].transform.position.z >= 1.0f && objects[kv.first].transform.position.z <= 2.0f) {
				float x1 = hamster->transform.position.x + elapsed*xv;
				float x2 = kv.second.transform.position.x;
				float y1 = hamster->transform.position.y + elapsed*yv;
				float y2 = kv.second.transform.position.y;
				if (abs(y1 - y2) <= 1.0f && abs(x1 - x2) <= 1.0f) {
					stun = 1.0f;
					xv = 5.0f;
					yv = 5.0f;
					if (y2 - y1 > 0.0f)
						yv = -5.0;
					if (x2 - x1 > 0.0f)
						xv = -5.0f;
					if (score > 0)
						score--;
					break;
				}
			}
		}
		hawk_time -= elapsed;
		if (!has_hawk && hawk_time == 0.0f) {
			if (mt_rand() % 20 == 0) {
				has_hawk = true;
			}
			if (mt_rand() % 2 == 0) {
				add_object("Hawk", "Hawk", glm::vec3((float)(mt_rand() % 60) - 30.0f, -30.0f, 2.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
				hyv = 10.0f;
			}
			else {
				add_object("Hawk", "Hawk", glm::vec3((float)(mt_rand() % 60) - 30.0f, 30.0f, 2.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
				hyv = -10.0f;
			}
		}
		if (has_hawk) {
			objects["Hawk"].transform.position += glm::vec3(hxv, hyv, hzv);
			if (objects["Hawk"].transform.position.y > 50.0f) {
				objects.erase(objects.find("Hawk"));
				hawk_time = 30.0f;
				has_hawk = false;
			}
			if (abs(objects["Hawk"].transform.position.x - hamster->transform.position.x) < 2.0f && abs(objects["Hawk"].transform.position.y - hamster->transform.position.y) < 2.0f) {
				score = 0;
				xv = hxv;
				yv = hyv;
			}
		}
		if (hamster->transform.position.x + xv*elapsed < -31.0f) {
			on_ladder = true;
			fell = true;
			score = std::max(0, score - 5);
			xv = 0.0f;
			yv = 0.0f;
			zv = -5.0f;
		}
		if (hamster->transform.position.x + xv*elapsed > 29.0f) {
			if (score >= max && abs(hamster->transform.position.y) <= 2.0f && direction == "Up") {
				on_ladder = true;
				score = 0;
				xv = 0.0f;
				yv = 0.0f;
				zv = 5.0f;
			} else
				xv = 0.0f;
			
		}
		if (hamster->transform.position.y + yv*elapsed > 31.0f || hamster->transform.position.y + yv*elapsed < -31.0f) {
			on_ladder = true;
			fell = true;
			score = std::max(0, score - 5);
			xv = 0.0f;
			yv = 0.0f;
			zv = -5.0f;
		}
		hamster->transform.position += glm::vec3(xv*elapsed, yv*elapsed, zv*elapsed);
		hammer->transform.position += glm::vec3(xv*elapsed, yv*elapsed, zv*elapsed);
		if (hamster->transform.position.z <1.5f && hamster->transform.position.z > 0.0f) {
			if (!fell) {
				on_ladder = false;
				if(zv > 0.0f)
					hamster->transform.position += glm::vec3(2.0f, 0.0f, 0.0f);
				hamster->transform.position.z = 1.5f;
				hammer->transform.position = hamster->transform.position + hammer_offset;
				zv = 0.0f;
			}
		}
		if (hamster->transform.position.z < -10.0f) {
			clearAll();
			if (fell)
				fell = false;
			if (abs(hamster->transform.position.y) > 30.0) {
				direction = "Down";
				hamster->transform.position = glm::vec3(29.0, hamster->transform.position.x, 30.0f);
				hammer->transform.position = hamster->transform.position + hammer_offset;
			}
			else {
				hamster->transform.position = glm::vec3(29.0, hamster->transform.position.y, 30.0f);
				hammer->transform.position = hamster->transform.position + hammer_offset;
			}
		}
		if (hamster->transform.position.z > 30.0f) {
			clearAll();
			hamster->transform.position = glm::vec3(-31.0, hamster->transform.position.y, -10.0f);
			hammer->transform.position = hamster->transform.position + hammer_offset;
		}
		//object drops
		next_drop -= elapsed;
		if (next_drop <= 0.0f) {
			next_drop = 1.0f;
			int drop_type = mt_rand() % 3;
			glm::vec3 pos = glm::vec3((float)(mt_rand() % 54) - 27.0f, (float)(mt_rand() % 54) - 27.0f, 30.0f);
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
						glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
					nut_count++;
				}
				else {
					std::string name = "Log" + std::to_string(log_count);
					add_object(name, "Log", pos,
						glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
					int rotation = mt_rand() % 4;
					rotateObj(name, (float)rotation*0.5*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
					log_count++;
				}
			}
		}
		if (windt != 0.0f) {
			windt -= elapsed;
			if (windt < 0.0f) {
				windt = 0.0f;
				windv = 0.0f;
			}
		}
		else {
			int dir = mt_rand() % 3;
			windt = 10.0f;
			windv = dir - 1;
		}
		auto kv = objects.begin();
		while (kv != objects.end()) {
			if (kv->first.substr(0, 3) == "Nut" || kv->first.substr(0, 3) == "Log") {
				glm::vec3 *pos = &objects[kv->first].transform.position;
				pos->z -= 5.0f*elapsed;
				if (pos->z <= 0.5f && kv->first.substr(0, 3) == "Nut" && abs(pos->x) <= center.x + 31.0f && abs(pos->y) <= center.y + 31.0f && pos->z > 0.0f) {
					pos->z = 0.5f;
				}
				if (pos->z <= 1.0f && kv->first.substr(0, 3) == "Log" && abs(pos->x) <= center.x + 31.0f && abs(pos->y) <= center.y + 31.0f && pos->z > 0.0f) {
					pos->z = 1.0f;
				}
				pos->y += windv*elapsed;
				if (pos->z < -1.5)
					kv = objects.erase(objects.find(kv->first));
				else
					kv++;
			}
			else {
				kv++;
			}
		}
		camera.set(60.0f, 0.2f * M_PI, 1.0f * M_PI, center);
		return true;
	}

	void StoryScene::Render()
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