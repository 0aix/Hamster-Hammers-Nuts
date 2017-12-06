#pragma once

namespace Hamster
{
	namespace Audio
	{
		bool Initialize();
		void Uninitialize();
		void PlayMusic(unsigned int oggID);
	}
}