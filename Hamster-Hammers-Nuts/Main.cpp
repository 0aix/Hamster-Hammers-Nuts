#include "Game.h"
#include "Assets.h"
#include "Graphics.h"
using namespace Hamster;

#define ASSETS_NAME "hamster.glom"
#define WINDOW_NAME "Hamster Hammers Nuts"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(int argc, char** args)
{
	if (!Assets::LoadAssets(ASSETS_NAME))
		return -1;
	if (!Graphics::Initialize(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT))
		return -1;

	if (Game::Initialize())
		while (Game::is_running)
			Game::Update();

	Graphics::Uninitialize();

	return 0;
}