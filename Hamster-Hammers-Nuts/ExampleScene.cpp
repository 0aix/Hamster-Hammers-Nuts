#include "Scene.h"
#include "Game.h"
#include "Graphics.h"
using namespace Hamster;

ExampleScene::ExampleScene()
{
	current_time = std::chrono::high_resolution_clock::now();
	previous_time = current_time;
	elapsed = 0.0f;
}

bool ExampleScene::HandleInput()
{
	if (Game::event.type == SDL_MOUSEMOTION)
	{

	}
	return true;
}

bool ExampleScene::Update()
{
	current_time = std::chrono::high_resolution_clock::now();
	elapsed += std::chrono::duration<float>(current_time - previous_time).count();
	previous_time = current_time;

	if (elapsed > 1.0f / 60.0f)
		elapsed = 1.0f / 60.0f;

	if (Game::KEYBD_STATE[SDL_SCANCODE_SPACE])
	{

	}
	return true;
}

void ExampleScene::Render()
{


	Graphics::Present();
}