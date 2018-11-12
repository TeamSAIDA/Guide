#include "ScoutManager.h"
#include "BuildManager.h"
#include "MapTools.h"

using namespace MyBot;

ScoutManager::ScoutManager()
	: currentScoutUnit(nullptr)
	, currentScoutStatus(ScoutStatus::NoScout)
	, currentScoutTargetBaseLocation(nullptr)
	, currentScoutTargetPosition(BWAPI::Positions::None)
	, currentScoutFreeToVertexIndex(-1)
{
}

ScoutManager & ScoutManager::Instance()
{
	static ScoutManager instance;
	return instance;
}

void ScoutManager::update()
{
	// 1초에 4번만 실행합니다
	if (BWAPI::Broodwar->getFrameCount() % 6 != 0) return;

	// scoutUnit 을 지정하고, scoutUnit 의 이동을 컨트롤함. 
	assignScoutIfNeeded();
	moveScoutUnit();

	// 참고로, scoutUnit 의 이동에 의해 발견된 정보를 처리하는 것은 InformationManager.update() 에서 수행함
}

// 상대방 MainBaseLocation 위치를 모르면서, 정찰 유닛이 지정되어있지 않거나 정찰 유닛이 죽었으면, ResourceDepot 이 아닌 다른 건물이 있을 경우, 미네랄 일꾼 중에서 새로 지정.
// 상대방 MainBaseLocation 위치를 알고있으면, 정찰 유닛이 죽었어도 새로 지정 안함
void ScoutManager::assignScoutIfNeeded()
{
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());

	if (enemyBaseLocation == nullptr)
	{
		if (!currentScoutUnit || currentScoutUnit->exists() == false || currentScoutUnit->getHitPoints() <= 0)
		{
			currentScoutUnit = nullptr;
			currentScoutStatus = ScoutStatus::NoScout;

			// first building (Pylon / Supply Depot / Spawning Pool) 을 건설 시작한 후, 가장 가까이에 있는 Worker 를 정찰유닛으로 지정한다
			BWAPI::Unit firstBuilding = nullptr;

			for (auto & unit : BWAPI::Broodwar->self()->getUnits())
			{
				if (unit->getType().isBuilding() == true && unit->getType().isResourceDepot() == false)
				{
					firstBuilding = unit;
					break;
				}
			}

			if (firstBuilding)
			{
				// grab the closest worker to the first building to send to scout
				BWAPI::Unit unit = WorkerManager::Instance().getClosestMineralWorkerTo(firstBuilding->getPosition());

				// if we find a worker (which we should) add it to the scout units
				// 정찰 나갈 일꾼이 없으면, 아무것도 하지 않는다
				if (unit)
				{
					// set unit as scout unit
					currentScoutUnit = unit;
					WorkerManager::Instance().setScoutWorker(currentScoutUnit);

					// 참고로, 일꾼의 정찰 임무를 해제하려면, 다음과 같이 하면 된다
					//WorkerManager::Instance().setIdleWorker(currentScoutUnit);
				}
			}
		}
	}
}


// 상대방 MainBaseLocation 위치를 모르는 상황이면, StartLocation 들에 대해 아군의 MainBaseLocation에서 가까운 것부터 순서대로 정찰
// 상대방 MainBaseLocation 위치를 아는 상황이면, 해당 BaseLocation 이 있는 Region의 가장자리를 따라 계속 이동함 (정찰 유닛이 죽을때까지) 
void ScoutManager::moveScoutUnit()
{
	if (!currentScoutUnit || currentScoutUnit->exists() == false || currentScoutUnit->getHitPoints() <= 0 )
	{
		currentScoutUnit = nullptr;
		currentScoutStatus = ScoutStatus::NoScout;
		return;
	}

	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(InformationManager::Instance().enemyPlayer);
	BWTA::BaseLocation * myBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self());

	if (enemyBaseLocation == nullptr)
	{
		// currentScoutTargetBaseLocation 가 null 이거나 정찰 유닛이 currentScoutTargetBaseLocation 에 도착했으면 
		// 아군 MainBaseLocation 으로부터 가장 가까운 미정찰 BaseLocation 을 새로운 정찰 대상 currentScoutTargetBaseLocation 으로 잡아서 이동
		if (currentScoutTargetBaseLocation == nullptr || currentScoutUnit->getDistance(currentScoutTargetBaseLocation->getPosition()) < 5 * TILE_SIZE) 
		{
			currentScoutStatus = ScoutStatus::MovingToAnotherBaseLocation;
			
			int closestDistance = INT_MAX;
			int tempDistance = 0;
			BWTA::BaseLocation * closestBaseLocation = nullptr;
			for (BWTA::BaseLocation * startLocation : BWTA::getStartLocations())
			{
				// if we haven't explored it yet (방문했었던 곳은 다시 가볼 필요 없음)
				if (BWAPI::Broodwar->isExplored(startLocation->getTilePosition()) == false)
				{
					// GroundDistance 를 기준으로 가장 가까운 곳으로 선정
					tempDistance = (int)(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getGroundDistance(startLocation) + 0.5);
				
					if (tempDistance > 0 && tempDistance < closestDistance) {
						closestBaseLocation = startLocation;
						closestDistance = tempDistance;
					}
				}
			}

			if (closestBaseLocation) {
				// assign a scout to go scout it
				CommandUtil::move(currentScoutUnit, closestBaseLocation->getPosition());
				currentScoutTargetBaseLocation = closestBaseLocation;
			}
		}

	}
	// if we know where the enemy region is
	else 
	{
		// if scout is exist, move scout into enemy region
		if (currentScoutUnit) {

			currentScoutTargetBaseLocation = enemyBaseLocation;

			if (BWAPI::Broodwar->isExplored(currentScoutTargetBaseLocation->getTilePosition()) == false)
			{
				currentScoutStatus = ScoutStatus::MovingToAnotherBaseLocation;

				currentScoutTargetPosition = currentScoutTargetBaseLocation->getPosition();

				CommandUtil::move(currentScoutUnit, currentScoutTargetPosition);
			}
			else {

				//currentScoutStatus = ScoutStatus::MoveAroundEnemyBaseLocation;
				//currentScoutTargetPosition = getScoutFleePositionFromEnemyRegionVertices();
				//CommandUtil::move(currentScoutUnit, myBaseLocation->getPosition());

				WorkerManager::Instance().setIdleWorker(currentScoutUnit);
				currentScoutStatus = ScoutStatus::NoScout;
				currentScoutTargetPosition = myBaseLocation->getPosition();
			}
		}
	}

}

BWAPI::Position ScoutManager::getScoutFleePositionFromEnemyRegionVertices()
{
	// calculate enemy region vertices if we haven't yet
	if (enemyBaseRegionVertices.empty()) {
		calculateEnemyRegionVertices();
	}

	if (enemyBaseRegionVertices.empty()) {
		return BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
	}

	// if this is the first flee, we will not have a previous perimeter index
	if (currentScoutFreeToVertexIndex == -1)
	{
		// so return the closest position in the polygon
		int closestPolygonIndex = getClosestVertexIndex(currentScoutUnit);

		if (closestPolygonIndex == -1)
		{
			return BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
		}
		else
		{
			// set the current index so we know how to iterate if we are still fleeing later
			currentScoutFreeToVertexIndex = closestPolygonIndex;
			return enemyBaseRegionVertices[closestPolygonIndex];
		}
	}
	// if we are still fleeing from the previous frame, get the next location if we are close enough
	else
	{
		double distanceFromCurrentVertex = enemyBaseRegionVertices[currentScoutFreeToVertexIndex].getDistance(currentScoutUnit->getPosition());

		// keep going to the next vertex in the perimeter until we get to one we're far enough from to issue another move command
		while (distanceFromCurrentVertex < 128)
		{
			currentScoutFreeToVertexIndex = (currentScoutFreeToVertexIndex + 1) % enemyBaseRegionVertices.size();

			distanceFromCurrentVertex = enemyBaseRegionVertices[currentScoutFreeToVertexIndex].getDistance(currentScoutUnit->getPosition());
		}

		return enemyBaseRegionVertices[currentScoutFreeToVertexIndex];
	}
}

// Enemy MainBaseLocation 이 있는 Region 의 가장자리를  enemyBaseRegionVertices 에 저장한다
// Region 내 모든 건물을 Eliminate 시키기 위한 지도 탐색 로직 작성시 참고할 수 있다
void ScoutManager::calculateEnemyRegionVertices()
{
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	if (!enemyBaseLocation) {
		return;
	}

	BWTA::Region * enemyRegion = enemyBaseLocation->getRegion();
	if (!enemyRegion) {
		return;
	}

	// 아군 Main BaseLocation 으로부터 가까운 순서대로 정렬된 타일들의 전체 목록을 갖고와서, enemyRegion 에 해당하는 타일들만 추려내면, 
	// enemyRegion 의 타일 중 아군 Main BaseLocation 으로부터 가장 가까운 순서대로 정렬된 타일들의 목록을 만들 수 있다
	const BWAPI::Position basePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());	
	const std::vector<BWAPI::TilePosition> & closestTobase = MapTools::Instance().getClosestTilesTo(basePosition);

	std::set<BWAPI::Position> unsortedVertices;

	// check each tile position
	for (size_t i(0); i < closestTobase.size(); ++i)
	{
		const BWAPI::TilePosition & tp = closestTobase[i];

		if (BWTA::getRegion(tp) != enemyRegion)
		{
			continue;
		}

		// a tile is 'surrounded' if
		// 1) in all 4 directions there's a tile position in the current region
		// 2) in all 4 directions there's a buildable tile
		bool surrounded = true;
		if (BWTA::getRegion(BWAPI::TilePosition(tp.x + 1, tp.y)) != enemyRegion || !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x + 1, tp.y))
			|| BWTA::getRegion(BWAPI::TilePosition(tp.x, tp.y + 1)) != enemyRegion || !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x, tp.y + 1))
			|| BWTA::getRegion(BWAPI::TilePosition(tp.x - 1, tp.y)) != enemyRegion || !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x - 1, tp.y))
			|| BWTA::getRegion(BWAPI::TilePosition(tp.x, tp.y - 1)) != enemyRegion || !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x, tp.y - 1)))
		{
			surrounded = false;
		}

		// Region의 가장자리 타일들 (surrounded 되지 않은 타일들)만 추가한다
		// push the tiles that aren't surrounded 
		if (!surrounded && BWAPI::Broodwar->isBuildable(tp))
		{
			if (Config::Debug::DrawScoutInfo)
			{
				int x1 = tp.x * 32 + 2;
				int y1 = tp.y * 32 + 2;
				int x2 = (tp.x + 1) * 32 - 2;
				int y2 = (tp.y + 1) * 32 - 2;

				BWAPI::Broodwar->drawTextMap(x1 + 3, y1 + 2, "%d", BWTA::getGroundDistance(tp, BWAPI::Broodwar->self()->getStartLocation()));
				BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Green, false);
			}

			unsortedVertices.insert(BWAPI::Position(tp) + BWAPI::Position(16, 16));
		}
	}

	std::vector<BWAPI::Position> sortedVertices;
	BWAPI::Position current = *unsortedVertices.begin();

	enemyBaseRegionVertices.push_back(current);
	unsortedVertices.erase(current);

	// while we still have unsorted vertices left, find the closest one remaining to current
	while (!unsortedVertices.empty())
	{
		double bestDist = 1000000;
		BWAPI::Position bestPos;

		for (const BWAPI::Position & pos : unsortedVertices)
		{
			double dist = pos.getDistance(current);

			if (dist < bestDist)
			{
				bestDist = dist;
				bestPos = pos;
			}
		}

		current = bestPos;
		sortedVertices.push_back(bestPos);
		unsortedVertices.erase(bestPos);
	}

	// let's close loops on a threshold, eliminating death grooves
	int distanceThreshold = 100;

	while (true)
	{
		// find the largest index difference whose distance is less than the threshold
		int maxFarthest = 0;
		int maxFarthestStart = 0;
		int maxFarthestEnd = 0;

		// for each starting vertex
		for (int i(0); i < (int)sortedVertices.size(); ++i)
		{
			int farthest = 0;
			int farthestIndex = 0;

			// only test half way around because we'll find the other one on the way back
			for (size_t j(1); j < sortedVertices.size() / 2; ++j)
			{
				int jindex = (i + j) % sortedVertices.size();

				if (sortedVertices[i].getDistance(sortedVertices[jindex]) < distanceThreshold)
				{
					farthest = j;
					farthestIndex = jindex;
				}
			}

			if (farthest > maxFarthest)
			{
				maxFarthest = farthest;
				maxFarthestStart = i;
				maxFarthestEnd = farthestIndex;
			}
		}

		// stop when we have no long chains within the threshold
		if (maxFarthest < 4)
		{
			break;
		}

		double dist = sortedVertices[maxFarthestStart].getDistance(sortedVertices[maxFarthestEnd]);

		std::vector<BWAPI::Position> temp;

		for (size_t s(maxFarthestEnd); s != maxFarthestStart; s = (s + 1) % sortedVertices.size())
		{
			temp.push_back(sortedVertices[s]);
		}

		sortedVertices = temp;
	}

	enemyBaseRegionVertices = sortedVertices;
}

int ScoutManager::getClosestVertexIndex(BWAPI::Unit unit)
{
	int closestIndex = -1;
	double closestDistance = 10000000;

	for (size_t i(0); i < enemyBaseRegionVertices.size(); ++i)
	{
		double dist = unit->getDistance(enemyBaseRegionVertices[i]);
		if (dist < closestDistance)
		{
			closestDistance = dist;
			closestIndex = i;
		}
	}

	return closestIndex;
}
BWAPI::Unit ScoutManager::getScoutUnit()
{
	return currentScoutUnit;
}

int ScoutManager::getScoutStatus()
{
	return currentScoutStatus;
}

BWTA::BaseLocation * ScoutManager::getScoutTargetBaseLocation()
{
	return currentScoutTargetBaseLocation;
}

std::vector<BWAPI::Position> & ScoutManager::getEnemyRegionVertices()
{
	return enemyBaseRegionVertices;
}