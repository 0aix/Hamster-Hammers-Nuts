#include "Assets.h"
#include "Scene.h"
#include "Game.h"
#include "Graphics.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <string>
#include <ctime>
#include <random>
#include <algorithm>

#define SPEED 7.5f
#define GROUND_WIDTH 30
#define GROUND_LENGTH 30

namespace Hamster
{
	Object& StoryScene::AddLog() {
		Object object;
		return object;
	}

	Object& StoryScene::AddNut() {
		Object object;
		return object;
	}

	StoryScene::StoryScene() : Scene()
	{
		hamster.transform.scale = glm::vec3(3.0f);
		hamster.anim = Animation(TOC::HAMSTER_SKN, TOC::HAMSTER_STAND_ANIM);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_BODY_MESH);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_EYES_MESH);
		hamster.animated = true;
		direction = Direction::Down;

		AddObject("Ground", TOC::GROUND_PROTO_MESH, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		//AddObject("BackWall", TOC::WALL_PROTO_MESH, glm::vec3(30.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		//AddObject("FrontWall", TOC::WALL_PROTO_MESH, glm::vec3(-30.0f, 0.0f, -20.0f), glm::vec3(0.0f, 0.0f, 0.0f));

		camera.set(100.0f, 0.2f * M_PI, 1.0f * M_PI, glm::vec3(0.0f, 0.0f, 0.0f));
	}

	bool StoryScene::HandleInput()
	{
		if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_SPACE && stun == 0.0f && !on_ladder && !swinging)
		{
			hamster.velocity = glm::vec3(0.0f);
			hamster.anim.Play(TOC::HAMSTER_SWING_ANIM, false);
			swinging = true; // should use state variable...
		}

		if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_EQUALS)
			hamster.transform.scale += glm::vec3(0.1f);
		else if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_MINUS)
			if (hamster.transform.scale.x > 0.2f)
				hamster.transform.scale -= glm::vec3(0.1f);

		return true;
	}

	bool StoryScene::Update()
	{
		static std::mt19937 mt_rand((unsigned int)time(NULL));

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
		if (stun == 0.0f && !on_ladder && !swinging) {
			if (Game::KEYBD_STATE[SDL_SCANCODE_A] && !Game::KEYBD_STATE[SDL_SCANCODE_D] && stun == 0.0f) {
				hamster.velocity.y = SPEED;
				direction = Direction::Left;
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_A] && Game::KEYBD_STATE[SDL_SCANCODE_D] && stun == 0.0f) {
				hamster.velocity.y = -SPEED;
				direction = Direction::Right;
			}
			else {
				hamster.velocity.y = 0.0f;
			}
			if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S] && stun == 0.0f) {
				hamster.velocity.x = SPEED;
				direction = Direction::Up;
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S] && stun == 0.0f) {
				hamster.velocity.x = -SPEED;
				direction = Direction::Down;
			}
			else {
				hamster.velocity.x = 0.0f;
			}
		}
		RotateDirection(&hamster, direction);
		hamster.transform.position += elapsed*hamster.velocity;

		// Update object animations
		for (auto it = objects.begin(); it != objects.end(); it++)
			if (it->animated)
				it->anim.Update(elapsed);
		hamster.anim.Update(elapsed);

		if (swinging && hamster.anim.state == AnimationState::FINISHED)
		{
			swinging = false;
		}
		if (!swinging) {
			if(hamster.velocity == glm::vec3(0.0f))
				hamster.anim.Play(TOC::HAMSTER_STAND_ANIM);
			else
				hamster.anim.Play(TOC::HAMSTER_WALK_ANIM);
		}

		next_drop -= elapsed;
		/*if (next_drop <= 0.0f) {
			next_drop = 1.0f;
			int drop_type = mt_rand() % level;
			glm::vec3 pos = 
				glm::vec3((float)(mt_rand() % (2*(GROUND_LENGTH-3))) - ((float)GROUND_LENGTH - 3.0f), 
				(float)(mt_rand() % (2 * (GROUND_WIDTH - 3))) - ((float)GROUND_WIDTH - 3.0f), 30.0f);
			bool can_drop = true;
			for (auto kv : logs) {
				if (abs(pos.x - kv.second.transform.position.x) <= 3.0f || abs(pos.y - kv.second.transform.position.y) <= 3.0f) {
					can_drop = false;
					break;
				}
			}
			for (auto kv : nuts) {
				if (abs(pos.x - kv.second.transform.position.x) <= 1.0f || abs(pos.y - kv.second.transform.position.y) <= 1.0f) {
					can_drop = false;
					break;
				}
			}
			if (can_drop) {
				if (drop_type != 0) {
					std::string name = "Nut" + std::to_string(nut_count);
					AddNut(name, "Nut", pos,
						glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
					nut_count++;
				}
				else {
					std::string name = "Log" + std::to_string(log_count);
					AddLog(name, "Log", pos,
						glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
					int rotation = mt_rand() % 4;
					RotateDirection(name, (float)rotation*0.5*M_PI, glm::vec3(0.0f, 0.0f, 1.0f));
					log_count++;
				}
			}
		}*/
		return true;
	}

	void StoryScene::Render()
	{
		// camera stuff
		glm::mat4 world_to_camera = camera.transform.make_world_to_local();
		glm::mat4 world_to_clip = camera.make_projection() * world_to_camera;
		glm::vec3 light_pos = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 to_light = glm::normalize(glm::mat3(world_to_camera) * light_pos);

		// compute model view projection from the light's point of view
		glm::mat4 light_projection = glm::ortho<float>(-32.0f, 32.0f, -32.0f, 32.0f, 0.0f, 100.0f);
		glm::mat4 light_view = glm::lookAt(50.0f * light_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 world_to_light = light_projection * light_view;

		// set world transforms (ALL CONSTANT RIGHT NOW)
		Graphics::WorldTransforms(world_to_camera, world_to_clip, world_to_light);

		// shadow map
		Graphics::BeginShadow();
		for (auto it = objects.begin(); it != objects.end(); it++)
			Graphics::RenderShadow(*it);
		Graphics::RenderShadow(hamster);

		// scene
		Graphics::BeginScene(to_light);
		for (auto it = objects.begin(); it != objects.end(); it++)
			Graphics::RenderScene(*it);
		Graphics::RenderScene(hamster);

		// ui
		//Graphics::UI(); / Graphics::Basic() / ::Draw()

		Graphics::Present();
	}
}