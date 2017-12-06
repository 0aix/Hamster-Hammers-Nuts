#include "Game.h"
#include "Assets.h"
#include "Audio.h"
#include "Graphics.h"
using namespace Hamster;

#define ASSETS_NAME "hamster.glom"
#define WINDOW_NAME "Hamster Hammers Nuts"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main(int argc, char** args)
{
	if (!Graphics::Initialize(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT))
		return 1;
	if (!Audio::Initialize())
		return 2;
	if (!Assets::LoadAssets(ASSETS_NAME))
		return 3;

	Audio::PlayMusic(TOC::HB_OGG);

	if (Game::Initialize())
		while (Game::is_running)
			Game::Update();

	Graphics::Uninitialize();
	SDL_Quit();
	return 0;
}