#pragma once

#include <SDL_mixer.h>

namespace Hamster
{
	struct Sound
	{
		Mix_Music* music = NULL;
		Mix_Chunk* chunk = NULL;
	};

	namespace Audio
	{
		bool Initialize();

		void LoadMusic(Sound* sound, char* data, int size);
		void LoadChunk(Sound* sound, char* data, int size);
		
		void Play(unsigned int oggID);
		//void FadeMusic();
		
		void HaltChannels();
		void PauseChannels();
		void ResumeChannels();
	}
}