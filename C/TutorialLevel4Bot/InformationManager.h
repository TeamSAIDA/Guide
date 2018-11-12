#pragma once

#include "Common.h"

namespace MyBot
{
	class InformationManager 
	{
		InformationManager();

	public:
		static InformationManager & Instance();
	
		BWAPI::Player			selfPlayer;
		BWAPI::Race				selfRace;

		BWAPI::Player			enemyPlayer;
		BWAPI::Race				enemyRace;

		std::map<BWAPI::Player, BWTA::BaseLocation * >      _mainBaseLocations;

		void                    update();
	};
}
