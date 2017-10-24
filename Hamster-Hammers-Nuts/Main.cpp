#include "Game.h"
#include "Graphics.h"
#include <iostream>
using namespace Hamster;

#define NAME "Hamster Hammers Nuts"
#define WIDTH 800
#define HEIGHT 600

int main()
{
	if (!Graphics::Initialize(NAME, WIDTH, HEIGHT))
		return -1;

	while (Game::IsRunning());

	Graphics::Uninitialize();

	return 0;
}