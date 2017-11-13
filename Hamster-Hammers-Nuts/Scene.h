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
		int max;
		int score;
		float xv;
		float yv;
		float hv;
		float hd;
		float next_drop = 5.0f;
		int nut_count = 0;
		int log_count = 0;
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