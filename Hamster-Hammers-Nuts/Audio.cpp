#include "Audio.h"
#include "Assets.h"
#include <SDL.h>
#include <SDL_mixer.h>

namespace Hamster
{
	namespace Audio
	{
		Mix_Music* music = NULL;
		SDL_RWops* rw = NULL;

		bool Initialize()
		{
			SDL_InitSubSystem(SDL_INIT_AUDIO);

			if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
				return false;

			return true;
		}

		void PlayMusic(unsigned int oggID)
		{
			rw = SDL_RWFromMem(&Assets::oggbuffer[Assets::oggs[TOC::HB_OGG].start], Assets::oggs[TOC::HB_OGG].count);
			if (rw)
			{
				music = Mix_LoadMUS_RW(rw, 1);
				if (music)
					Mix_PlayMusic(music, -1);
			}
		}
	}
}