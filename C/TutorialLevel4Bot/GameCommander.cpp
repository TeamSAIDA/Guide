#include "GameCommander.h"

using namespace MyBot;

GameCommander::GameCommander()
{
}

void GameCommander::onStart() 
{
}

void GameCommander::onEnd(bool isWinner)
{
}

void GameCommander::onFrame()
{
	// 아군 베이스 위치. 적군 베이스 위치 정보를 저장/업데이트한다
	InformationManager::Instance().update();

	// 플레이어 정보 표시 - InformationManager 의 멤버변수 사용
	BWAPI::Broodwar->drawTextScreen(5, 5, "My Player: %c%s (%s) ",
		BWAPI::Broodwar->self()->getTextColor(), BWAPI::Broodwar->self()->getName().c_str(), InformationManager::Instance().selfRace.c_str());
	BWAPI::Broodwar->drawTextScreen(5, 15, "Enemy Player: %c%s (%s)",
		BWAPI::Broodwar->enemy()->getTextColor(), BWAPI::Broodwar->enemy()->getName().c_str(), InformationManager::Instance().enemyRace.c_str());

	// 현재 FrameCount 표시
	BWAPI::Broodwar->drawTextScreen(300, 100, "FrameCount: %d", BWAPI::Broodwar->getFrameCount());

	// 유닛 id 표시
	for (auto & unit : BWAPI::Broodwar->getAllUnits()) {
		BWAPI::Broodwar->drawTextMap(unit->getPosition().x, unit->getPosition().y, "%d", unit->getID());
		BWAPI::Broodwar->drawTextMap(unit->getPosition().x, unit->getPosition().y, "%d", unit->getID());
	}

	// 플레이어 Start Location 표시 - InformationManager 의 멤버변수 사용
	if (InformationManager::Instance()._mainBaseLocations[InformationManager::Instance().selfPlayer]) {
		BWAPI::Broodwar->drawTextScreen(200, 5, "Start Location: %d, %d",
			InformationManager::Instance()._mainBaseLocations[InformationManager::Instance().selfPlayer]->getTilePosition().x,
			InformationManager::Instance()._mainBaseLocations[InformationManager::Instance().selfPlayer]->getTilePosition().y);
	}
	if (InformationManager::Instance()._mainBaseLocations[InformationManager::Instance().enemyPlayer]) {
		BWAPI::Broodwar->drawTextScreen(200, 15, "Start Location: %d, %d",
			InformationManager::Instance()._mainBaseLocations[InformationManager::Instance().enemyPlayer]->getTilePosition().x,
			InformationManager::Instance()._mainBaseLocations[InformationManager::Instance().enemyPlayer]->getTilePosition().y);
	}

	// 일꾼 유닛에게 자원 채취를 명령한다
	WorkerManager::Instance().update();	

	// 유닛 훈련 및 건물 건설을 한다
	BuildManager::Instance().update();
}

void GameCommander::onUnitShow(BWAPI::Unit unit)			
{ 
}

void GameCommander::onUnitHide(BWAPI::Unit unit)			
{
}

void GameCommander::onUnitCreate(BWAPI::Unit unit)		
{ 
}

void GameCommander::onUnitComplete(BWAPI::Unit unit)
{
}

void GameCommander::onUnitDestroy(BWAPI::Unit unit)		
{
	WorkerManager::Instance().onUnitDestroy(unit);
}

void GameCommander::onUnitRenegade(BWAPI::Unit unit)
{
}

void GameCommander::onUnitMorph(BWAPI::Unit unit)
{ 
	WorkerManager::Instance().onUnitMorph(unit);
}

void GameCommander::onUnitDiscover(BWAPI::Unit unit)
{
}

void GameCommander::onUnitEvade(BWAPI::Unit unit)
{
}

