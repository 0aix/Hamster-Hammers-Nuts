#include "Assets.h"
#include "Scene.h"
#include "Game.h"
#include "Graphics.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>
#include <string>
#include <ctime>
#include <random>
#include <algorithm>

namespace Hamster
{
	StoryScene::StoryScene() : Scene()
	{
		AddObject("Hamster", TOC::HAMSTER_MESH, glm::vec3(0.0f, 0.0f, 1.5f));
		direction = Direction::Down;

		AddObject("Hammer", TOC::HAMMER_MESH, glm::vec3(0.0f, 0.0f, 1.5f) + hammer_offset);

		AddObject("Ground" + std::to_string(ground_count++), TOC::GROUND_MESH, glm::vec3(0.0f, 0.0f, 0.0f));
		AddObject("Wall" + std::to_string(wall_count++), TOC::WALL_MESH, glm::vec3(30.0f, 0.0f, 20.0f));
		AddObject("Wall" + std::to_string(wall_count++), TOC::WALL_MESH, glm::vec3(-30.0f, 0.0f, -20.0f));

		camera.set(100.0f, 0.2f * M_PI, 1.0f * M_PI, glm::vec3(0.0f, 0.0f, 0.0f));

		nutcounter.mesh = Mesh(TOC::NUT_MESH);
		nutcounter.transform.position = glm::vec3(-40.45f - 11.76f, 0.0f, 29.39f - 16.18f);
	}

	bool StoryScene::HandleInput()
	{
		if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_SPACE && stun == 0.0f && !on_ladder)
		{
			hd = 0;
			hv = 20.0f * M_PI;
		}
		return true;
	}

	bool StoryScene::Update()
	{
		static std::mt19937 mt_rand((unsigned int)time(NULL));
		static Object* hamster = &objects["Hamster"];
		static Object* hammer = &objects["Hammer"];

		auto clearAll = [&]()
		{
			auto kv = objects.begin();
			while (kv != objects.end())
			{
				if (kv->first.substr(0, 3) == "Nut" || kv->first.substr(0, 3) == "Log")
					kv = objects.erase(kv);
				else
					kv++;
			}
		};
		auto rotateHamster = [&](Direction dir)
		{
			hamster->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			hammer->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

			float degrees;
			switch (dir)
			{
			case Direction::Left:
				hammer_offset = glm::vec3(-1.0f, 1.0f, 0.5f);
				degrees = 1.5f * M_PI;
				RotateObject("Hammer", hd / 180.0f * M_PI, glm::vec3(-1.0f, 0.0f, 0.0f));
				break;
			case Direction::Up:
				hammer_offset = glm::vec3(1.0f, 1.0f, 0.5f);
				degrees = 1.0f * M_PI;
				RotateObject("Hammer", hd / 180.0f * M_PI, glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case Direction::Right:
				hammer_offset = glm::vec3(1.0f, -1.0f, 0.5f);
				degrees = 0.5f * M_PI;
				RotateObject("Hammer", hd / 180.0f * M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
				break;
			case Direction::Down:
				hammer_offset = glm::vec3(-1.0f, -1.0f, 0.5f);
				degrees = 0.0f;
				RotateObject("Hammer", hd / 180.0f * M_PI, glm::vec3(0.0f, -1.0f, 0.0f));
				break;
			}
			objects["Hammer"].transform.position = objects["Hamster"].transform.position + hammer_offset;
			RotateObject("Hammer", degrees, glm::vec3(0.0f, 0.0f, 1.0f));
			RotateObject("Hamster", degrees, glm::vec3(0.0f, 0.0f, 1.0f));
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

		// Move
		if (stun == 0.0f && !on_ladder)
		{
			if (Game::KEYBD_STATE[SDL_SCANCODE_A] && !Game::KEYBD_STATE[SDL_SCANCODE_D] && stun == 0.0f) 
			{
				yv = 5.0f;
				hv = 0.0f;
				hd = 0;
				direction = Direction::Left;
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_A] && Game::KEYBD_STATE[SDL_SCANCODE_D] && stun == 0.0f)
			{
				yv = -5.0f;
				hv = 0.0f;
				hd = 0;
				direction = Direction::Right;
			}
			else
				yv = 0.0f;

			if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S] && stun == 0.0f)
			{
				xv = 5.0f;
				hv = 0.0f;
				hd = 0;
				direction = Direction::Up;
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S] && stun == 0.0f)
			{
				xv = -5.0f;
				hv = 0.0f;
				hd = 0;
				direction = Direction::Down;
			}
			else
				xv = 0.0f;
		}

		// Hammering
		if (hv != 0.0f)
		{
			hd += elapsed*hv;
			if (hd >= 120.0f)
			{	
				for (auto kv : objects)
				{
					if (kv.first.substr(0, 3) == "Nut")
					{
						auto nut_pos = objects[kv.first].transform.position;
						auto ham_pos = objects["Hammer"].transform.position;
						if (direction == Direction::Up && abs(ham_pos.x - nut_pos.x + 2.0f) <= 1.0f && abs(ham_pos.y - nut_pos.y) <= 1.0f)
						{
							objects.erase(objects.find(kv.first));
							if (score < max)
								score++;
							break;
						}
						if (direction == Direction::Down && abs(ham_pos.x - nut_pos.x - 2.0f) <= 1.0f && abs(ham_pos.y - nut_pos.y) <= 1.0f)
						{
							objects.erase(objects.find(kv.first));
							if (score < max)
								score++;
							break;
						}
						if (direction == Direction::Left && abs(ham_pos.x - nut_pos.x) <= 1.0f && abs(ham_pos.y - nut_pos.y + 2.0f) <= 1.0f)
						{
							objects.erase(objects.find(kv.first));
							if (score < max)
								score++;
							break;
						}
						if (direction == Direction::Right && abs(ham_pos.x - nut_pos.x) <= 1.0f && abs(ham_pos.y - nut_pos.y - 2.0f) <= 1.0f)
						{
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
		if (stun != 0.0f)
		{
			stun -= elapsed;
			if (stun < 0.0f)
				stun = 0.0f;
		}
		for (auto kv : objects)
		{
			if (kv.first.substr(0,3) == "Log" && objects[kv.first].transform.position.z == 1.0f)
			{
				float x1 = hamster->transform.position.x+elapsed*xv;
				float x2 = kv.second.transform.position.x;
				float y1 = hamster->transform.position.y+elapsed*yv;
				float y2 = kv.second.transform.position.y;
				if (kv.second.transform.rotation.z <= 0.9f && kv.second.transform.rotation.z >= 0.4f)
				{
					if (abs(y1 - y2) <= 4.0f && abs(x1 - x2) <= 2.0f)
					{
						if (windv > 0.0f && hamster->transform.position.y - y2 > 0.0f)
							yv = windv;
						else if (windv < 0.0f && hamster->transform.position.y - y2 < 0.0f)
							yv = windv;
						else
						{
							xv = 0.0f;
							yv = 0.0f;
						}
						break;
					}
				}
				else
				{
					if (abs(y1 - y2) <= 2.0f && abs(x1 - x2) <= 4.0f)
					{
						if (windv > 0.0f && hamster->transform.position.y - y2 > 0.0f)
							yv = windv;
						else if (windv < 0.0f && hamster->transform.position.y - y2 < 0.0f)
							yv = windv;
						else
						{
							xv = 0.0f;
							yv = 0.0f;
						}
						break;
					}
				}
			}
			else if (kv.first.substr(0, 3) == "Log" && objects[kv.first].transform.position.z > 1.0f && objects[kv.first].transform.position.z <= 2.0f)
			{
				float x1 = hamster->transform.position.x + elapsed*xv;
				float x2 = kv.second.transform.position.x;
				float y1 = hamster->transform.position.y + elapsed*yv;
				float y2 = kv.second.transform.position.y;
				if (kv.second.transform.rotation.z <= 0.9f && kv.second.transform.rotation.z >= 0.4f)
				{
					if (abs(y1 - y2) <= 4.0f && abs(x1 - x2) <= 2.0f)
					{
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
				else
				{
					if (abs(y1 - y2) <= 2.0f && abs(x1 - x2) <= 4.0f)
					{
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
			else if (kv.first.substr(0, 3) == "Nut" && objects[kv.first].transform.position.z >= 1.0f && objects[kv.first].transform.position.z <= 2.0f)
			{
				float x1 = hamster->transform.position.x + elapsed*xv;
				float x2 = kv.second.transform.position.x;
				float y1 = hamster->transform.position.y + elapsed*yv;
				float y2 = kv.second.transform.position.y;
				if (abs(y1 - y2) <= 1.0f && abs(x1 - x2) <= 1.0f)
				{
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
		if (!has_hawk && hawk_time <= 0.0f)
		{
			if (mt_rand() % 20 == 0)
			{
				has_hawk = true;
				if (mt_rand() % 2 == 0)
				{
					AddObject("Hawk", TOC::HAWK_MESH, glm::vec3((float)(mt_rand() % 60) - 30.0f, -30.0f, 2.0f));
					hyv = 10.0f;
				}
				else
				{
					AddObject("Hawk", TOC::HAWK_MESH, glm::vec3((float)(mt_rand() % 60) - 30.0f, 30.0f, 2.0f));
					hyv = -10.0f;
				}
			}
		}
		if (has_hawk)
		{
			Object& hawk = objects["Hawk"];
			hawk.transform.position += glm::vec3(hxv, hyv, hzv);
			if (hawk.transform.position.y > 50.0f)
			{
				objects.erase(objects.find("Hawk"));
				hawk_time = 30.0f;
				has_hawk = false;
			}
			if (abs(hawk.transform.position.x - hamster->transform.position.x) < 2.0f && 
				abs(hawk.transform.position.y - hamster->transform.position.y) < 2.0f)
			{
				score = 0;
				xv = hxv;
				yv = hyv;
			}
		}
		if (hamster->transform.position.x + xv * elapsed < -31.0f)
		{
			on_ladder = true;
			fell = true;
			score = std::max(0, score - 5);
			xv = 0.0f;
			yv = 0.0f;
			zv = -5.0f;
		}
		if (hamster->transform.position.x + xv * elapsed > 29.0f)
		{
			if (score >= max && abs(hamster->transform.position.y) <= 2.0f && direction == Direction::Up)
			{
				on_ladder = true;
				score = 0;
				xv = 0.0f;
				yv = 0.0f;
				zv = 5.0f;
			}
			else
				xv = 0.0f;
			
		}
		if (hamster->transform.position.y + yv * elapsed > 31.0f || hamster->transform.position.y + yv * elapsed < -31.0f)
		{
			on_ladder = true;
			fell = true;
			score = std::max(0, score - 5);
			xv = 0.0f;
			yv = 0.0f;
			zv = -5.0f;
		}
		hamster->transform.position += glm::vec3(xv * elapsed, yv * elapsed, zv * elapsed);
		hammer->transform.position += glm::vec3(xv * elapsed, yv * elapsed, zv * elapsed);
		if (hamster->transform.position.z < 1.5f && hamster->transform.position.z > 0.0f)
		{
			if (!fell)
			{
				on_ladder = false;
				if(zv > 0.0f)
					hamster->transform.position += glm::vec3(2.0f, 0.0f, 0.0f);
				hamster->transform.position.z = 1.5f;
				hammer->transform.position = hamster->transform.position + hammer_offset;
				zv = 0.0f;
			}
		}
		if (hamster->transform.position.z < -10.0f)
		{
			clearAll();
			if (fell)
				fell = false;
			if (abs(hamster->transform.position.y) > 30.0)
			{
				direction = Direction::Down;
				hamster->transform.position = glm::vec3(29.0, hamster->transform.position.x, 30.0f);
				hammer->transform.position = hamster->transform.position + hammer_offset;
			}
			else
			{
				hamster->transform.position = glm::vec3(29.0, hamster->transform.position.y, 30.0f);
				hammer->transform.position = hamster->transform.position + hammer_offset;
			}
		}
		if (hamster->transform.position.z > 30.0f)
		{
			clearAll();
			hamster->transform.position = glm::vec3(-31.0, hamster->transform.position.y, -10.0f);
			hammer->transform.position = hamster->transform.position + hammer_offset;
		}
		//object drops
		next_drop -= elapsed;
		if (next_drop <= 0.0f)
		{
			next_drop = 1.0f;
			int drop_type = mt_rand() % 3;
			glm::vec3 pos = glm::vec3((float)(mt_rand() % 54) - 27.0f, (float)(mt_rand() % 54) - 27.0f, 30.0f);
			bool can_drop = true;
			for (auto kv : objects)
			{
				if (kv.first.substr(0,3)=="Log")
				{
					if (abs(pos.x - kv.second.transform.position.x) <= 3.0f || abs(pos.y - kv.second.transform.position.y) <= 3.0f)
					{
						can_drop = false;
						break;
					}
				}
				else if (kv.first.substr(0, 3) == "Nut")
				{
					if (abs(pos.x - kv.second.transform.position.x) <= 1.0f || abs(pos.y - kv.second.transform.position.y) <= 1.0f)
					{
						can_drop = false;
						break;
					}
				}
			}
			if (can_drop)
			{
				if (drop_type != 0)
					AddObject("Nut" + std::to_string(nut_count++), TOC::NUT_MESH, pos);
				else
				{
					std::string name = "Log" + std::to_string(log_count++);
					AddObject(name, TOC::LOG_MESH, pos);
					RotateObject(name, (float)(mt_rand() % 4) * 0.5f * M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
				}
			}
		}
		if (windt != 0.0f)
		{
			windt -= elapsed;
			if (windt < 0.0f)
			{
				windt = 0.0f;
				windv = 0.0f;
			}
		}
		else
		{
			int dir = mt_rand() % 3; // ?
			windt = 10.0f;
			windv = dir - 1.0f;
		}
		auto kv = objects.begin();
		while (kv != objects.end())
		{
			if (kv->first.substr(0, 3) == "Nut" || kv->first.substr(0, 3) == "Log")
			{
				glm::vec3& pos = objects[kv->first].transform.position;
				pos.z -= 5.0f * elapsed;
				if (pos.z <= 0.5f && kv->first.substr(0, 3) == "Nut" && abs(pos.x) <= center.x + 31.0f && abs(pos.y) <= center.y + 31.0f && pos.z > 0.0f)
					pos.z = 0.5f;
				if (pos.z <= 1.0f && kv->first.substr(0, 3) == "Log" && abs(pos.x) <= center.x + 31.0f && abs(pos.y) <= center.y + 31.0f && pos.z > 0.0f)
					pos.z = 1.0f;
				pos.y += windv * elapsed;
				if (pos.z < -1.5f)
					kv = objects.erase(objects.find(kv->first));
				else
					kv++;
			}
			else
				kv++;
		}
		//camera.set(60.0f, 0.2f * M_PI, 1.0f * M_PI, center);
		return true;
	}

	void StoryScene::Render()
	{
		Graphics::Begin();


		glm::mat4 world_to_camera = camera.transform.make_world_to_local();
		glm::mat4 world_to_clip = camera.make_projection() * world_to_camera;
		glm::vec3 light_pos = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 light_in_camera = glm::mat3(world_to_camera) * light_pos;

		// Compute the MVP matrix from the light's point of view
		glm::mat4 depthProjectionMatrix = glm::ortho<float>(-30.0f, 30.0f, -30.0f, 30.0f, -30.0f, 30.0f);
		glm::mat4 depthViewMatrix = glm::lookAt(light_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::mat4 depthModelMatrix = glm::mat4(1.0);
		//glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		//glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0])

		// Shadows
		static GLint depthMatrixID = glGetUniformLocation(Graphics::shadow, "depthMVP");
		glBindFramebuffer(GL_FRAMEBUFFER, Graphics::FramebufferName);
		glViewport(0, 0, 1024, 1024);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);

		glUseProgram(Graphics::shadow);
		for (auto kv : objects)
		{
			Object& obj = kv.second;
			glm::mat4 local_to_world = obj.transform.make_local_to_world();
			glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * local_to_world;

			glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0]);
			glDrawArrays(GL_TRIANGLES, obj.mesh.vertex_start, obj.mesh.vertex_count);
		}

		static GLint basic_depth_bias_mvp = glGetUniformLocation(Graphics::basic, "depth_bias_mvp");
		static GLint shadowmap = glGetUniformLocation(Graphics::basic, "shadowmap");
		static glm::mat4 biasMatrix(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, 640, 480);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		for (auto kv : objects)
		{
			Object& obj = kv.second;
			glm::mat4 local_to_world = obj.transform.make_local_to_world();

			// compute model view + projection (object space to clip space) matrix
			glm::mat4 mvp = world_to_clip * local_to_world;

			if (obj.mesh.animated)
			{
				obj.mesh.Update(elapsed);
				glUseProgram(Graphics::animated);
				glUniformMatrix4fv(animated_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
				glUniformMatrix4x3fv(animated_bones, obj.mesh.bind_to_world.size(), GL_FALSE, glm::value_ptr(obj.mesh.bind_to_world[0]));
				glUniform3fv(animated_to_light, 1, glm::value_ptr(glm::normalize(light_in_camera)));
				glDrawArrays(GL_TRIANGLES, obj.mesh.vertex_start, obj.mesh.vertex_count);
			}
			else
			{
				// compute model view (object space to camera local space) matrix
				glm::mat4 mv = world_to_camera * local_to_world;

				// NOTE: inverse cancels out transpose unless there is scale involved
				glm::mat3 itmv = glm::inverse(glm::transpose(glm::mat3(mv)));

				glUseProgram(Graphics::basic);
				glUniformMatrix4fv(basic_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
				glUniformMatrix3fv(basic_itmv, 1, GL_FALSE, glm::value_ptr(itmv));

				glm::mat4 depthBiasMVP = biasMatrix * depthProjectionMatrix * depthViewMatrix * local_to_world;
				glUniformMatrix4fv(basic_depth_bias_mvp, 1, GL_FALSE, glm::value_ptr(depthBiasMVP));

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Graphics::depthTexture);
				glUniform1i(shadowmap, 0);

				glUniform3fv(basic_to_light, 1, glm::value_ptr(glm::normalize(light_in_camera)));
				glDrawArrays(GL_TRIANGLES, obj.mesh.vertex_start, obj.mesh.vertex_count);
			}
		}

		// Nut counter
		nutcounter.transform.scale = glm::vec3(2.5f);
		static float shake = 0.0f;
		static float sign = -2.0f;
		shake += sign * elapsed;
		if (shake >= 0.30f)
		{
			shake = 0.30f;
			sign = -2.0f;
		}
		else if (shake <= -0.30f)
		{
			shake = -0.30f;
			sign = 2.0f;
		}
		nutcounter.transform.rotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), shake, glm::vec3(1.0f, 0.0f, 0.0f));
		nutcounter.transform.rotation = glm::rotate(nutcounter.transform.rotation, (float)(0.2f * M_PI), glm::vec3(0.0f, 1.0f, 0.0f));
		for (int i = 0; i < max; i++)
		{
			if (i == score)
			{
				nutcounter.transform.scale = glm::vec3(1.25f);
				
				//nutcounter.transform.rotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), (float)(-0.3f * M_PI), glm::vec3(0.0f, 1.0f, 0.0f));
			}
			nutcounter.transform.position.y = 26.5f - 5.0f * i;
			glm::mat4 local_to_world = nutcounter.transform.make_local_to_world();

			// compute model view + projection (object space to clip space) matrix
			glm::mat4 mvp = world_to_clip * local_to_world;

			// compute model view (object space to camera local space) matrix
			glm::mat4 mv = world_to_camera * local_to_world;

			// NOTE: inverse cancels out transpose unless there is scale involved
			glm::mat3 itmv = glm::inverse(glm::transpose(glm::mat3(mv)));

			glUseProgram(Graphics::basic);
			glUniformMatrix4fv(basic_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
			glUniformMatrix3fv(basic_itmv, 1, GL_FALSE, glm::value_ptr(itmv));

			glm::mat4 depthBiasMVP = biasMatrix * depthProjectionMatrix * depthViewMatrix * local_to_world;
			glUniformMatrix4fv(basic_depth_bias_mvp, 1, GL_FALSE, glm::value_ptr(depthBiasMVP));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Graphics::depthTexture);
			glUniform1i(shadowmap, 0);

			glUniform3fv(basic_to_light, 1, glm::value_ptr(glm::normalize(light_in_camera)));
			glDrawArrays(GL_TRIANGLES, nutcounter.mesh.vertex_start, nutcounter.mesh.vertex_count);
		}

		Graphics::Present();
	}
}