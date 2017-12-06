#include "Scene.h"
#include "Game.h"
#include "Graphics.h"

namespace Hamster
{
	MainMenu::MainMenu() : Scene()
	{
		selection = 0;
		howtoplay = false;
	}

	bool MainMenu::HandleInput()
	{
		if (Game::event.type == SDL_KEYDOWN)
		{
			if (Game::event.key.keysym.sym == SDLK_RETURN)
			{
				switch (selection)
				{
				case 0: // Story Mode
					Game::NextScene(new StoryScene());
					return false;
				case 1: // Endless
					Game::NextScene(NULL);
					return false;
				case 2: // How-to-play
					howtoplay = true;
					return true;
				case 3: // Quit
					Game::NextScene(NULL);
					return false;
				}
			}
			else if (Game::event.key.keysym.sym == SDLK_w)
			{
				if (selection > 0)
					selection--;
			}
			else if (Game::event.key.keysym.sym == SDLK_s)
			{
				if (selection < 3)
					selection++;
			}
			else if (Game::event.key.keysym.sym == SDLK_ESCAPE && howtoplay)
				howtoplay = false;
		}
		return true;
	}

	bool MainMenu::Update()
	{
		// Idk what to stick here tbh
		return true;
	}
	
	void MainMenu::Render()
	{
		Graphics::BeginSprite();

		// Draw background

		if (!howtoplay)
		{
			// Draw title
			// some array
			switch (selection)
			{
			case 0:
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			}
		}
		else
		{
			// Draw how to play title
			// Draw instructions
			// Draw
		}
	}
}