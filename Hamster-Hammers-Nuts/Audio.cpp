#include "Audio.h"
#include "Assets.h"
#include <SDL.h>

namespace Hamster
{
	namespace Audio
	{
		bool muted = false;

		bool Initialize()
		{
			SDL_InitSubSystem(SDL_INIT_AUDIO);

			if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
				return false;

			return true;
		}

		void LoadMusic(Sound* sound, char* data, int size)
		{
			SDL_RWops* rw = SDL_RWFromMem(data, size);
			if (rw != NULL)
				sound->music = Mix_LoadMUS_RW(rw, 1);
		}

		void LoadChunk(Sound* sound, char* data, int size)
		{
			SDL_RWops* rw = SDL_RWFromMem(data, size);
			if (rw != NULL)
				sound->chunk = Mix_LoadWAV_RW(rw, 1);
		}

		void Play(unsigned int oggID)
		{
			if (Assets::sounds[oggID].music)
				Mix_PlayMusic(Assets::sounds[oggID].music, -1);
			else if (Assets::sounds[oggID].chunk)
				Mix_PlayChannel(-1, Assets::sounds[oggID].chunk, 0);
		}

		void HaltMusic()
		{
			Mix_HaltMusic();
		}

		void ToggleMute()
		{
			static int mvolume = 0;
			static int volume = 0;
			
			if ((muted = !muted)) //this is probably too clever
			{
				mvolume = Mix_VolumeMusic(0);
				volume = Mix_Volume(-1, 0);
			}
			else
			{
				mvolume = Mix_VolumeMusic(mvolume);
				Mix_Volume(-1, volume);
			}
		}

		void HaltChannels()
		{
			Mix_HaltChannel(-1);
		}

		void PauseChannels()
		{
			Mix_Pause(-1);
		}

		void ResumeChannels()
		{
			Mix_Resume(-1);
		}
	}
}
