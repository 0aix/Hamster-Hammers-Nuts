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
		Object& AddObject(const std::string& name,
						  unsigned int meshID, 
						  glm::vec3& position, 
						  glm::vec3& dimension,
						  glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 
						  glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f));
		void RotateObject(Object* obj, float degrees, glm::vec3 axis);
		void RotateDirection(Object* obj, Direction direction);

		virtual bool HandleInput() = 0;
		virtual bool Update() = 0;
		virtual void Render() = 0;

	protected:
		std::chrono::time_point<std::chrono::steady_clock> current_time;
		std::chrono::time_point<std::chrono::steady_clock> previous_time;
		float elapsed;

		Camera camera;

		//std::unordered_map<std::string, Object> objects = std::unordered_map<std::string, Object>();
		std::vector<Object> objects;
	};

	class StoryScene : public Scene
	{
	public:
		StoryScene();
		bool HandleInput();
		bool Update();
		void Render();

	private:
		int level = 1;
		float speed = 7.5f;
		Object hamster;
		Object target;
		Object ground;
		Object ladder;
		Object hawk;
		Direction direction;
		Object* AddNut(glm::vec3 position, glm::quat rotation);
		Object* AddLog(glm::vec3 position, glm::quat rotation);
		std::vector<Object*> logs;
		std::vector<Object*> nuts;
		float gravity = 9.0f;
		//bool on_ladder = false;
		//bool transition = false;
		int max_score = 5;
		float stun = 0.0f;
		float windv = 0.0f;
		float windt = 0.0f;
		int score = 0;
		float next_drop = 1.0f;
		float drop_interval = 1.0f;
		//bool preparing = false;

		//bool swinging = false;
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
	};
}