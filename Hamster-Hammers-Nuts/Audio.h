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
		extern bool muted;

		bool Initialize();

		void LoadMusic(Sound* sound, char* data, int size);
		void LoadChunk(Sound* sound, char* data, int size);
		
		void Play(unsigned int oggID);
		void ToggleMute();
		
		void HaltChannels();
		void PauseChannels();
		void ResumeChannels();
	}
}