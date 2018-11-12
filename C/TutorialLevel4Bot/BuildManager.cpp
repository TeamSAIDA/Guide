#include "BuildManager.h"

using namespace MyBot;

BuildManager & BuildManager::Instance()
{
	static BuildManager instance;
	return instance;
}

BuildManager::BuildManager() 
{
}

void BuildManager::update()
{
	//constructBuildings();

	//buildCombatUnits();

	buildWorkerUnits();
}

void BuildManager::buildWorkerUnits()
{
	// 자원이 50이상 있으면 일꾼 유닛을 훈련한다
	if (BWAPI::Broodwar->self()->minerals() >= 50) {
		buildWorkerUnit();
	}
}

void BuildManager::buildWorkerUnit()
{
	BWAPI::Unit producer = nullptr;

	BWAPI::UnitType targetUnitType;

	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss) {
		targetUnitType = BWAPI::UnitTypes::Protoss_Probe;
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran) {
		targetUnitType = BWAPI::UnitTypes::Terran_SCV;
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) {
		targetUnitType = BWAPI::UnitTypes::Zerg_Drone;
	}
		
	// ResourceDepot 건물이 일꾼 유닛을 생산 가능한 상태이면 생산을 명령한다
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType().isResourceDepot() ){

			if (BWAPI::Broodwar->canMake(targetUnitType, unit) && unit->isTraining() == false && unit->isMorphing() == false) {

				producer = unit;

				if (BWAPI::Broodwar->self()->getRace() != BWAPI::Races::Zerg) {
					producer->train(targetUnitType);
				}
				else {
					producer->morph(targetUnitType);
				}
				break;
			}
		}
	}
}

void BuildManager::buildCombatUnits()
{
	BWAPI::UnitType targetUnitType;

	// 자원이 100이상 있으면 먼저 전투 유닛을 훈련한다
	if (BWAPI::Broodwar->self()->minerals() >= 100) {
		if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss) {
			targetUnitType = BWAPI::UnitTypes::Protoss_Zealot;
		}
		else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran) {
			targetUnitType = BWAPI::UnitTypes::Terran_Marine;
		}
		else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) {
			targetUnitType = BWAPI::UnitTypes::Zerg_Zergling;
		}

		trainUnit(targetUnitType);
	}
}

void BuildManager::trainUnit(BWAPI::UnitType targetUnitType)
{
	BWAPI::Unit producer = nullptr;
	BWAPI::UnitType producerUnitType = targetUnitType.whatBuilds().first;

	// targetUnitType을 생산 가능한 상태가 되면 생산을 명령한다
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() == producerUnitType) {
			if (BWAPI::Broodwar->canMake(targetUnitType, unit) && unit->isTraining() == false && unit->isMorphing() == false) {

				producer = unit;

				if (BWAPI::Broodwar->self()->getRace() != BWAPI::Races::Zerg) {
					producer->train(targetUnitType);
				}
				else {
					producer->morph(targetUnitType);
				}
				break;
			}
		}
	}
}


void BuildManager::constructBuildings()
{
	BWAPI::UnitType targetUnitType;

	// 자원이 200이상 있으면 전투유닛 생산 건물을 건설 한다
	if (BWAPI::Broodwar->self()->minerals() >= 200) {
		if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss) {
			targetUnitType = BWAPI::UnitTypes::Protoss_Gateway;
		}
		else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran) {
			targetUnitType = BWAPI::UnitTypes::Terran_Barracks;
		}
		else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) {
			targetUnitType = BWAPI::UnitTypes::Zerg_Spawning_Pool;
		}
		constructBuilding(targetUnitType);
	}

	// 자원이 100이상 있고, 서플라이가 부족해지면 SupplyProvider 에 해당하는 유닛을 만든다
	// 서플라이 숫자는 스타크래프트 게임에서 표시되는 숫자의 2배로 계산해야한다
	if (BWAPI::Broodwar->self()->minerals() >= 100
		&& BWAPI::Broodwar->self()->supplyUsed() + 6 > BWAPI::Broodwar->self()->supplyTotal()) {
		if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss) {
			targetUnitType = BWAPI::UnitTypes::Protoss_Pylon;
			constructBuilding(targetUnitType);
		}
		else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran) {
			targetUnitType = BWAPI::UnitTypes::Terran_Supply_Depot;
			constructBuilding(targetUnitType);
		}
		else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) {
			targetUnitType = BWAPI::UnitTypes::Zerg_Overlord;
			trainUnit(targetUnitType);
		}
	}
}

void BuildManager::constructBuilding(BWAPI::UnitType targetBuildingType)
{
	// 일꾼 중 미네랄을 운반하고 있지 않은 일꾼 하나를 producer로 선정한다
	BWAPI::Unit producer = nullptr;
	BWAPI::UnitType producerUnitType = targetBuildingType.whatBuilds().first;

	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() == producerUnitType) {
			if (BWAPI::Broodwar->canMake(targetBuildingType, unit)
				&& unit->isCompleted() 
				&& unit->isCarryingMinerals() == false
				&& unit->isConstructing() == false) {

				producer = unit;
				break;
			}
		}
	}

	if (producer == nullptr) {
		return;
	}

	// 건물을 건설할 위치를 Start Location 근처에서 찾는다
	// 처음에는 Start Location 반경 4타일에 대해 찾아보고, 
	// 다음에는 Start Location 반경 8타일에 대해 찾아보는 식으로 범위를 넓혀나간다
	BWAPI::TilePosition seedPosition = InformationManager::Instance()._mainBaseLocations[InformationManager::Instance().selfPlayer]->getTilePosition();
	BWAPI::TilePosition desiredPosition = BWAPI::TilePositions::None;
	int maxRange = 32;
	bool constructionPlaceFound = false;

	for (int range = 4; range <= maxRange; range *= 2) {
		for (int i = seedPosition.x - range; i < seedPosition.x + range; i++) {
			for (int j = seedPosition.y - range; j < seedPosition.y + range; j++) {
				desiredPosition.x = i;
				desiredPosition.y = j;
				if (BWAPI::Broodwar->canBuildHere(desiredPosition, targetBuildingType, producer, true))	{
					constructionPlaceFound = true;
					break;
				}
			}
			if (constructionPlaceFound) break;
		}
		if (constructionPlaceFound) break;
	}

	if (constructionPlaceFound == true && desiredPosition != BWAPI::TilePositions::None) {
		producer->build(targetBuildingType, desiredPosition);
	}
}


