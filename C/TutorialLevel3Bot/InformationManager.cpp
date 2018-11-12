#include "InformationManager.h"

using namespace MyBot;

InformationManager & InformationManager::Instance()
{
	static InformationManager instance;
	return instance;
}

InformationManager::InformationManager()
{
	selfPlayer = BWAPI::Broodwar->self();
	selfRace = selfPlayer->getRace();

	enemyPlayer = BWAPI::Broodwar->enemy();
	enemyRace = enemyPlayer->getRace();

	_mainBaseLocations[selfPlayer] = BWTA::getStartLocation(BWAPI::Broodwar->self());
	_mainBaseLocations[enemyPlayer] = nullptr;
}

void InformationManager::update() 
{
	// enemy 의 종족을 아직 모르는 경우
	if (enemyRace == BWAPI::Races::Unknown) {
		for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
		{
			enemyRace = unit->getType().getRace();
			break;
		}
	}

	// enemy 의 startLocation을 아직 모르는 경우
	if (_mainBaseLocations[enemyPlayer] == nullptr) {

		for (BWTA::BaseLocation * startLocation : BWTA::getStartLocations())
		{
			for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
			{
				if (unit->getType().isBuilding()) {
					if (BWTA::getRegion(unit->getTilePosition()) == BWTA::getRegion(startLocation->getTilePosition())) {
						_mainBaseLocations[enemyPlayer] = startLocation;
						break;
					}
				}
			}
		}
	}

}