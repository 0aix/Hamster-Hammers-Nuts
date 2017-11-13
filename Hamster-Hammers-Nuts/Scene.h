#pragma once

#include "Object.h"
#include <chrono>
#include <unordered_map>
#include "Meshes.hpp"

namespace Hamster
{
	class Scene
	{
	public:
		virtual bool HandleInput() = 0;
		virtual bool Update() = 0;
		virtual void Render() = 0;
		Meshes scene_meshes;
		glm::vec3 hammer_offset = glm::vec3(-1.0f, -1.0f, 0.5f);
		glm::vec3 center;
		bool on_ladder = false;
		bool fell = false;
		bool has_hawk = false;
		float hawk_time;
		float hxv;
		float hyv;
		float hzv = 0.0f;
		int max = 25;
		float stun = 0.0f;
		float windv = 0.0f;
		float windt = 0.0f;
		int score;
		float xv;
		float yv;
		float zv =0.0f;
		float hv;
		float hd;
		float next_drop = 5.0f;
		int nut_count = 0;
		int log_count = 0;
		int ground_count = 0;
		int wall_count = 0;
		std::string direction = "Down";
	};

	class StoryScene : public Scene
	{
	public:
		StoryScene();
		bool HandleInput();
		bool Update();
		void Render();

	private:
		std::chrono::time_point<std::chrono::steady_clock> current_time;
		std::chrono::time_point<std::chrono::steady_clock> previous_time;
		float elapsed;
		// temp
		GLint program_mvp;
		GLint program_itmv;
		GLint program_to_light;

		std::unordered_map<std::string, Object> objects;
		Camera camera;
	};
}