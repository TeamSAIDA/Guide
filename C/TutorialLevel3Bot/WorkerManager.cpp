#include "WorkerManager.h"

using namespace MyBot;

WorkerManager & WorkerManager::Instance()
{
	static WorkerManager instance;
	return instance;
}

WorkerManager::WorkerManager()
{
	for (auto & unit : BWAPI::Broodwar->getAllUnits())
	{
		if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field))
		{
			workerCountOnMineral[unit] = 0;
		}
	}
}


void WorkerManager::update() 
{
	updateWorkers1();
	
	//updateWorkers2();	

	//updateWorkers3();
}

void WorkerManager::updateWorkers1()
{
	for (auto & unit : BWAPI::Broodwar->self()->getUnits()){

		if (!unit) continue;

		if (unit->getType().isWorker()) {

			// unit 으로부터 가장 가까이에 있는 Mineral 을 찾아, 그 Mineral 로 Right Click 을 한다
			BWAPI::Unit closestMineral = getClosestMineralFrom(unit);

			if (closestMineral) {
				std::cout << "closestMineral from " << unit->getType().getName() << " " << unit->getID()
					<< " is " << closestMineral->getType().getName() << " " << closestMineral->getID() << " at " << closestMineral->getTilePosition().x << "," << closestMineral->getTilePosition().y << std::endl;

				// 매 frame 마다 Right Click 을 하는 것이므로, 결국 해당 unit 은 아무 일도 못하게 된다. 
				unit->gather(closestMineral);
			}
		}
	}

}

BWAPI::Unit WorkerManager::getClosestMineralFrom(BWAPI::Unit worker)
{
	if (!worker) return nullptr;

	// worker으로부터 가장 가까운 BaseLocation을 찾는다
	// BWTA::BaseLocation = 맵 상에서 Mineral / Gas Geyser 들이 주위에 있어서 Terran_Command_Center, Protoss_Nexus, Zerg_Hatchery 를 건설하기에 적당한 지역
	BWTA::BaseLocation * closestBaseLocation = nullptr;
	// 128 * 128 타일사이즈의 맵에서 가장 먼 거리는 sqrt(128 * 32  * 128 * 32 + 128 * 32 * 128 * 32) = 5792.6 point 
	double closestDistance = 1000000000;

	for (auto & baseLocation : BWTA::getBaseLocations()){

		if (!baseLocation) continue;

		double distance = worker->getDistance(baseLocation->getPosition());

		if (distance < closestDistance)
		{
			closestBaseLocation = baseLocation;
			closestDistance = distance;
		}
	}

	if (!closestBaseLocation) {
		return nullptr;
	}
	std::cout << "closestBaseLocation from " << worker->getType().getName() << " " << worker->getID()
		<< " is " << closestBaseLocation->getTilePosition().x << "," << closestBaseLocation->getTilePosition().y << std::endl;

	// 해당 BaseLocation 의 Mineral 들 중에서 worker에게 가장 가까운 Mineral 을 찾는다
	BWAPI::Unit closestMineral = nullptr;
	closestDistance = 1000000000;

	//BaseLocation->getMinerals() -> 어두운 영역에 있으면, null 을 리턴
	//BaseLocation->getStaticMinerals() -> 어두운 영역에 있으면, BWAPI::UnitTypes::Unknown 을 리턴
	for (auto & mineral : closestBaseLocation->getMinerals()){
		if (!mineral) continue;

		double distance = worker->getDistance(mineral->getPosition());

		if (distance < closestDistance)
		{
			closestMineral = mineral;
			closestDistance = distance;
		}
	}

	return closestMineral;
}

void WorkerManager::updateWorkers2()
{
	for (auto & unit : BWAPI::Broodwar->self()->getUnits()){

		if (!unit) continue;

		if (unit->getType().isWorker()) {

			if (unit->isIdle()) {
				std::cout << unit->getType().getName() << " " << unit->getID() << " is idle" << std::endl;
			}

			// unit 이 idle 상태이고, 탄생한 이후이면 
			if (unit->isIdle() && unit->isCompleted())
			{
				std::cout << unit->getType().getName() << " " << unit->getID() << " is ready to command" << std::endl;

				// unit 으로부터 가장 가까이에 있는 Mineral 을 찾아, 그 Mineral 로 Right Click 을 한다
				BWAPI::Unit closestMineral = getClosestMineralFrom(unit);

				if (closestMineral) {
					std::cout << "closestMineral from " << unit->getType().getName() << " " << unit->getID()
						<< " is " << closestMineral->getType().getName() << " " << closestMineral->getID() << " at " << closestMineral->getTilePosition().x << "," << closestMineral->getTilePosition().y << std::endl;

					unit->gather(closestMineral);
				}
			}
		}
	}
}

void WorkerManager::updateWorkers3()
{
	// 각각의 Worker 에 대해서
	// 가장 가까운 아군 ResourceDepot 근처의, 가장 가까운 Mineral 에 채취하도록 하되 (거리 계산 필요)
	// Worker 들이 여러 Mineral 에 분산되도록 한다 (각각의 Mineral 에 할당된 worker 들의 숫자를 저장 / 최신화 해야 한다)

	// worker 는 일이 없을 때도 idle 상태가 되지만, 일을 수행하는 도중에도 잠깐 idle 상태가 된다
	for (auto & unit : BWAPI::Broodwar->self()->getUnits()){

		if (!unit) continue;
		
		if (unit->getType().isWorker()) {

			// unit 이 idle 상태이고, 탄생한 이후이면 
			if (unit->isIdle() && unit->isCompleted())
			{
				std::cout << unit->getType().getName() << " " << unit->getID() << " is idle" << std::endl;

				// unit 에게 적절한 Mineral 을 찾아, 그 Mineral 로 Right Click 을 한다
				BWAPI::Unit bestMineral = getBestMineralTo(unit);

				if (bestMineral) {
					std::cout << "bestMineral from " << unit->getType().getName() << " " << unit->getID()
						<< " is " << bestMineral->getType().getName() << " " << bestMineral->getID() << " at " << bestMineral->getTilePosition().x << "," << bestMineral->getTilePosition().y << std::endl;

					unit->gather(bestMineral);

					// unit 과 Mineral 간 assign 정보를 업데이트한다
					workerMineralAssignment[unit] = bestMineral;
					// Mineral 별 assigned unit 숫자를 업데이트한다
					increaseWorkerCountOnMineral(bestMineral, 1);
				}
			}
		}
	}

	// Mineral 별 assigned unit 숫자를 화면에 표시
	for (auto & i : workerMineralAssignment) {
		if (i.first != nullptr && i.second != nullptr) {
			BWAPI::Unit mineral = i.second;
			if (workerCountOnMineral.find(mineral) != workerCountOnMineral.end()) {
				BWAPI::Broodwar->drawTextMap(mineral->getPosition().x, mineral->getPosition().y + 12, "worker: %d", workerCountOnMineral[mineral]);
			}
		}
	}
}

BWAPI::Unit WorkerManager::getBestMineralTo(BWAPI::Unit worker)
{
	if (!worker) return nullptr;

	// worker으로부터 가장 가까운 BaseLocation을 찾는다
	BWTA::BaseLocation * closestBaseLocation = nullptr;
	// 128 * 128 타일사이즈의 맵에서 가장 먼 거리는 sqrt(128 * 32  * 128 * 32 + 128 * 32 * 128 * 32) = 5792.6 point 
	double closestDistance = 1000000000;

	for (auto & baseLocation : BWTA::getBaseLocations()){

		if (!baseLocation) continue;

		double distance = worker->getDistance(baseLocation->getPosition());

		if (distance < closestDistance)
		{
			closestBaseLocation = baseLocation;
			closestDistance = distance;
		}
	}

	if (!closestBaseLocation) {
		return nullptr;
	}
	std::cout << "closestBaseLocation from " << worker->getType().getName() << " " << worker->getID()
		<< " is " << closestBaseLocation->getTilePosition().x << "," << closestBaseLocation->getTilePosition().y << std::endl;

	// 해당 BaseLocation 의 Mineral 들 중에서 worker 가 가장 적게 지정되어있는 것, 그중에서도 BaseLocation 으로부터 가장 가까운 것을 찾는다
	BWAPI::Unit bestMineral = nullptr;
	double bestDistance = 1000000000;
	int bestNumAssigned = 1000000000;

	//BaseLocation->getMinerals() -> 어두운 영역에 있으면, null 을 리턴
	//BaseLocation->getStaticMinerals() -> 어두운 영역에 있으면, BWAPI::UnitTypes::Unknown 을 리턴
	for (auto & mineral : closestBaseLocation->getMinerals()){
		if (!mineral) continue;

		// 해당 Mineral 에 지정된 worker 숫자
		int numAssigned = workerCountOnMineral.find(mineral) == workerCountOnMineral.end() ? 0 : workerCountOnMineral[mineral];
		// 해당 Mineral 과 BaseLocation 간의 거리
		double dist = mineral->getDistance(closestBaseLocation->getPosition());

		if (numAssigned < bestNumAssigned)
		{
			bestMineral = mineral;
			bestDistance = dist;
			bestNumAssigned = numAssigned;
		}
		else if (numAssigned == bestNumAssigned)
		{
			if (dist < bestDistance)
			{
				bestMineral = mineral;
				bestDistance = dist;
				bestNumAssigned = numAssigned;
			}
		}
	}

	return bestMineral;
}

void WorkerManager::increaseWorkerCountOnMineral(BWAPI::Unit mineral, int num)
{
	// Mineral 에 assign 된 worker 숫자를 변경한다
	if (workerCountOnMineral.find(mineral) == workerCountOnMineral.end())
	{
		workerCountOnMineral[mineral] = num;
	}
	else
	{
		workerCountOnMineral[mineral] = workerCountOnMineral[mineral] + num;
	}
}

void WorkerManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (!unit) return;

	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self()) 
	{
		// 해당 일꾼과 Mineral 간 assign 정보를 삭제한다
		std::cout << "removeWorker " << unit->getID() << " from Mineral Worker " << std::endl;
		increaseWorkerCountOnMineral(workerMineralAssignment[unit], -1);
		workerMineralAssignment.erase(unit);
	}
}
void WorkerManager::onUnitMorph(BWAPI::Unit unit)
{
	if (!unit) return;

	// 저그 종족 일꾼이 건물로 morph 한 경우
	if (unit->getPlayer()->getRace() == BWAPI::Races::Zerg && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType().isBuilding())
	{
		// 해당 일꾼과 Mineral 간 assign 정보를 삭제한다
		std::cout << "removeWorker " << unit->getID() << " from Mineral Worker " << std::endl;				
		increaseWorkerCountOnMineral(workerMineralAssignment[unit], -1);
		workerMineralAssignment.erase(unit);
	}
}

