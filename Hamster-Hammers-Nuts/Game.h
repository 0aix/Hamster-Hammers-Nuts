#pragma once

#include "Scene.h"
#include <SDL.h>
#include "Meshes.hpp"

namespace Hamster
{
	namespace Game
	{
		extern SDL_Event event;
		extern const Uint8* KEYBD_STATE;
		extern bool is_running;

		bool Initialize();
		void Update();
		void NextScene(Scene* next);
	}
}