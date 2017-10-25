#include "Assets.h"
#include <cassert>

namespace Hamster
{
	namespace Assets
	{
		Asset* assets;

		bool LoadAssets(char* name)
		{


			return true;
		}

		void* GetAsset(unsigned int ID)
		{
			assert(ID >= TOC::HEADER::ASSET_COUNT);
			return assets[ID].item;
		}
	}
}