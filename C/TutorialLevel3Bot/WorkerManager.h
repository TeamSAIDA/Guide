#pragma once

#include "Common.h"
#include "InformationManager.h"

namespace MyBot
{
	class WorkerManager
	{
		// Worker ~ Mineral Field 간 assign 관계를 저장하는 map
		std::map<BWAPI::Unit, BWAPI::Unit> workerMineralAssignment;

		// 각각의 Mineral Field 에 assign 된 Worker 숫자 를 저장하는 map
		std::map<BWAPI::Unit, int> workerCountOnMineral;

		WorkerManager();

	public:
		static		WorkerManager &  Instance();

		void        update();

		void		updateWorkers1();
		void		updateWorkers2();
		void		updateWorkers3();

		BWAPI::Unit getClosestMineralFrom(BWAPI::Unit worker);
		BWAPI::Unit getBestMineralTo(BWAPI::Unit worker);

		void        increaseWorkerCountOnMineral(BWAPI::Unit mineral, int num);

		void        onUnitDestroy(BWAPI::Unit unit);
		void        onUnitMorph(BWAPI::Unit unit);
	};
}