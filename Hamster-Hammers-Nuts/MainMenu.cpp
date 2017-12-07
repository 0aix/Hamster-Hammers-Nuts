#include "Scene.h"
#include "Game.h"
#include "Graphics.h"

namespace Hamster
{
	MainMenu::MainMenu() : Scene()
	{
		selection = 0;
		howtoplay = false;
		render = true;
	}

	bool MainMenu::HandleInput()
	{
		if (Game::event.type == SDL_KEYDOWN)
		{
			if (Game::event.key.keysym.sym == SDLK_RETURN || Game::event.key.keysym.sym == SDLK_SPACE)
			{
				switch (selection)
				{
				case 0: // Story Mode
					Game::NextScene(new StoryScene());
					return false;
				case 1: // Endless
					Game::NextScene(new EndlessScene());
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
			else if (Game::event.key.keysym.sym == SDLK_BACKQUOTE)
				Audio::ToggleMute();
		}
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

	bool MainMenu::Update()
	{
		// Keep the FPS at 60 instead of max
		current_time = std::chrono::high_resolution_clock::now();
		elapsed += std::chrono::duration<float>(current_time - previous_time).count();
		previous_time = current_time;
		if (elapsed >= 1.0f / 60.0f)
		{
			elapsed -= 1.0f / 60.0f;
			render = true;
		}
		return true;
	}
	
	void MainMenu::Render()
	{
		if (!render)
			return;
		render = false;

		Graphics::BeginSprite();

		if (!howtoplay)
		{
			if (selection == 0)
				Graphics::RenderSprite(TOC::MENU_STORY_PNG, glm::vec4(-1.0f, 1.0f, 1.0f, -1.0f));
			else if (selection == 1)
				Graphics::RenderSprite(TOC::MENU_ENDLESS_PNG, glm::vec4(-1.0f, 1.0f, 1.0f, -1.0f));
			else if (selection == 2)
				Graphics::RenderSprite(TOC::MENU_HOW_PNG, glm::vec4(-1.0f, 1.0f, 1.0f, -1.0f));
			else if (selection == 3)
				Graphics::RenderSprite(TOC::MENU_QUIT_PNG, glm::vec4(-1.0f, 1.0f, 1.0f, -1.0f));
		}
		else
			Graphics::RenderSprite(TOC::INSTRUCTION_PNG, glm::vec4(-1.0f, 1.0f, 1.0f, -1.0f));

		if (Audio::muted)
			Graphics::RenderSprite(TOC::MUTE_PNG, glm::vec4(1.0f - 50.0f / 400.0f, -1.0f + 50.0f / 300.0f, 1.0f - 10.0f / 400.0f, -1.0f + 10.0f / 300.0f));
		else
			Graphics::RenderSprite(TOC::SOUND_PNG, glm::vec4(1.0f - 50.0f / 400.0f, -1.0f + 50.0f / 300.0f, 1.0f - 10.0f / 400.0f, -1.0f + 10.0f / 300.0f));

		Graphics::Present();
	}
}