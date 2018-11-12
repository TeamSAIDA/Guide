#pragma once

#include "Common.h"
#include "InformationManager.h"
#include "WorkerManager.h"
#include "BuildManager.h"

namespace MyBot
{
	class GameCommander 
	{
	public:

		GameCommander();
		~GameCommander() {};

		void onStart();
		void onFrame();
		void onEnd(bool isWinner);

		void onUnitShow(BWAPI::Unit unit);
		void onUnitHide(BWAPI::Unit unit);
		void onUnitCreate(BWAPI::Unit unit);
		void onUnitComplete(BWAPI::Unit unit);
		void onUnitRenegade(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);
		void onUnitMorph(BWAPI::Unit unit);
		void onUnitDiscover(BWAPI::Unit unit);
		void onUnitEvade(BWAPI::Unit unit);
	};

}