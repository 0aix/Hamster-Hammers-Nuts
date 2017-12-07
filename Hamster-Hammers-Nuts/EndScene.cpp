#include "Scene.h"
#include "Game.h"
#include "Graphics.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Hamster
{
	EndScene::EndScene(int score) : Scene()
	{
		hamster.transform.scale = glm::vec3(3.0f);
		hamster.transform.position.z = 30.0f;
		hamster.height = 3.0f;
		hamster.length = 1.0f;
		hamster.width = 1.0f;
		hamster.anim = Animation(TOC::HAMSTER_SKN, TOC::HAMSTER_STAND_ANIM, false);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_BODY_MESH);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_EYES_MESH);
		hamster.animated = true;
		RotateDirection(&hamster, Direction::Down);

		hawk.anim = Animation(TOC::ARMATURE_SKN, TOC::ARMATURE_FLAP_ANIM, true, 3.0f);
		hawk.anim.mesh.emplace_back(TOC::ARMATURE_BODY_MESH);
		hawk.animated = true;
		hawk.transform.scale = glm::vec3(3.0f);
		hawk.transform.position = glm::vec3(30.0f, 30.0f, -7.5f);
		hawk.height = 3.0f;
		hawk.length = 2.0f;
		hawk.width = 4.0f;

		camera.set(100.0f, 0.2f * M_PI, 1.0f * M_PI, glm::vec3(0.0f, 0.0f, 0.0f));

		max_score = score;

		Audio::HaltMusic();
		Audio::Play(TOC::HAWK_OGG);
	}

	bool EndScene::HandleInput()
	{
		if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_BACKQUOTE)
				Audio::ToggleMute();
		else if (Game::event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (Game::event.button.button == SDL_BUTTON_LEFT)
			{
				float dx = Game::event.button.x - 770.0f;
				float dy = Game::event.button.y - 570.0f;
				if (dx * dx + dy * dy <= 400.0f)
					Audio::ToggleMute();
			}
		}
		return true;
	}

	bool EndScene::Update()
	{
		current_time = std::chrono::high_resolution_clock::now();
		elapsed = std::chrono::duration<float>(current_time - previous_time).count();
		previous_time = current_time;
		if (elapsed > 1.0f / 60.0f)
			elapsed = 1.0f / 60.0f;

		skyoffset -= elapsed * 0.05f;
		if (skyoffset <= 0.0f)
			skyoffset += 1.0f;

		hamster.anim.Update(elapsed);

		if (time < 2.0f)
		{
			hamster.transform.position.z -= elapsed * 500.0f;
			if (hamster.transform.position.z <= 0.0f)
				hamster.transform.position.z = -50.0f;
			hawk.transform.position.x -= elapsed * 300.0f;
			hawk.transform.position.y -= elapsed * 300.0f;
		}
		else if (time > 5.0f)
		{

		}
		RotateDirection(&hawk, Direction::Right);

		time += elapsed;
		return true;
	}

	void EndScene::Render()
	{
		// camera stuff
		glm::mat4 world_to_camera = camera.transform.make_world_to_local();
		glm::mat4 world_to_clip = camera.make_projection() * world_to_camera;
		glm::vec3 light_pos = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 to_light = glm::normalize(glm::mat3(world_to_camera) * light_pos);

		// compute model view projection from the light's point of view
		glm::mat4 light_projection = glm::ortho<float>(-17.0f, 17.0f, -17.0f, 17.0f, 0.0f, 100.0f);
		glm::mat4 light_view = glm::lookAt(50.0f * light_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 world_to_light = light_projection * light_view;

		// set world transforms (ALL CONSTANT RIGHT NOW)
		Graphics::WorldTransforms(world_to_camera, world_to_clip, world_to_light);

		// shadow map
		Graphics::BeginShadow();
		Graphics::RenderShadow(hamster);
		Graphics::RenderShadow(hawk);

		// scene
		Graphics::BeginScene(to_light);
		Graphics::RenderScene(hamster);
		Graphics::RenderScene(hawk);

		// background
		Graphics::BeginSprite();
		Graphics::RenderSprite(TOC::SKY_PNG, glm::vec4(-1.0f, 1.0f, -1.0f + skyoffset * 2.0f, -1.0f),
			glm::vec4(1.0f - skyoffset, 1.0f, 1.0f, 0.0f));
		Graphics::RenderSprite(TOC::SKY_PNG, glm::vec4(-1.0f + skyoffset * 2.0f, 1.0f, 1.0f, -1.0f),
			glm::vec4(0.0f, 1.0f, 1.0f - skyoffset, 0.0f));

		// actual scene
		Graphics::CompositeScene();

		Graphics::BeginSprite();

		if (Audio::muted)
			Graphics::RenderSprite(TOC::MUTE_PNG, glm::vec4(1.0f - 50.0f / 400.0f, -1.0f + 50.0f / 300.0f, 1.0f - 10.0f / 400.0f, -1.0f + 10.0f / 300.0f));
		else
			Graphics::RenderSprite(TOC::SOUND_PNG, glm::vec4(1.0f - 50.0f / 400.0f, -1.0f + 50.0f / 300.0f, 1.0f - 10.0f / 400.0f, -1.0f + 10.0f / 300.0f));

		Graphics::Present();
	}
}