#include "Game.h"
using namespace Hamster;

namespace Game
{
	bool HandleInput()
	{
		return true;
	}

	bool Update()
	{
		return true;
	}

	bool IsRunning()
	{
		return HandleInput() && Update();
	}
}