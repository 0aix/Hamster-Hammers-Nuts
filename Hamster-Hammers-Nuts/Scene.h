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
		Up
	};

	class Scene
	{
	public:
		Scene();
		Object& AddObject(const std::string& name, unsigned int meshID, 
						  glm::vec3& position, 
						  glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 
						  glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f));
		//Object& AddObject(const std::string& name, unsigned int meshID, unsigned int sknID, unsigned int animID,
		//				  glm::vec3& position,
		//				  glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
		//				  glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f));
		//void RotateObject(const std::string& name, float degrees, glm::vec3 axis);

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

	class ExampleScene : public Scene
	{
	public:
		ExampleScene();
		bool HandleInput();
		bool Update();
		void Render();

	private:
		Object hamster;
		Direction direction;

		bool swinging = false;
	};

	//class StoryScene : public Scene
	//{
	//public:
	//	StoryScene();
	//	bool HandleInput();
	//	bool Update();
	//	void Render();
	//
	//private:
	//	glm::vec3 hammer_offset = glm::vec3(-1.0f, -1.0f, 0.5f);
	//	glm::vec3 center = glm::vec3(0.0f);
	//	bool on_ladder = false;
	//	bool fell = false;
	//	bool has_hawk = false;
	//	float hawk_time;
	//	float hxv;
	//	float hyv;
	//	float hzv = 0.0f;
	//	int max = 5;
	//	float stun = 0.0f;
	//	float windv = 0.0f;
	//	float windt = 0.0f;
	//	int score = 0;
	//	float xv;
	//	float yv;
	//	float zv = 0.0f;
	//	float hv;
	//	float hd;
	//	float next_drop = 5.0f;
	//	int nut_count = 0;
	//	int log_count = 0;
	//	int ground_count = 0;
	//	int wall_count = 0;
	//	Direction direction;
	//
	//	Object nutcounter;
	//};
	//
	//class StoryScene2 : public Scene
	//{
	//public:
	//	StoryScene2();
	//	bool HandleInput();
	//	bool Update();
	//	void Render();
	//
	//private:
	//	Object hamster;
	//	Object eyes;
	//	Object hammer;
	//	std::list<Object> nuts;
	//	std::list<Object> logs;
	//
	//	bool swinging = false;
	//	bool gettingready = false;
	//
	//	glm::vec3 hammer_offset = glm::vec3(0.0f, 0.0f, 0.5f);
	//	glm::vec3 center = glm::vec3(0.0f);
	//	bool on_ladder = false;
	//	bool fell = false;
	//	bool has_hawk = false;
	//	float hawk_time;
	//	float hxv;
	//	float hyv;
	//	float hzv = 0.0f;
	//	int max = 5;
	//	float stun = 0.0f;
	//	float windv = 0.0f;
	//	float windt = 0.0f;
	//	int score = 0;
	//	float xv;
	//	float yv;
	//	float zv = 0.0f;
	//	float hv;
	//	float hd;
	//	float next_drop = 5.0f;
	//	int nut_count = 0;
	//	int log_count = 0;
	//	Direction direction;
	//
	//	Object nutcounter;
	//};
}