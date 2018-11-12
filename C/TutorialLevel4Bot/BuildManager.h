#pragma once

#include "Common.h"
#include "InformationManager.h"

namespace MyBot
{
	class BuildManager
	{
		BuildManager();

		void				buildWorkerUnits();
		void				buildCombatUnits();
		void				constructBuildings();

		void				buildWorkerUnit();
		void				trainUnit(BWAPI::UnitType targetUnitType);
		void				constructBuilding(BWAPI::UnitType targetUnitType);

	public:

		static BuildManager &	Instance();

		void				update();

	};
}