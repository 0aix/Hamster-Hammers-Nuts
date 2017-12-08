#pragma once

#include "Object.h"
#include <chrono>
#include <vector>
#include <unordered_map>
#include <list>

namespace Hamster
{
	enum class Direction
	{
		Left,
		Right,
		Down,
		Up,
		LeftDown,
		LeftUp,
		RightDown,
		RightUp
	};

	class Scene
	{
	public:
		Scene();
		virtual ~Scene() { }
		Object& AddObject(const std::string& name,
						  unsigned int meshID, 
						  glm::vec3 const & position, 
						  glm::vec3 const & dimension,
						  glm::quat const & rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 
						  glm::vec3 const & scale = glm::vec3(1.0f, 1.0f, 1.0f));
		void RotateObject(Object* obj, float degrees, glm::vec3 axis);
		void RotateDirection(Object* obj, Direction direction);

		virtual bool HandleInput() = 0;
		virtual bool Update() = 0;
		virtual void Render() = 0;

	protected:
		std::chrono::time_point<std::chrono::high_resolution_clock> current_time;
		std::chrono::time_point<std::chrono::high_resolution_clock> previous_time;
		float elapsed;

		Camera camera;

		//std::unordered_map<std::string, Object> objects = std::unordered_map<std::string, Object>();
		std::vector<Object> objects;
	};

	class StoryScene : public Scene
	{
	public:
		StoryScene();
		virtual ~StoryScene() { }
		bool HandleInput();
		bool Update();
		void Render();

	private:
		bool game_over;
		float windv = 2.5f;
		int level = 1;
		float speed = 7.5f;
		Object hamster;
		Object target;
		Object indicator;
		Object ground;
		Object ladder;
		Object hawk;
		Direction direction;
		Object* AddNut(glm::vec3 position, glm::quat rotation);
		Object* AddLog(glm::vec3 position, glm::quat rotation);
		std::vector<Object*> logs;
		std::vector<Object*> nuts;
		float gravity;
		int max_score = 5;
		float stun = 0.0f;
		float windxv = 0.0f;
		float windyv = 0.0f;
		float windt = 0.0f;
		int score = 0;
		float next_drop = 1.0f;
		float drop_interval = 1.0f;
		bool hawkstrike = false;
		int paused = 0;
		float whiteout = 0.0f;
		float skyoffset = 0.0f;
		float hawk_pos;

		enum class State
		{
			Idle,
			Walking,
			Swinging,
			Stunned,
			OnLadder0,
			OnLadder1,
			OnLadder2,
			Falling0,
			Falling1,
			Hawked
		} state;
	};

	class EndlessScene : public Scene
	{
	public:
		EndlessScene();
		virtual ~EndlessScene() { }
		bool HandleInput();
		bool Update();
		void Render();

	private:
		float transition_time = 20.0f;
		float windv = 2.5f;
		bool game_over;
		int level = 1;
		float speed = 7.5f;
		Object hamster;
		Object target;
		Object indicator;
		Object ground;
		Object hawk;
		Direction direction;
		Object* AddNut(glm::vec3 position, glm::quat rotation);
		Object* AddLog(glm::vec3 position, glm::quat rotation);
		std::vector<Object*> logs;
		std::vector<Object*> nuts;
		float gravity;
		float stun = 0.0f;
		float windxv = 0.0f;
		float windyv = 0.0f;
		float windt = 0.0f;
		int score = 0;
		int max_score = 0;
		float next_drop = 1.0f;
		float drop_interval = 1.0f;
		bool hawkstrike = false;
		int paused = 0;
		float whiteout = 0.0f;
		float skyoffset = 0.0f;
		float hawk_pos;

		enum class State
		{
			Idle,
			Walking,
			Swinging,
			Stunned,
			OnLadder0,
			OnLadder1,
			OnLadder2,
			Falling0,
			Falling1,
			Hawked
		} state;
	};

	class EndScene : public Scene
	{
	public:
		EndScene(int score);
		bool HandleInput();
		bool Update();
		void Render();

	private:
		Object hamster;
		Object hawk;
		int max_score;
		float time = 0.0f;
		float skyoffset = 0.0f;
	};

	class MainMenu : public Scene
	{
	public:
		MainMenu();
		bool HandleInput();
		bool Update();
		void Render();

	private:
		int selection;
		bool howtoplay;
		bool render;
	};
}
