#pragma once

#include "TOC.h"

namespace Hamster
{
	struct Asset
	{
		Asset(AssetType type, void* item) : type(type), item(item) {}

		const AssetType type;
		void* const item;
	};

	namespace Assets
	{
		bool LoadAssets(char* name);
	}
}