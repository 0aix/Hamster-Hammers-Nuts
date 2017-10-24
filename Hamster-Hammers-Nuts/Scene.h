#pragma once

#include <chrono>

namespace Hamster
{
	class Scene
	{
	public:
		virtual bool HandleInput() = 0;
		virtual bool Update() = 0;
		virtual void Render() = 0;
	};

	class ExampleScene : public Scene
	{
	public:
		ExampleScene();
		bool HandleInput();
		bool Update();
		void Render();

	private:
		std::chrono::time_point<std::chrono::steady_clock> current_time;
		std::chrono::time_point<std::chrono::steady_clock> previous_time;
		float elapsed;
	};
}