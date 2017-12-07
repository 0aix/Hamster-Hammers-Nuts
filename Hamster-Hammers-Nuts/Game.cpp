#include "Game.h"
#include "Audio.h"
#include "TOC.h"
#include "Scene.h"
#include <SDL.h>

namespace Hamster
{
	namespace Game
	{
		Scene* scene;
		SDL_Event event;
		const Uint8* KEYBD_STATE = SDL_GetKeyboardState(NULL);
		bool is_running = true;

		bool Initialize()
		{

			scene = new EndlessScene();
			//Audio::Play(TOC::BGM_OGG);
			//scene = new MainMenu();
			return true;
		}

		void Update()
		{
			while (SDL_PollEvent(&event) == 1)
			{
				if (event.type == SDL_QUIT)
				{
					is_running = false;
					return;
				}
				if (!scene->HandleInput())
					return;
			}

			if (!scene->Update())
				return;

			scene->Render();
		}

		void NextScene(Scene* next)
		{
			if (!next)
				is_running = false;
			else
				scene = next;
		}
	}
}