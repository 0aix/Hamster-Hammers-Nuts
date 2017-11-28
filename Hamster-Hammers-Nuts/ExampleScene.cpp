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

namespace Hamster
{
	ExampleScene::ExampleScene() : Scene()
	{
		hamster.transform.rotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), (float)(-1.5f * M_PI), glm::vec3(0.0f, 0.0f, 1.0f));
		hamster.transform.scale = glm::vec3(3.0f);
		hamster.anim = Animation(TOC::HAMSTER_SKN, TOC::HAMSTER_STAND_ANIM);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_BODY_MESH);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_EYES_MESH);
		hamster.animated = true;
		direction = Direction::Down;

		AddObject("Ground", TOC::GROUND_PROTO_MESH, glm::vec3(0.0f, 0.0f, 0.0f));
		AddObject("BackWall", TOC::WALL_PROTO_MESH, glm::vec3(30.0f, 0.0f, 20.0f));
		AddObject("FrontWall", TOC::WALL_PROTO_MESH, glm::vec3(-30.0f, 0.0f, -20.0f));

		camera.set(100.0f, 0.2f * M_PI, 1.0f * M_PI, glm::vec3(0.0f, 0.0f, 0.0f));
	}

	bool ExampleScene::HandleInput()
	{
		if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_SPACE)
		{
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

	bool ExampleScene::Update()
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

		// Update object animations
		for (auto it = objects.begin(); it != objects.end(); it++)
			if (it->animated)
				it->anim.Update(elapsed);
		hamster.anim.Update(elapsed);

		if (swinging && hamster.anim.state == AnimationState::FINISHED)
		{
			swinging = false;
			hamster.anim.Play(TOC::HAMSTER_STAND_ANIM);
		}

		return true;
	}

	void ExampleScene::Render()
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