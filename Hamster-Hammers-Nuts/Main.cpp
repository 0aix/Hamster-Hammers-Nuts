#include "Game.h"
#include "Assets.h"
#include "Graphics.h"
using namespace Hamster;

#define ASSETS_NAME "mesh.blob"
//#define ASSETS_NAME "assets.glom"
#define WINDOW_NAME "Hamster Hammers Nuts"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main()
{
	if (!Assets::LoadAssets(ASSETS_NAME))
		return -1;
	if (!Graphics::Initialize(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT))
		return -1;

	while (Game::is_running)
		Game::Update();

	Graphics::Uninitialize();

	return 0;
}