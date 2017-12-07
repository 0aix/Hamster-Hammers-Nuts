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
#include <iostream>

#define GROUND_WIDTH 15
#define GROUND_LENGTH 15
#define DROP_HEIGHT 15
#define SWING_FACTOR 1.3f
#define WALK_FACTOR 1.8f
#define HAWK_POS 700.0f

namespace Hamster
{
	Object* EndlessScene::AddNut(glm::vec3 position, glm::quat rotation)
	{
		Object* object = new Object();
		object->velocity = glm::vec3(0.0f);
		object->transform.position = position;
		object->transform.rotation = rotation;
		object->transform.scale = glm::vec3(0.5f);
		object->height = 0.5f;
		object->length = 0.5f;
		object->width = 0.5f;
		object->mesh = Mesh(TOC::NUT_MESH);
		object->animated = false;
		nuts.push_back(object);
		return nuts.back();
	}

	Object* EndlessScene::AddLog(glm::vec3 position, glm::quat rotation)
	{
		Object* object = new Object();
		object->velocity = glm::vec3(0.0f);
		object->transform.position = position;
		object->transform.rotation = rotation;
		object->base_rotation = rotation;
		object->transform.scale = glm::vec3(1.0f);
		object->height = 1.0f;
		if (rotation.z <= 0.9f && rotation.z >= 0.4f) {
			object->length = 1.0f;
			object->width = 3.0f;
		}
		else {
			object->length = 3.0f;
			object->width = 1.0f;
		}
		object->mesh = Mesh(TOC::LOG_MESH);
		object->animated = false;
		logs.push_back(object);
		return logs.back();
	}

	EndlessScene::EndlessScene() : Scene()
	{
		// Center of hamster is not actual center
		hamster.transform.scale = glm::vec3(3.0f);
		hamster.transform.position.z = 0.0f;
		hamster.height = 3.0f;
		hamster.length = 1.0f;
		hamster.width = 1.0f;
		hamster.anim = Animation(TOC::HAMSTER_SKN, TOC::HAMSTER_STAND_ANIM);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_BODY_MESH);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_EYES_MESH);
		hamster.animated = true;
		direction = Direction::Down;

		target.mesh = Mesh(TOC::CIRCLE_MESH);
		target.transform.scale = glm::vec3(0.9f, 0.9f, 0.001f);
		indicator.mesh = Mesh(TOC::SQUARE_MESH);
		indicator.transform.scale = glm::vec3(1.5f, GROUND_WIDTH, 0.001f);

		ground.mesh = Mesh(TOC::GROUND_SPRING_MESH);
		ground.transform.scale = glm::vec3(0.5f);

		hawk.anim = Animation(TOC::ARMATURE_SKN, TOC::ARMATURE_FLAP_ANIM, true, 3.0f);
		hawk.anim.mesh.emplace_back(TOC::ARMATURE_BODY_MESH);
		hawk.animated = true;
		hawk.transform.scale = glm::vec3(3.0f);
		hawk.transform.position = glm::vec3(7.0f, HAWK_POS * 2.0f, -7.5f);
		hawk.height = 3.0f;
		hawk.length = 2.0f;
		hawk.width = 4.0f;

		camera.set(100.0f, 0.2f * M_PI, 1.0f * M_PI, glm::vec3(0.0f, 0.0f, 0.0f));

		gravity = 18.0f;
		level = 1;
		next_drop = 3.0f;
		whiteout = 1.0f;
		hawk_pos = HAWK_POS;
	}

	bool EndlessScene::HandleInput()
	{
		if (paused == 0)
		{
			if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_SPACE && state <= State::Walking)
			{
				hamster.velocity = glm::vec3(0.0f);
				hamster.anim.Play(TOC::HAMSTER_SWING_ANIM, false, true, SWING_FACTOR);
				state = State::Swinging;
			}
			if (Game::event.window.event == SDL_WINDOWEVENT_FOCUS_LOST ||
				(Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_ESCAPE))
			{
				Audio::PauseChannels();
				paused = 1;
			}
		}
		else
		{
			if (Game::event.type == SDL_KEYDOWN)
			{
				if (Game::event.key.keysym.sym == SDLK_ESCAPE)
				{
					Audio::ResumeChannels();
					paused = 0;
				}
				else if (Game::event.key.keysym.sym == SDLK_w)
				{
					if (paused == 2)
						paused = 1;
				}
				else if (Game::event.key.keysym.sym == SDLK_s)
				{
					if (paused == 1)
						paused = 2;
				}
				else if (Game::event.key.keysym.sym == SDLK_RETURN || Game::event.key.keysym.sym == SDLK_SPACE)
				{
					if (paused == 1)
					{
						Audio::ResumeChannels();
						paused = 0;
					}
					else
					{
						Audio::HaltChannels();
						Game::NextScene(new MainMenu());
						return false;
					}
				}
			}
		}
		if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_BACKQUOTE)
			Audio::ToggleMute();
		if (Game::event.type == SDL_MOUSEBUTTONDOWN)
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

	bool EndlessScene::Update()
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

		if (paused > 0)
			return true;

		skyoffset -= elapsed * 0.05f;
		if (skyoffset <= 0.0f)
			skyoffset += 1.0f;

		if (whiteout >= 0.0f)
			whiteout -= elapsed * 0.5f;

		transition_time -= elapsed;
		if (transition_time < 0.0f && !hawkstrike) {
			level = (mt_rand() % 4)*3 + 1;
			transition_time = 20.0f;
			speed = 7.5f;
			windv = mt_rand() % 10;
			hawk.transform.position.y = (((int)mt_rand() % 2) * 2 - 1) * hawk_pos * 2.0f;
			hawk.velocity.y = 0.0f;
			hawkstrike = false;
			hawk_pos = HAWK_POS - (level - 7) * 100.0f;
			if (level < 4) {
				ground.mesh = Mesh(TOC::GROUND_SPRING_MESH);
			}
			else if (level < 7) {
				ground.mesh = Mesh(TOC::GROUND_SUMMER_MESH);
			}
			else if (level < 10) {
				ground.mesh = Mesh(TOC::GROUND_FALL_MESH);
			}
			else {
				ground.mesh = Mesh(TOC::GROUND_WINTER_MESH);
				speed = 5.0f;
			}
		}

		if (state <= State::Walking)
		{
			state = State::Walking;
			if (Game::KEYBD_STATE[SDL_SCANCODE_A] && !Game::KEYBD_STATE[SDL_SCANCODE_D])
			{
				if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S]) {
					hamster.velocity.x = speed * 0.707107f;
					hamster.velocity.y = speed * 0.707107f;
				}
				else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S]) {
					hamster.velocity.x = -speed * 0.707107f;
					hamster.velocity.y = speed * 0.707107f;
				}
				else {
					hamster.velocity.x = 0.0f;
					hamster.velocity.y = speed;
				}
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_A] && Game::KEYBD_STATE[SDL_SCANCODE_D])
			{
				if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S]) {
					hamster.velocity.x = speed * 0.707107f;
					hamster.velocity.y = -speed * 0.707107f;
				}
				else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S]) {
					hamster.velocity.x = -speed * 0.707107f;
					hamster.velocity.y = -speed * 0.707107f;
				}
				else {
					hamster.velocity.x = 0.0f;
					hamster.velocity.y = -speed;
				}
			}
			else if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S]) {
				hamster.velocity.x = speed;
				hamster.velocity.y = 0.0f;
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S]) {
				hamster.velocity.x = -speed;
				hamster.velocity.y = 0.0f;
			}
			else {
				state = State::Idle;
				hamster.velocity.x = 0.0f;
				hamster.velocity.y = 0.0f;
			}
		}

		if (state <= State::Walking)
		{
			if (hamster.velocity.x > 0.0f) {
				if (hamster.velocity.y < 0.0f) {
					direction = Direction::RightUp;
				}
				else if (hamster.velocity.y > 0.0f) {
					direction = Direction::LeftUp;
				}
				else {
					direction = Direction::Up;
				}
			}
			else if (hamster.velocity.x < 0.0f) {
				if (hamster.velocity.y < 0.0f) {
					direction = Direction::RightDown;
				}
				else if (hamster.velocity.y > 0.0f) {
					direction = Direction::LeftDown;
				}
				else {
					direction = Direction::Down;
				}
			}
			else {
				if (hamster.velocity.y < 0.0f) {
					direction = Direction::Right;
				}
				else if (hamster.velocity.y > 0.0f) {
					direction = Direction::Left;
				}
			}
		}


		float x1 = hamster.transform.position.x + elapsed * hamster.velocity.x;
		float y1 = hamster.transform.position.y + elapsed * hamster.velocity.y;
		float z1 = hamster.transform.position.z;

		if (abs(hamster.transform.position.x) - 2.0f * hamster.length > GROUND_LENGTH ||
			abs(hamster.transform.position.y) - 2.0f * hamster.width > GROUND_WIDTH)
		{
			if (state == State::Hawked)
				hamster.transform.position.y = hamster.transform.position.y < 0.0f ? -GROUND_WIDTH - 5.0f * hamster.width : GROUND_WIDTH + 5.0f * hamster.width;
			state = State::Falling0;
			score = std::max(0, score - 5);
			hamster.velocity.x = 0.0f;
			hamster.velocity.y = 0.0f;
			hamster.velocity.z -= elapsed * gravity;
		}

		if (state == State::Stunned)
		{
			stun -= elapsed;
			RotateObject(&hamster, 540.0f * elapsed, glm::vec3(0.0f, 0.0f, 1.0f));
			if (stun < 0.0f)
				state = State::Idle;
		}

		if (state != State::OnLadder0 && state != State::OnLadder1 && state != State::OnLadder2) {
			for (auto log : logs)
			{
				if (abs(log->transform.position.z + elapsed * log->velocity.z - z1) <= hamster.height + log->height &&
					log->transform.position.z > log->height &&
					log->velocity.z < 0.0f)
				{
					if (abs(log->transform.position.x - hamster.transform.position.x) < hamster.length + log->length &&
						abs(log->transform.position.y - hamster.transform.position.y) < hamster.width + log->width)
					{
						if (score > 0 && stun < 0.9f)
							score--;
						state = State::Stunned;
						stun = 1.0f;
						hamster.velocity.x = 10.0f;
						hamster.velocity.y = 10.0f;
						if (log->transform.position.y - hamster.transform.position.y > 0.0f)
							hamster.velocity.y = -10.0;
						if (log->transform.position.x - hamster.transform.position.x > 0.0f)
							hamster.velocity.x = -10.0f;
						break;
					}
				}
			}
			for (auto nut : nuts)
			{
				if (abs(abs(nut->transform.position.z + elapsed * nut->velocity.z - z1) - hamster.height - nut->height) < 0.25f &&
					nut->transform.position.z > nut->height && nut->velocity.z < 0.0f)
				{
					if (abs(nut->transform.position.x - hamster.transform.position.x) < hamster.length + nut->length &&
						abs(nut->transform.position.y - hamster.transform.position.y) < hamster.width + nut->width)
					{
						if (score > 0 && stun < 0.9f)
							score--;
						state = State::Stunned;
						stun = 1.0f;
						hamster.velocity.x = 10.0f;
						hamster.velocity.y = 10.0f;
						if (nut->transform.position.y - hamster.transform.position.y > 0.0f)
							hamster.velocity.y = -10.0;
						if (nut->transform.position.x - hamster.transform.position.x > 0.0f)
							hamster.velocity.x = -10.0f;
						break;
					}
				}
			}
			for (auto log : logs) {
				if (abs(log->transform.position.z - z1) <= hamster.height + log->height && state != State::Stunned) {
					if (abs(log->transform.position.x - x1) < hamster.length + log->length && abs(log->transform.position.y - y1) < hamster.width + log->width) {
						hamster.velocity.x = log->velocity.x;
						hamster.velocity.y = log->velocity.y;
					}
				}
				if (abs(log->transform.position.z - z1) <= hamster.height + log->height && state == State::Stunned) {
					if (abs(log->transform.position.x - x1) < hamster.length + log->length && abs(log->transform.position.y - y1) < hamster.width + log->width) {
						hamster.velocity.x = 10.0f;
						hamster.velocity.y = 10.0f;
						if (log->transform.position.y - hamster.transform.position.y > 0.0f)
							hamster.velocity.y = -10.0f;
						if (log->transform.position.x - hamster.transform.position.x > 0.0f)
							hamster.velocity.x = -10.0f;
					}
				}
			}
		}

		// Update object animations
		hamster.anim.Update(elapsed);
		hawk.anim.Update(elapsed);

		if (state == State::Idle)
		{
			hamster.anim.Play(TOC::HAMSTER_STAND_ANIM);
		}
		else if (state == State::Walking)
		{
			hamster.anim.Play(TOC::HAMSTER_WALK_ANIM, true, true, WALK_FACTOR);
		}

		next_drop -= elapsed;
		if (next_drop <= 0.0f) {
			//next_drop = drop_interval;
			//int drop_type = level < 4 ? 0 : mt_rand() % (level / 4 + 1);
			int drop_type = level < 4 ? 0 : mt_rand() % 2;
			glm::vec3 pos =
				glm::vec3((float)(mt_rand() % (2 * (GROUND_LENGTH - 3))) - ((float)GROUND_LENGTH - 3.0f),
				(float)(mt_rand() % (2 * (GROUND_WIDTH - 3))) - ((float)GROUND_WIDTH - 3.0f), 50.0f);
			bool can_drop = true;
			for (auto log : logs) {
				if (abs(pos.x - log->transform.position.x) <= 4.0f || abs(pos.y - log->transform.position.y) <= 4.0f) {
					can_drop = false;
					break;
				}
			}
			for (auto nut : nuts) {
				if (abs(pos.x - nut->transform.position.x) <= 1.0f || abs(pos.y - nut->transform.position.y) <= 1.0f) {
					can_drop = false;
					break;
				}
			}
			if (can_drop) {
				if (drop_type == 0) {
					AddNut(pos,
						normalize(glm::quat(mt_rand() % 10, mt_rand() % 10, mt_rand() % 10, mt_rand() % 10)));
				}
				else {
					int rotation = mt_rand() % 4;
					glm::quat quart = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
					quart = glm::rotate(quart, (float)M_PI*(float)rotation / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
					AddLog(pos, quart);

				}
				next_drop = drop_interval;
			}
		}

		for (auto it = logs.begin(); it != logs.end();) {
			auto log = *it;
			if (log->transform.position.z > log->height) {
				log->velocity.z -= elapsed*gravity;
			}
			if (abs(log->transform.position.x) - log->length > GROUND_LENGTH || abs(log->transform.position.y) - log->width > GROUND_WIDTH) {
				log->velocity.z -= elapsed*gravity;
			}
			if (abs(log->transform.position.x) - log->length < GROUND_LENGTH && abs(log->transform.position.y) - log->width < GROUND_WIDTH) {
				if (log->transform.position.z <= log->height && log->velocity.z < 0.0f) {
					log->velocity.z = -log->velocity.z / 4.0f;
					if (abs(log->velocity.z) < 1.0f) {
						log->transform.position.z = log->height;
						log->velocity.z = 0.0f;
					}
				}
			}
			if (log->transform.position.z < -10.0f)
				it = logs.erase(it);
			else
				it++;
		}
		for (auto it = nuts.begin(); it != nuts.end();) {
			auto nut = *it;
			if (nut->transform.position.z > nut->height) {
				nut->velocity.z -= elapsed*gravity;
			}
			if (abs(nut->transform.position.x) - nut->length > GROUND_LENGTH || abs(nut->transform.position.y) - nut->width > GROUND_WIDTH) {
				nut->velocity.z -= elapsed*gravity;
			}
			if (abs(nut->transform.position.x) - nut->length < GROUND_LENGTH && abs(nut->transform.position.y) - nut->width < GROUND_WIDTH) {
				if (nut->transform.position.z <= nut->height && nut->velocity.z < 0.0f) {
					nut->velocity.z = -nut->velocity.z / 4.0f;
					if (abs(nut->velocity.z) < 1.0f) {
						nut->transform.position.z = nut->height;
						nut->velocity.z = 0.0f;
					}
				}
			}
			if (nut->transform.position.z < -10.0f)
				it = nuts.erase(it);
			else
				it++;
		}
		//wind
		if (level >= 4) {
			if (windt != 0.0f) {
				windt -= elapsed;
				if (windt < 0.0f) {
					windt = 0.0f;
					windyv = 0.0f;
					windxv = 0.0f;
				}
			}
			else {
				int dir = mt_rand() % 6;
				windt = 10.0f;
				if (dir < 3) {
					windyv = windv*(dir - 1);
					windxv = 0.0f;
				}
				else {
					windxv = windv*(dir - 4);
					windyv = 0.0f;
				}
			}
		}
		//Log rolling not fully working
		for (auto log : logs) {
			log->velocity.y = windyv;
			log->velocity.x = windxv;
			if (log->length != 1.0f && log->base_rotation.z == 1.0f) {
				RotateObject(log, (float)M_PI / 4.0f*elapsed*windyv, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			if (log->length != 1.0f && log->base_rotation.w == 1.0f) {
				RotateObject(log, (float)-M_PI / 4.0f*elapsed*windyv, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			if (log->length == 1.0f && log->base_rotation.w > 0.0f) {
				RotateObject(log, (float)M_PI / 4.0f*elapsed*windxv, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			if (log->length == 1.0f && log->base_rotation.w < 0.0f) {
				RotateObject(log, (float)-M_PI / 4.0f*elapsed*windxv, glm::vec3(1.0f, 0.0f, 0.0f));
			}
		}
		for (auto nut : nuts) {
			nut->velocity.y = windyv;
			nut->velocity.x = windxv;
		}
		// HAWK STRIKE
		if (level >= 7) {
			if (hawk.transform.position.y >= hawk_pos) {
				hawk.velocity.y = -200.0f;
				hawk.transform.position.x = (int)((unsigned int)mt_rand() % (2 * (GROUND_LENGTH - 3))) - (GROUND_LENGTH - 3);
			}
			else if (hawk.transform.position.y <= -hawk_pos) {
				hawk.velocity.y = 200.0f;
				hawk.transform.position.x = (int)((unsigned int)mt_rand() % (2 * (GROUND_LENGTH - 3))) - (GROUND_LENGTH - 3);
			}
			else if (hawk.velocity.y > 0.0f && hawk.transform.position.y >= 50.0f)
				hawkstrike = false;
			else if (hawk.velocity.y < 0.0f && hawk.transform.position.y <= -50.0f)
				hawkstrike = false;
			else if (!hawkstrike && (abs(hawk.transform.position.y) < hawk_pos * 0.3f || abs(hawk.transform.position.y) < 250.0f))
			{
				Audio::Play(TOC::HAWK_OGG);
				hawkstrike = true;
				indicator.transform.position = glm::vec3(hawk.transform.position.x, 0.0f, 0.01f);
			}
			if (abs(hawk.transform.position.x - hamster.transform.position.x) < 2.0f && abs(hawk.transform.position.y - hamster.transform.position.y) < 2.0f &&
				abs(hamster.transform.position.x) - 2.0f * hamster.length < GROUND_LENGTH &&
				abs(hamster.transform.position.y) - 2.0f * hamster.width < GROUND_WIDTH) {
				state = State::Hawked;
			}

			if (state == State::Hawked) {
				hamster.velocity = hawk.velocity;
			}
		}

		hawk.transform.position += elapsed*hawk.velocity;
		hamster.transform.position += elapsed*hamster.velocity;
		
		if (hamster.transform.position.z < -30.0f && state == State::Falling0 && hamster.velocity.z < 0.0f) {
			logs.clear();
			nuts.clear();
			if (score == 0)
			{
				Game::NextScene(new EndScene(max_score));
				return false;
			}
			//END GAME HERE SOMEHOW
		}
		
		for (auto nut : nuts) {
			nut->transform.position += nut->velocity*elapsed;
		}
		for (auto log : logs) {
			log->transform.position += log->velocity*elapsed;
		}

		if (state == State::Swinging && hamster.anim.frame_number == 12) {
			bool thunk = false;
			for (auto it = nuts.begin(); it != nuts.end(); it++) {
				auto nut = *it;
				//if (nut->velocity.z == 0.0f) {
				if (nut->transform.position.z <= 1.0f)
				{
					float dx = hamster.transform.position.x - nut->transform.position.x;
					float dy = hamster.transform.position.y - nut->transform.position.y;
					if (direction == Direction::Up && abs(dx + 2.3f) <= 0.9f && abs(dy) <= 0.9f)
					{
						it = nuts.erase(it);
						score++;
						max_score++;
						thunk = true;
						break;
					}
					if (direction == Direction::Down && abs(dx - 2.3f) <= 0.9f && abs(dy) <= 0.9f) {
						it = nuts.erase(it);
						score++;
						max_score++;
						thunk = true;
						break;
					}
					if (direction == Direction::Left && abs(dx) <= 0.9f && abs(dy + 2.3f) <= 0.9f) {
						it = nuts.erase(it);
						score++;
						max_score++;
						thunk = true;
						break;
					}
					if (direction == Direction::Right && abs(dx) <= 0.9f && abs(dy - 2.3f) <= 0.9f)
					{
						it = nuts.erase(it);
						score++;
						max_score++;
						thunk = true;
						break;
					}
					if (direction == Direction::LeftUp && abs(dx + 1.626346f) <= 0.9f && abs(dy + 1.626346f) <= 0.9f) {
						it = nuts.erase(it);
						score++;
						max_score++;
						thunk = true;
						break;
					}
					if (direction == Direction::LeftDown && abs(dx - 1.626346f) <= 0.9f && abs(dy + 1.626346f) <= 0.9f) {
						it = nuts.erase(it);
						score++;
						max_score++;
						thunk = true;
						break;
					}
					if (direction == Direction::RightUp && abs(dx + 1.626346f) <= 0.9f && abs(dy - 1.626346f) <= 0.9f) {
						it = nuts.erase(it);
							score++;
						thunk = true;
						break;
					}
					if (direction == Direction::RightDown && abs(dx - 1.626346f) <= 0.9f && abs(dy - 1.626346f) <= 0.9f) {
						it = nuts.erase(it);
						score++;
						max_score++;
						thunk = true;
						break;
					}
				}
			}
			if (thunk)
				Audio::Play(TOC::CRACK_OGG);
		}

		if (state == State::Swinging && hamster.anim.state == AnimationState::FINISHED)
		{
			state = State::Idle;
		}
		if (state != State::Stunned)
			RotateDirection(&hamster, direction);
		if (hawk.velocity.y < 0.0f) {
			RotateDirection(&hawk, Direction::Right);
		}
		else {
			RotateDirection(&hawk, Direction::Left);
		}
		if (state == State::Falling0 || state == State::Falling1)
			hamster.velocity.z -= elapsed * gravity;
		return true;
	}

	void EndlessScene::Render()
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
		for (auto it = nuts.begin(); it != nuts.end(); it++)
			Graphics::RenderShadow(**it);
		for (auto it = logs.begin(); it != logs.end(); it++)
			Graphics::RenderShadow(**it);
		Graphics::RenderShadow(hamster);
		Graphics::RenderShadow(hawk);

		// scene
		Graphics::BeginScene(to_light);
		for (auto it = nuts.begin(); it != nuts.end(); it++)
			Graphics::RenderScene(**it);
		for (auto it = logs.begin(); it != logs.end(); it++)
			Graphics::RenderScene(**it);
		Graphics::RenderScene(hamster);
		Graphics::RenderScene(ground);
		Graphics::RenderScene(hawk);

		if (hawkstrike)
			Graphics::RenderScene(indicator, 0.75f);

		if (state <= State::Swinging)
		{
			switch (direction)
			{
			case Direction::Left:
				target.transform.position = hamster.transform.position + glm::vec3(0.0f, 2.3f, 0.0f);
				break;
			case Direction::Right:
				target.transform.position = hamster.transform.position + glm::vec3(0.0f, -2.3f, 0.0f);
				break;
			case Direction::Down:
				target.transform.position = hamster.transform.position + glm::vec3(-2.3f, 0.0f, 0.0f);
				break;
			case Direction::Up:
				target.transform.position = hamster.transform.position + glm::vec3(2.3f, 0.0f, 0.0f);
				break;
			case Direction::LeftDown:
				target.transform.position = hamster.transform.position + glm::vec3(-1.626346f, 1.626346f, 0.0f);
				break;
			case Direction::LeftUp:
				target.transform.position = hamster.transform.position + glm::vec3(1.626346f, 1.626346f, 0.0f);
				break;
			case Direction::RightDown:
				target.transform.position = hamster.transform.position + glm::vec3(-1.626346f, -1.626346f, 0.0f);
				break;
			case Direction::RightUp:
				target.transform.position = hamster.transform.position + glm::vec3(1.626346f, -1.626346f, 0.0f);
				break;
			}
			Graphics::RenderScene(target, 0.5f);
		}

		// background
		Graphics::BeginSprite();
		Graphics::RenderSprite(TOC::SKY_PNG, glm::vec4(-1.0f, 1.0f, -1.0f + skyoffset * 2.0f, -1.0f),
			glm::vec4(1.0f - skyoffset, 1.0f, 1.0f, 0.0f));
		Graphics::RenderSprite(TOC::SKY_PNG, glm::vec4(-1.0f + skyoffset * 2.0f, 1.0f, 1.0f, -1.0f),
			glm::vec4(0.0f, 1.0f, 1.0f - skyoffset, 0.0f));

		// actual scene
		Graphics::CompositeScene();

		// ui
		Graphics::BeginSprite();
		const float loc_x[11] = { .027f, .116f, .207f, .296f, .386f, .481f, .566f, .662f, .747f, .838f, .92f };
		const float loc_y[12] = { .85f, .85f, .85f, .675f, .675f, .675f, .5f, .5f, .5f, .325f, .325f, .325f };
		const float ax = -1.0f + 2.0f * 10.0f / 800.0f;
		const float ay = 1.0f - 2.0f * 15.0f / 600.0f;
		const float aw = 2.0f * 50.0f / 800.0f;
		const float ah = 2.0f * 50.0f / 600.0f;
		const float bx = ax + aw;
		const float by = 1.0f - 2.0f * 16.0f / 600.0f;
		const float w = 2.0f * 36.0f / 800.0f;
		const float h = 2.0f * 48.0f / 600.0f;

		Graphics::RenderSprite(TOC::NUT_LINE_PNG, glm::vec4(ax, ay, ax + aw, ay - ah));
		int i = score;
		int j = level - 1;
		int n = 0;
		// draw score
		while (i >= 10)
		{
			Graphics::RenderSprite(TOC::NUMBER_PNG, glm::vec4(bx + n * w, by, bx + (n + 1) * w, by - h),
													glm::vec4(loc_x[i / 10], loc_y[j], loc_x[i / 10] + 0.06f, loc_y[j] - 0.1f));
			n++;
			i %= 10;
		}
		Graphics::RenderSprite(TOC::NUMBER_PNG, glm::vec4(bx + n * w, by, bx + (n + 1) * w, by - h),
												glm::vec4(loc_x[i], loc_y[j], loc_x[i] + 0.06f, loc_y[j] - 0.1f));

		if (whiteout >= 0.0f)
			Graphics::RenderSprite(TOC::DIFFUSE_PNG, glm::vec4(-1.0f, 1.0f, 1.0f, -1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, whiteout));

		if (paused == 1)
			Graphics::RenderSprite(TOC::PAUSE_CONTINUE_PNG, glm::vec4(-1.0f, 1.0f, 1.0f, -1.0f));
		else if (paused == 2)
			Graphics::RenderSprite(TOC::PAUSE_TO_MENU_PNG, glm::vec4(-1.0f, 1.0f, 1.0f, -1.0f));

		if (Audio::muted)
			Graphics::RenderSprite(TOC::MUTE_PNG, glm::vec4(1.0f - 50.0f / 400.0f, -1.0f + 50.0f / 300.0f, 1.0f - 10.0f / 400.0f, -1.0f + 10.0f / 300.0f));
		else
			Graphics::RenderSprite(TOC::SOUND_PNG, glm::vec4(1.0f - 50.0f / 400.0f, -1.0f + 50.0f / 300.0f, 1.0f - 10.0f / 400.0f, -1.0f + 10.0f / 300.0f));

		Graphics::Present();
	}
}