#include "UXManager.h"

using namespace MyBot;

UXManager::UXManager()
{
}

UXManager & UXManager::Instance()
{
	static UXManager instance;
	return instance;
}


void UXManager::onStart()
{
}

void UXManager::update()
{
	//std::cout << 1;

	drawGameInformationOnScreen(5, 5);

	//std::cout << 2;

	if (Config::Debug::DrawEnemyUnitInfo)
	{
		drawUnitStatisticsOnScreen(400, 20);
	}

	//std::cout << 3;

	if (Config::Debug::DrawBWTAInfo)
	{
		drawBWTAResultOnMap();
	}

	//std::cout << 4;

	if (Config::Debug::DrawMapGrid)
	{
		drawMapGrid();
	}

	//std::cout << 5;

	// 빌드오더큐 : 빌드 실행 전
	if (Config::Debug::DrawProductionInfo)
	{
		drawBuildOrderQueueOnScreen(80, 60);
	}

	//std::cout << 6;

	// 빌드 실행 상황 : 건물 건설, 유닛 생산, 업그레이드, 리서치
	if (Config::Debug::DrawProductionInfo)
	{
		drawBuildStatusOnScreen(200, 60);
	}

	//std::cout << 7;

	// 건물 건설 큐. 건물 건설 상황
	if (Config::Debug::DrawBuildingInfo)
	{
		drawConstructionQueueOnScreenAndMap(200, 150);
	}

	//std::cout << 8;

	// 건물이 건설될 위치
	if (Config::Debug::DrawReservedBuildingTiles)
	{
		// 건물 건설 장소 예약 지점
		drawReservedBuildingTilesOnMap();
		// 건물 건설 불가 구역 (미네랄/가스/베이스 사이)
		drawTilesToAvoidOnMap();
	}

	//std::cout << 9;

	if (Config::Debug::DrawUnitHealthBars)
	{
		drawUnitExtendedInformationOnMap();
		drawUnitIdOnMap();
	}

	//std::cout << 10;

	if (Config::Debug::DrawWorkerInfo)
	{
		// 각 일꾼들의 임무 상황
		drawWorkerStateOnScreen(5, 60);

		// 베이스캠프당 일꾼 수
		drawWorkerCountOnMap();
	}

	//std::cout << 11;

	// 일꾼 자원채취 임무 상황
	if (Config::Debug::DrawResourceInfo)
	{
		drawWorkerMiningStatusOnMap();
	}

	//std::cout << 12;

	// 정찰
	if (Config::Debug::DrawScoutInfo)
	{
		drawScoutInformation(220,330);
	}

	//std::cout << 13;

	// 공격
	if (Config::Debug::DrawUnitTargetInfo)
	{
		drawUnitTargetOnMap();

		// 미사일, 럴커의 보이지않는 공격등을 표시
		drawBulletsOnMap();
	}
	
	//std::cout << 14;

	// draw tile position of mouse cursor
	if (Config::Debug::DrawMouseCursorInfo)
	{
		int mouseX = BWAPI::Broodwar->getMousePosition().x + BWAPI::Broodwar->getScreenPosition().x;
		int mouseY = BWAPI::Broodwar->getMousePosition().y + BWAPI::Broodwar->getScreenPosition().y;
		BWAPI::Broodwar->drawTextMap(mouseX + 20, mouseY, "(%d, %d)", (int)(mouseX/TILE_SIZE), (int)(mouseY/TILE_SIZE));
	}
}

void UXManager::drawGameInformationOnScreen(int x, int y)
{
	BWAPI::Broodwar->drawTextScreen(x, y, "\x04Players:");
	BWAPI::Broodwar->drawTextScreen(x + 50, y, "%c%s(%s) \x04vs. %c%s(%s)",
		BWAPI::Broodwar->self()->getTextColor(), BWAPI::Broodwar->self()->getName().c_str(), InformationManager::Instance().selfRace.c_str(),
		BWAPI::Broodwar->enemy()->getTextColor(), BWAPI::Broodwar->enemy()->getName().c_str(), InformationManager::Instance().enemyRace.c_str());
	y += 12;

	BWAPI::Broodwar->drawTextScreen(x, y, "\x04Map:");
	BWAPI::Broodwar->drawTextScreen(x + 50, y, "\x03%s (%d x %d size)", BWAPI::Broodwar->mapFileName().c_str(), BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight());
	BWAPI::Broodwar->setTextSize();
	y += 12;

	BWAPI::Broodwar->drawTextScreen(x, y, "\x04Time:");
	BWAPI::Broodwar->drawTextScreen(x + 50, y, "\x04%d", BWAPI::Broodwar->getFrameCount());
	BWAPI::Broodwar->drawTextScreen(x + 90, y, "\x04%4d:%3d", (int)(BWAPI::Broodwar->getFrameCount() / (23.8 * 60)), (int)((int)(BWAPI::Broodwar->getFrameCount() / 23.8) % 60));
}

void UXManager::drawAPM(int x, int y)
{
	int bwapiAPM = BWAPI::Broodwar->getAPM();
	BWAPI::Broodwar->drawTextScreen(x, y, "APM : %d", bwapiAPM);
}

void UXManager::drawPlayers()
{
	BWAPI::Playerset players = BWAPI::Broodwar->getPlayers();
	for (auto p : players)
		BWAPI::Broodwar << "Player [" << p->getID() << "]: " << p->getName() << " is in force: " << p->getForce()->getName() << std::endl;
}

void UXManager::drawForces()
{
	BWAPI::Forceset forces = BWAPI::Broodwar->getForces();
	for (auto f : forces)
	{
		BWAPI::Playerset players = f->getPlayers();
		BWAPI::Broodwar << "Force " << f->getName() << " has the following players:" << std::endl;
		for (auto p : players)
			BWAPI::Broodwar << "  - Player [" << p->getID() << "]: " << p->getName() << std::endl;
	}
}

void UXManager::drawUnitExtendedInformationOnMap()
{
	int verticalOffset = -10;

	// draw enemy units
	for (const auto & kv : InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getUnitAndUnitInfoMap())
	{
		const UnitInfo & ui(kv.second);

		BWAPI::UnitType type(ui.type);
		int hitPoints = ui.lastHealth;
		int shields = ui.lastShields;

		const BWAPI::Position & pos = ui.lastPosition;

		int left = pos.x - type.dimensionLeft();
		int right = pos.x + type.dimensionRight();
		int top = pos.y - type.dimensionUp();
		int bottom = pos.y + type.dimensionDown();

		// 적 유닛이면 주위에 박스 표시
		if (!BWAPI::Broodwar->isVisible(BWAPI::TilePosition(ui.lastPosition)))
		{
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, top), BWAPI::Position(right, bottom), BWAPI::Colors::Grey, false);
			BWAPI::Broodwar->drawTextMap(BWAPI::Position(left + 3, top + 4), "%s", ui.type.getName().c_str());
		}

		// 유닛의 HitPoint 남아있는 비율 표시
		if (!type.isResourceContainer() && type.maxHitPoints() > 0)
		{
			double hpRatio = (double)hitPoints / (double)type.maxHitPoints();

			BWAPI::Color hpColor = BWAPI::Colors::Green;
			if (hpRatio < 0.66) hpColor = BWAPI::Colors::Orange;
			if (hpRatio < 0.33) hpColor = BWAPI::Colors::Red;

			int ratioRight = left + (int)((right - left) * hpRatio);
			int hpTop = top + verticalOffset;
			int hpBottom = top + 4 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), hpColor, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

		// 유닛의 Shield 남아있는 비율 표시
		if (!type.isResourceContainer() && type.maxShields() > 0)
		{
			double shieldRatio = (double)shields / (double)type.maxShields();

			int ratioRight = left + (int)((right - left) * shieldRatio);
			int hpTop = top - 3 + verticalOffset;
			int hpBottom = top + 1 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Blue, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

	}

	// draw neutral units and our units
	for (auto & unit : BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy())
		{
			continue;
		}

		const BWAPI::Position & pos = unit->getPosition();

		int left = pos.x - unit->getType().dimensionLeft();
		int right = pos.x + unit->getType().dimensionRight();
		int top = pos.y - unit->getType().dimensionUp();
		int bottom = pos.y + unit->getType().dimensionDown();

		//BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, top), BWAPI::Position(right, bottom), BWAPI::Colors::Grey, false);

		// 유닛의 HitPoint 남아있는 비율 표시
		if (!unit->getType().isResourceContainer() && unit->getType().maxHitPoints() > 0)
		{
			double hpRatio = (double)unit->getHitPoints() / (double)unit->getType().maxHitPoints();

			BWAPI::Color hpColor = BWAPI::Colors::Green;
			if (hpRatio < 0.66) hpColor = BWAPI::Colors::Orange;
			if (hpRatio < 0.33) hpColor = BWAPI::Colors::Red;

			int ratioRight = left + (int)((right - left) * hpRatio);
			int hpTop = top + verticalOffset;
			int hpBottom = top + 4 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), hpColor, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

		// 유닛의 Shield 남아있는 비율 표시
		if (!unit->getType().isResourceContainer() && unit->getType().maxShields() > 0)
		{
			double shieldRatio = (double)unit->getShields() / (double)unit->getType().maxShields();

			int ratioRight = left + (int)((right - left) * shieldRatio);
			int hpTop = top - 3 + verticalOffset;
			int hpBottom = top + 1 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Blue, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

		// Mineral / Gas 가 얼마나 남아있는가
		if (unit->getType().isResourceContainer() && unit->getInitialResources() > 0)
		{

			double mineralRatio = (double)unit->getResources() / (double)unit->getInitialResources();

			int ratioRight = left + (int)((right - left) * mineralRatio);
			int hpTop = top + verticalOffset;
			int hpBottom = top + 4 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Cyan, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}
	}
}

// 아군이 입은 피해 누적값, 적군이 입은 피해 누적값, 적군의 UnitType 별 파악된 Unit 숫자를 표시
void UXManager::drawUnitStatisticsOnScreen(int x, int y)
{
	int currentY = y;

	// 아군이 입은 피해 누적값
	BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 Self Loss:\x04 Minerals: \x1f%d \x04Gas: \x07%d",
		InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getMineralsLost(), 
		InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getGasLost());
	currentY += 10;

	// 아군 모든 유닛 숫자 합계
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 allUnitCount: %d", BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	// 아군 건설/훈련 완료한 유닛 숫자 합계
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 completedUnitCount: %d", BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	// 아군 건설/훈련중인 유닛 숫자 합계
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 incompleteUnitCount: %d", BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	// 아군 유닛 파괴/사망 숫자 누적값
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 deadUnitCount: %d", BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	// 상대방 유닛을 파괴/사망 시킨 숫자 누적값
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 killedUnitCount: %d", BWAPI::Broodwar->self()->killedUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 UnitScore: %d", BWAPI::Broodwar->self()->getUnitScore());
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 RazingScore: %d", BWAPI::Broodwar->self()->getRazingScore());
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 BuildingScore: %d", BWAPI::Broodwar->self()->getBuildingScore());
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 KillScore: %d", BWAPI::Broodwar->self()->getKillScore());

	// 적군이 입은 피해 누적값
	BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 Enemy Loss:\x04 Minerals: \x1f%d \x04Gas: \x07%d",
		InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getMineralsLost(),
		InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getGasLost());

	// 적군의 UnitType 별 파악된 Unit 숫자를 표시
	BWAPI::Broodwar->drawTextScreen(x,		 currentY + 20, "\x04 UNIT NAME");
	BWAPI::Broodwar->drawTextScreen(x + 110, currentY + 20, "\x04 Created");
	BWAPI::Broodwar->drawTextScreen(x + 150, currentY + 20, "\x04 Dead");
	BWAPI::Broodwar->drawTextScreen(x + 190, currentY + 20, "\x04 Alive");

	int yspace = 0;
	for (BWAPI::UnitType t : BWAPI::UnitTypes::allUnitTypes())
	{
		int numCreatedUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getNumCreatedUnits(t);
		int numDeadUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getNumDeadUnits(t);
		int numUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getNumUnits(t);

		if (numUnits > 0)
		{
			BWAPI::Broodwar->drawTextScreen(x,		 currentY + 30 + ((yspace)* 10), "%s", t.getName().c_str());
			BWAPI::Broodwar->drawTextScreen(x + 120, currentY + 30 + ((yspace)* 10), "%d", numCreatedUnits);
			BWAPI::Broodwar->drawTextScreen(x + 160, currentY + 30 + ((yspace)* 10), "%d", numDeadUnits);
			BWAPI::Broodwar->drawTextScreen(x + 200, currentY + 30 + ((yspace)* 10), "%d", numUnits);
			yspace++;
		}
	}
	yspace++;

	// 아군의 UnitType 별 파악된 Unit 숫자를 표시
	for (BWAPI::UnitType t : BWAPI::UnitTypes::allUnitTypes())
	{
		int numCreatedUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getNumCreatedUnits(t);
		int numDeadUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getNumDeadUnits(t);
		int numUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getNumUnits(t);

		if (numUnits > 0)
		{
			BWAPI::Broodwar->drawTextScreen(x, currentY + 30 + ((yspace)* 10), "%s", t.getName().c_str());
			BWAPI::Broodwar->drawTextScreen(x + 120, currentY + 30 + ((yspace)* 10), "%d", numCreatedUnits);
			BWAPI::Broodwar->drawTextScreen(x + 160, currentY + 30 + ((yspace)* 10), "%d", numDeadUnits);
			BWAPI::Broodwar->drawTextScreen(x + 200, currentY + 30 + ((yspace)* 10), "%d", numUnits);
			yspace++;
		}
	}
}

void UXManager::drawBWTAResultOnMap()
{
	//we will iterate through all the base locations, and draw their outlines.
	for (std::set<BWTA::BaseLocation*>::const_iterator i = BWTA::getBaseLocations().begin(); i != BWTA::getBaseLocations().end(); i++)
	{
		BWAPI::TilePosition p = (*i)->getTilePosition();
		BWAPI::Position c = (*i)->getPosition();

		//draw outline of Base location 
		BWAPI::Broodwar->drawBoxMap(p.x * 32, p.y * 32, p.x * 32 + 4 * 32, p.y * 32 + 3 * 32, BWAPI::Colors::Blue);

		//draw a circle at each mineral patch
		for (BWAPI::Unitset::iterator j = (*i)->getStaticMinerals().begin(); j != (*i)->getStaticMinerals().end(); j++)
		{
			BWAPI::Position q = (*j)->getInitialPosition();
			BWAPI::Broodwar->drawCircleMap(q.x, q.y, 30, BWAPI::Colors::Cyan);
		}

		//draw the outlines of vespene geysers
		for (BWAPI::Unitset::iterator j = (*i)->getGeysers().begin(); j != (*i)->getGeysers().end(); j++)
		{
			BWAPI::TilePosition q = (*j)->getInitialTilePosition();
			BWAPI::Broodwar->drawBoxMap(q.x * 32, q.y * 32, q.x * 32 + 4 * 32, q.y * 32 + 2 * 32, BWAPI::Colors::Orange);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if ((*i)->isIsland())
			BWAPI::Broodwar->drawCircleMap(c, 80, BWAPI::Colors::Yellow);
	}

	//we will iterate through all the regions and draw the polygon outline of it in green.
	for (std::set<BWTA::Region*>::const_iterator r = BWTA::getRegions().begin(); r != BWTA::getRegions().end(); r++)
	{
		BWTA::Polygon p = (*r)->getPolygon();
		for (int j = 0; j<(int)p.size(); j++)
		{
			BWAPI::Position point1 = p[j];
			BWAPI::Position point2 = p[(j + 1) % p.size()];
			BWAPI::Broodwar->drawLineMap(point1, point2, BWAPI::Colors::Green);
		}
	}

	//we will visualize the chokepoints with red lines
	for (std::set<BWTA::Region*>::const_iterator r = BWTA::getRegions().begin(); r != BWTA::getRegions().end(); r++)
	{
		for (std::set<BWTA::Chokepoint*>::const_iterator c = (*r)->getChokepoints().begin(); c != (*r)->getChokepoints().end(); c++)
		{
			BWAPI::Position point1 = (*c)->getSides().first;
			BWAPI::Position point2 = (*c)->getSides().second;
			BWAPI::Broodwar->drawLineMap(point1, point2, BWAPI::Colors::Red);
		}
	}

	if (InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->self()) != nullptr) {
		BWAPI::Broodwar->drawTextMap(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition(), "My MainBaseLocation");
	}
	if (InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->self()) != nullptr) {
		BWAPI::Broodwar->drawTextMap(InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->self())->getCenter(), "My First ChokePoint");
	}
	if (InformationManager::Instance().getSecondChokePoint(BWAPI::Broodwar->self()) != nullptr) {
		BWAPI::Broodwar->drawTextMap(InformationManager::Instance().getSecondChokePoint(BWAPI::Broodwar->self())->getCenter(), "My Second ChokePoint");
	}
	if (InformationManager::Instance().getFirstExpansionLocation(BWAPI::Broodwar->self()) != nullptr) {
		BWAPI::Broodwar->drawTextMap(InformationManager::Instance().getFirstExpansionLocation(BWAPI::Broodwar->self())->getPosition(), "My First ExpansionLocation");
	}

	if (InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->enemy()) != nullptr) {
		BWAPI::Broodwar->drawTextMap(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getPosition(), "Enemy MainBaseLocation");
	}
	if (InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->enemy()) != nullptr) {
		BWAPI::Broodwar->drawTextMap(InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->enemy())->getCenter(), "Enemy First ChokePoint");
	}
	if (InformationManager::Instance().getSecondChokePoint(BWAPI::Broodwar->enemy()) != nullptr) {
		BWAPI::Broodwar->drawTextMap(InformationManager::Instance().getSecondChokePoint(BWAPI::Broodwar->enemy())->getCenter(), "Enemy Second ChokePoint");
	}
	if (InformationManager::Instance().getFirstExpansionLocation(BWAPI::Broodwar->enemy()) != nullptr) {
		BWAPI::Broodwar->drawTextMap(InformationManager::Instance().getFirstExpansionLocation(BWAPI::Broodwar->enemy())->getPosition(), "Enemy First ExpansionLocation");
	}

}

void UXManager::drawMapGrid()
{
	int	cellSize = MapGrid::Instance().getCellSize();
	int	mapWidth = MapGrid::Instance().getMapWidth();
	int mapHeight = MapGrid::Instance().getMapHeight();
	int	rows = MapGrid::Instance().getRows();
	int	cols = MapGrid::Instance().getCols();

	for (int i = 0; i<cols; i++)
	{
		BWAPI::Broodwar->drawLineMap(i*cellSize, 0, i*cellSize, mapHeight, BWAPI::Colors::Blue);
	}

	for (int j = 0; j<rows; j++)
	{
		BWAPI::Broodwar->drawLineMap(0, j*cellSize, mapWidth, j*cellSize, BWAPI::Colors::Blue);
	}

	for (int r = 0; r < rows; r+=2)
	{
		for (int c = 0; c < cols; c+=2)
		{
			GridCell & cell = MapGrid::Instance().getCellByIndex(r, c);

			BWAPI::Broodwar->drawTextMap(cell.center.x - cellSize / 2, cell.center.y - cellSize / 2, "%d,%d", c, r);
			//BWAPI::Broodwar->drawTextMap(cell.center.x, cell.center.y + 10, "Last seen at %d", cell.timeLastVisited);
		}
	}

}


void UXManager::drawBuildOrderQueueOnScreen(int x, int y)
{
	BWAPI::Broodwar->drawTextScreen(x, y, "\x04 <Build Order>");

	/*
	std::deque< BuildOrderItem >* queue = BuildManager::Instance().buildQueue.getQueue();
	size_t reps = queue->size() < 24 ? queue->size() : 24;
	for (size_t i(0); i<reps; i++) {
		const MetaType & type = (*queue)[queue->size() - 1 - i].metaType;
		BWAPI::Broodwar->drawTextScreen(x, y + 10 + (i * 10), " %s", type.getName().c_str());
	}
	*/

	std::deque<BuildOrderItem> * buildQueue = BuildManager::Instance().buildQueue.getQueue();
	int itemCount = 0;

	for (std::deque<BuildOrderItem>::reverse_iterator itr = buildQueue->rbegin(); itr != buildQueue->rend(); itr++) {
		BuildOrderItem & currentItem = *itr;
		BWAPI::Broodwar->drawTextScreen(x, y + 10 + (itemCount * 10), " %s %d", currentItem.metaType.getName().c_str(), currentItem.blocking);
		itemCount++;
		if (itemCount >= 24) break;
	}
}


void UXManager::drawBuildStatusOnScreen(int x, int y)
{
	// 건설 / 훈련 중인 유닛 진행상황 표시
	std::vector<BWAPI::Unit> unitsUnderConstruction;
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit != nullptr && unit->isBeingConstructed())
		{
			unitsUnderConstruction.push_back(unit);
		}
	}

	// sort it based on the time it was started
	std::sort(unitsUnderConstruction.begin(), unitsUnderConstruction.end(), CompareWhenStarted());

	BWAPI::Broodwar->drawTextScreen(x, y, "\x04 <Build Status>");

	size_t reps = unitsUnderConstruction.size() < 10 ? unitsUnderConstruction.size() : 10;

	std::string prefix = "\x07";
	for (auto & unit : unitsUnderConstruction)
	{
		y += 10;
		BWAPI::UnitType t = unit->getType();
		if (t == BWAPI::UnitTypes::Zerg_Egg)
		{
			t = unit->getBuildType();
		}

		BWAPI::Broodwar->drawTextScreen(x, y, " %s%s (%d)", prefix.c_str(), t.getName().c_str(), unit->getRemainingBuildTime());
	}

	// Tech Research 표시

	// Upgrade 표시

}

void UXManager::drawReservedBuildingTilesOnMap()
{
	std::vector< std::vector<bool> > & _reserveMap = ConstructionPlaceFinder::Instance().getReserveMap();
	int rwidth = _reserveMap.size();
	int rheight = _reserveMap[0].size();

	for (int x = 0; x < rwidth; ++x)
	{
		for (int y = 0; y < rheight; ++y)
		{
			if (_reserveMap[x][y])
			{
				int x1 = x * 32 + 8;
				int y1 = y * 32 + 8;
				int x2 = (x + 1) * 32 - 8;
				int y2 = (y + 1) * 32 - 8;

				BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Yellow, false);
			}
		}
	}
}
void UXManager::drawTilesToAvoidOnMap()
{
	std::set< BWAPI::TilePosition > & _tilesToAvoid = ConstructionPlaceFinder::Instance().getTilesToAvoid();
	for (auto & t : _tilesToAvoid)
	{
		int x1 = t.x * 32 + 8;
		int y1 = t.y * 32 + 8;
		int x2 = (t.x + 1) * 32 - 8;
		int y2 = (t.y + 1) * 32 - 8;

		BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Orange, false);
	}
}

void UXManager::drawConstructionQueueOnScreenAndMap(int x, int y)
{
	BWAPI::Broodwar->drawTextScreen(x, y, "\x04 <Construction Status>");

	int yspace = 0;

	std::vector<ConstructionTask> * constructionQueue = ConstructionManager::Instance().getConstructionQueue();

	for (const auto & b : *constructionQueue)
	{
		std::string constructionState = "";

		if (b.status == ConstructionStatus::Unassigned)
		{
			BWAPI::Broodwar->drawTextScreen(x, y + 10 + ((yspace)* 10), "\x03 %s - No Worker", b.type.getName().c_str());
		}
		else if (b.status == ConstructionStatus::Assigned)
		{
			if (b.constructionWorker == nullptr) {
				BWAPI::Broodwar->drawTextScreen(x, y + 10 + ((yspace)* 10), "\x03 %s - Assigned Worker Null", b.type.getName().c_str());
			}			
			else {
				BWAPI::Broodwar->drawTextScreen(x, y + 10 + ((yspace)* 10), "\x03 %s - Assigned Worker %d, Position (%d,%d)", b.type.getName().c_str(), b.constructionWorker->getID(), b.finalPosition.x, b.finalPosition.y);
			}

			int x1 = b.finalPosition.x * 32;
			int y1 = b.finalPosition.y * 32;
			int x2 = (b.finalPosition.x + b.type.tileWidth()) * 32;
			int y2 = (b.finalPosition.y + b.type.tileHeight()) * 32;

			BWAPI::Broodwar->drawLineMap(b.constructionWorker->getPosition().x, b.constructionWorker->getPosition().y, (x1 + x2) / 2, (y1 + y2) / 2, BWAPI::Colors::Orange);
			BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Red, false);
		}
		else if (b.status == ConstructionStatus::UnderConstruction)
		{
			BWAPI::Broodwar->drawTextScreen(x, y + 10 + ((yspace)* 10), "\x03 %s - Under Construction", b.type.getName().c_str());
		}
		yspace++;
	}
}



void UXManager::drawUnitIdOnMap() {
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		BWAPI::Broodwar->drawTextMap(unit->getPosition().x, unit->getPosition().y + 5, "\x07%d", unit->getID());
	}
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		BWAPI::Broodwar->drawTextMap(unit->getPosition().x, unit->getPosition().y + 5, "\x07%d", unit->getID());
	}
}



void UXManager::drawWorkerStateOnScreen(int x, int y)
{
	WorkerData  workerData = WorkerManager::Instance().getWorkerData();

	BWAPI::Broodwar->drawTextScreen(x, y, "\x04<Workers : %d>", workerData.getNumMineralWorkers());

	int yspace = 0;

	for (auto & unit : workerData.getWorkers())
	{
		if (!unit) continue;

		// Mineral / Gas / Idle Worker 는 표시 안한다
		if (workerData.getJobCode(unit) == 'M' || workerData.getJobCode(unit) == 'I' || workerData.getJobCode(unit) == 'G') {
			continue;
		}

		BWAPI::Broodwar->drawTextScreen(x, y + 10 + ((yspace)* 10), "\x03 %d", unit->getID());

		if (workerData.getJobCode(unit) == 'B') {
			BWAPI::Broodwar->drawTextScreen(x + 30, y + 10 + ((yspace++) * 10), "\x03 %c %s %c (%d, %d)", workerData.getJobCode(unit),
				unit->getBuildType().c_str(), unit->isConstructing() ? 'Y' : 'N', unit->getTilePosition().x, unit->getTilePosition().y);
		}
		else {
			BWAPI::Broodwar->drawTextScreen(x + 30, y + 10 + ((yspace++) * 10), "\x03 %c", workerData.getJobCode(unit));
		}
	}
}

void UXManager::drawWorkerCountOnMap()
{
	BWAPI::Unitset depots = WorkerManager::Instance().getWorkerData().getDepots();
	for (auto & depot : depots)
	{
		if (!depot) continue;

		int x = depot->getPosition().x - 64;
		int y = depot->getPosition().y - 32;

		BWAPI::Broodwar->drawBoxMap(x - 2, y - 1, x + 75, y + 14, BWAPI::Colors::Black, true);
		BWAPI::Broodwar->drawTextMap(x, y, "\x04 Workers: %d", WorkerManager::Instance().getWorkerData().getNumAssignedWorkers(depot));
	}
}

void UXManager::drawWorkerMiningStatusOnMap()
{
	WorkerData  workerData = WorkerManager::Instance().getWorkerData();

	for (auto & worker : workerData.getWorkers())
	{
		if (!worker) continue;

		BWAPI::Position pos = worker->getTargetPosition();

		BWAPI::Broodwar->drawTextMap(worker->getPosition().x, worker->getPosition().y - 5, "\x07%c", workerData.getJobCode(worker));
		
		BWAPI::Broodwar->drawLineMap(worker->getPosition().x, worker->getPosition().y, pos.x, pos.y, BWAPI::Colors::Cyan);

		/*
		// ResourceDepot ~ Worker 사이에 직선 표시
		BWAPI::Unit depot = workerData.getWorkerDepot(worker);
		if (depot) {
			BWAPI::Broodwar->drawLineMap(worker->getPosition().x, worker->getPosition().y, depot->getPosition().x, depot->getPosition().y, BWAPI::Colors::Orange);
		}
		*/
	}
}

void UXManager::drawScoutInformation(int x, int y)
{
	int currentScoutStatus = ScoutManager::Instance().getScoutStatus();
	std::string scoutStatusString;

	switch (currentScoutStatus) {
	case ScoutStatus::MovingToAnotherBaseLocation:
		scoutStatusString = "Moving To Another Base Location";
		break;
	case ScoutStatus::MoveAroundEnemyBaseLocation:
		scoutStatusString = "Move Around Enemy BaseLocation";
		break;
	case ScoutStatus::NoScout:
	default:
		scoutStatusString = "No Scout";
		break;
	}

	// get the enemy base location, if we have one
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());

	if (enemyBaseLocation != nullptr) {
		BWAPI::Broodwar->drawTextScreen(x, y, "Enemy MainBaseLocation : (%d, %d)", enemyBaseLocation->getTilePosition().x, enemyBaseLocation->getTilePosition().y);
	}
	else {
		BWAPI::Broodwar->drawTextScreen(x, y, "Enemy MainBaseLocation : Unknown");
	}

	if (currentScoutStatus == ScoutStatus::NoScout) {
		BWAPI::Broodwar->drawTextScreen(x, y + 10, "No Scout Unit");
	}
	else {
		BWAPI::Unit scoutUnit = ScoutManager::Instance().getScoutUnit();
		if (!scoutUnit) {

			BWAPI::Broodwar->drawTextScreen(x, y + 10, "Scout Unit : %s %d (%d, %d)", scoutUnit->getType().getName().c_str(), scoutUnit->getID(), scoutUnit->getTilePosition().x, scoutUnit->getTilePosition().y);

			BWAPI::Position scoutMoveTo = scoutUnit->getTargetPosition();

			if (scoutMoveTo && scoutMoveTo != BWAPI::Positions::None && scoutMoveTo.isValid()) {

				double currentScoutTargetDistance;

				if (currentScoutStatus == ScoutStatus::MovingToAnotherBaseLocation) {

					if (scoutUnit->getType().isFlyer()) {
						currentScoutTargetDistance = (int)(scoutUnit->getPosition().getDistance(scoutMoveTo));
					}
					else {
						currentScoutTargetDistance = BWTA::getGroundDistance(scoutUnit->getTilePosition(), BWAPI::TilePosition(scoutMoveTo.x / TILE_SIZE, scoutMoveTo.y / TILE_SIZE));
					}

					BWAPI::Broodwar->drawTextScreen(x, y + 20, "Target = (%d, %d) Distance = %4.0f",
						scoutMoveTo.x / TILE_SIZE, scoutMoveTo.y / TILE_SIZE,
						currentScoutTargetDistance);

				}
				/*
				else if (currentScoutStatus == ScoutStatus::MoveAroundEnemyBaseLocation) {

				std::vector<BWAPI::Position> vertices = ScoutManager::Instance().getEnemyRegionVertices();
				for (size_t i(0); i < vertices.size(); ++i)
				{
				BWAPI::Broodwar->drawCircleMap(vertices[i], 4, BWAPI::Colors::Green, false);
				BWAPI::Broodwar->drawTextMap(vertices[i], "%d", i);
				}

				BWAPI::Broodwar->drawCircleMap(scoutMoveTo, 5, BWAPI::Colors::Red, true);
				}
				*/
			}
		}
	}
}

void UXManager::drawUnitTargetOnMap() 
{
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit != nullptr && unit->isCompleted() && !unit->getType().isBuilding() && !unit->getType().isWorker())
		{
			BWAPI::Unit targetUnit = unit->getTarget();
			if (targetUnit != nullptr && targetUnit->getPlayer() != BWAPI::Broodwar->self()) {
				BWAPI::Broodwar->drawCircleMap(unit->getPosition(), dotRadius, BWAPI::Colors::Red, true);
				BWAPI::Broodwar->drawCircleMap(targetUnit->getTargetPosition(), dotRadius, BWAPI::Colors::Red, true);
				BWAPI::Broodwar->drawLineMap(unit->getPosition(), targetUnit->getTargetPosition(), BWAPI::Colors::Red);
			}
			else if (unit->isMoving()) {
				BWAPI::Broodwar->drawCircleMap(unit->getPosition(), dotRadius, BWAPI::Colors::Orange, true);
				BWAPI::Broodwar->drawCircleMap(unit->getTargetPosition(), dotRadius, BWAPI::Colors::Orange, true);
				BWAPI::Broodwar->drawLineMap(unit->getPosition(), unit->getTargetPosition(), BWAPI::Colors::Orange);
			}

		}
	}
}

// Bullet 을 Line 과 Text 로 표시한다. Cloaking Unit 의 Bullet 표시에 쓰인다
void UXManager::drawBulletsOnMap()
{
	for (auto &b : BWAPI::Broodwar->getBullets())
	{
		BWAPI::Position p = b->getPosition();
		double velocityX = b->getVelocityX();
		double velocityY = b->getVelocityY();

		// 아군 것이면 녹색, 적군 것이면 빨간색
		BWAPI::Broodwar->drawLineMap(p, p + BWAPI::Position((int)velocityX, (int)velocityY), b->getPlayer() == BWAPI::Broodwar->self() ? BWAPI::Colors::Green : BWAPI::Colors::Red);

		BWAPI::Broodwar->drawTextMap(p, "%c%s", b->getPlayer() == BWAPI::Broodwar->self() ? BWAPI::Text::Green : BWAPI::Text::Red, b->getType().c_str());
	}
}

