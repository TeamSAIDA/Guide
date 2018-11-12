#include "ConstructionManager.h"

using namespace MyBot;

ConstructionManager::ConstructionManager()
    : reservedMinerals(0)
    , reservedGas(0)
{

}

// add a new building to be constructed
void ConstructionManager::addConstructionTask(BWAPI::UnitType type, BWAPI::TilePosition desiredPosition)
{
	if (type == BWAPI::UnitTypes::None || type == BWAPI::UnitTypes::Unknown) {
		return;
	}
	if (desiredPosition == BWAPI::TilePositions::None || desiredPosition == BWAPI::TilePositions::Invalid || desiredPosition == BWAPI::TilePositions::Unknown) {
		return;
	}

	ConstructionTask b(type, desiredPosition);
	b.status = ConstructionStatus::Unassigned;

	// reserve resources
	reservedMinerals += type.mineralPrice();
	reservedGas += type.gasPrice();

	constructionQueue.push_back(b);
}

// ConstructionTask 하나를 삭제한다
void ConstructionManager::cancelConstructionTask(BWAPI::UnitType type, BWAPI::TilePosition desiredPosition)
{
	reservedMinerals -= type.mineralPrice();
	reservedGas -= type.gasPrice();

	ConstructionTask b(type, desiredPosition);
    auto & it = std::find(constructionQueue.begin(), constructionQueue.end(), b);
    if (it != constructionQueue.end())
    {
		std::cout << std::endl << "Cancel Construction " << it->type.getName() << " at " << it->desiredPosition.x << "," << it->desiredPosition.y << std::endl;

		if (it->constructionWorker) {
			WorkerManager::Instance().setIdleWorker(it->constructionWorker);
		}
		if (it->finalPosition) {
			ConstructionPlaceFinder::Instance().freeTiles(it->finalPosition, it->type.tileWidth(), it->type.tileHeight());
		}
        constructionQueue.erase(it);
    }
}

// ConstructionTask 여러개를 삭제한다
// 건설을 시작했었던 ConstructionTask 이기 때문에 reservedMinerals, reservedGas 는 건드리지 않는다
void ConstructionManager::removeCompletedConstructionTasks(const std::vector<ConstructionTask> & toRemove)
{
	for (auto & b : toRemove)
	{		
		auto & it = std::find(constructionQueue.begin(), constructionQueue.end(), b);

		if (it != constructionQueue.end())
		{
		    constructionQueue.erase(it);
		}
	}
}


// gets called every frame from GameCommander
void ConstructionManager::update()
{
	// 1초에 1번만 실행한다
	//if (BWAPI::Broodwar->getFrameCount() % 24 != 0) return;

	// constructionQueue 에 들어있는 ConstructionTask 들은 
	// Unassigned -> Assigned (buildCommandGiven=false) -> Assigned (buildCommandGiven=true) -> UnderConstruction -> (Finished) 로 상태 변화된다

	validateWorkersAndBuildings();
	assignWorkersToUnassignedBuildings();
	checkForStartedConstruction();
	constructAssignedBuildings();
	checkForDeadTerranBuilders();
	checkForCompletedBuildings();
	checkForDeadlockConstruction();
}

// STEP 1: DO BOOK KEEPING ON WORKERS WHICH MAY HAVE DIED
void ConstructionManager::validateWorkersAndBuildings()
{
	std::vector<ConstructionTask> toRemove;

	for (auto & b : constructionQueue)
    {
		if (b.status == ConstructionStatus::UnderConstruction)
		{
			// 건설 진행 도중 (공격을 받아서) 건설하려던 건물이 파괴된 경우, constructionQueue 에서 삭제한다
			// 그렇지 않으면 (아마도 전투가 벌어지고있는) 기존 위치에 다시 건물을 지으려 할 것이기 때문.
			if (b.buildingUnit == nullptr || !b.buildingUnit->getType().isBuilding() || b.buildingUnit->getHitPoints() <= 0 || !b.buildingUnit->exists())
			{
				std::cout << "Construction Failed case -> remove ConstructionTask " << b.type.getName() << std::endl;

				toRemove.push_back(b);

				if (b.constructionWorker) {
					WorkerManager::Instance().setIdleWorker(b.constructionWorker);
				}
			}
		}
    }

	removeCompletedConstructionTasks(toRemove);
}

// STEP 2: ASSIGN WORKERS TO BUILDINGS WITHOUT THEM
void ConstructionManager::assignWorkersToUnassignedBuildings()
{

	// for each building that doesn't have a builder, assign one
    for (ConstructionTask & b : constructionQueue)
    {
        if (b.status != ConstructionStatus::Unassigned)
        {
            continue;
        }

		//std::cout << "find build place near desiredPosition " << b.desiredPosition.x << "," << b.desiredPosition.y << std::endl;

		// 건설 일꾼이 Unassigned 인 상태에서 getBuildLocationNear 로 건설할 위치를 다시 정한다. -> Assigned 
		BWAPI::TilePosition testLocation = ConstructionPlaceFinder::Instance().getBuildLocationNear(b.type, b.desiredPosition);

		// std::cout << "ConstructionPlaceFinder Selected Location : " << testLocation.x << "," << testLocation.y << std::endl;

		if (testLocation == BWAPI::TilePositions::None || testLocation == BWAPI::TilePositions::Invalid || testLocation.isValid() == false) {
						
			// 지금 건물 지을 장소를 전혀 찾을 수 없게 된 경우는, 
			// desiredPosition 주위에 다른 건물/유닛들이 있게 되었거나, Pylon 이 파괴되었거나, Creep 이 없어진 경우이고,
			// 대부분 다른 건물/유닛들이 있게된 경우이므로 다음 frame 에서 다시 지을 곳을 탐색한다
			continue;
		}

        // grab a worker unit from WorkerManager which is closest to this final position
		// 건설을 못하는 worker 가 계속 construction worker 로 선정될 수 있다. 직전에 선정되었었던 worker 는 다시 선정안하도록 한다
		BWAPI::Unit workerToAssign = WorkerManager::Instance().chooseConstuctionWorkerClosestTo(b.type, testLocation, true, b.lastConstructionWorkerID);
		
		//std::cout << "assignWorkersToUnassignedBuildings - chooseConstuctionWorkerClosest for " << b.type.getName().c_str() << " to worker near " << testLocation.x << "," << testLocation.y << std::endl;

        if (workerToAssign)
        {
			//std::cout << "set ConstuctionWorker " << workerToAssign->getID() << std::endl;

            b.constructionWorker = workerToAssign;
			
			b.finalPosition = testLocation;

			b.status = ConstructionStatus::Assigned;

			// reserve this building's space
			ConstructionPlaceFinder::Instance().reserveTiles(testLocation, b.type.tileWidth(), b.type.tileHeight());

			b.lastConstructionWorkerID = b.constructionWorker->getID();
        }
    }
}

// STEP 3: ISSUE CONSTRUCTION ORDERS TO ASSIGN BUILDINGS AS NEEDED
void ConstructionManager::constructAssignedBuildings()
{
    for (auto & b : constructionQueue)
    {
        if (b.status != ConstructionStatus::Assigned)
        {
            continue;
        }

		/*
		if (b.constructionWorker == nullptr) {
			std::cout << b.type.c_str() << " constructionWorker null" << std::endl;
		}
		else {
			std::cout << b.type.c_str() 
				<< " constructionWorker " << b.constructionWorker->getID()
				<< " exists " << b.constructionWorker->exists()
				<< " isIdle " << b.constructionWorker->isIdle()
				<< " isConstructing " << b.constructionWorker->isConstructing()
				<< " isMorphing " << b.constructionWorker->isMorphing() << std::endl;
		}
		*/

		// 일꾼에게 build 명령을 내리기 전에는 isConstructing = false 이다
		// 아직 탐색하지 않은 곳에 대해서는 build 명령을 내릴 수 없다
		// 일꾼에게 build 명령을 내리면, isConstructing = true 상태가 되어 이동을 하다가
		// build 를 실행할 수 없는 상황이라고 판단되면 isConstructing = false 상태가 된다
		// build 를 실행할 수 있으면, 프로토스 / 테란 종족의 경우 일꾼이 build 를 실행하고
		// 저그 종족 건물 중 Extractor 건물이 아닌 다른 건물의 경우 일꾼이 exists = true, isConstructing = true, isMorphing = true 가 되고, 일꾼 ID 가 건물 ID가 된다
		// 저그 종족 건물 중 Extractor 건물의 경우 일꾼이 exists = false, isConstructing = true, isMorphing = true 가 된 후, 일꾼 ID 가 건물 ID가 된다. 
		//                  Extractor 건물 빌드를 도중에 취소하면, 새로운 ID 를 가진 일꾼이 된다

		// 일꾼이 Assigned 된 후, UnderConstruction 상태로 되기 전, 즉 일꾼이 이동 중에 일꾼이 죽은 경우, 건물을 Unassigned 상태로 되돌려 일꾼을 다시 Assign 하도록 한다		
		if (b.constructionWorker == nullptr || b.constructionWorker->exists() == false || b.constructionWorker->getHitPoints() <= 0)
		{
			// 저그 종족 건물 중 Extractor 건물의 경우 일꾼이 exists = false 이지만 isConstructing = true 가 되므로, 일꾼이 죽은 경우가 아니다
			if (b.type == BWAPI::UnitTypes::Zerg_Extractor && b.constructionWorker != nullptr && b.constructionWorker->isConstructing() == true) {
				continue;
			}

			//std::cout << "unassign " << b.type.getName().c_str() << " worker " << b.constructionWorker->getID() << ", because it is not exists" << std::endl;

			// Unassigned 된 상태로 되돌린다
			WorkerManager::Instance().setIdleWorker(b.constructionWorker);

			// free the previous location in reserved
			ConstructionPlaceFinder::Instance().freeTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

			b.constructionWorker = nullptr;

			b.buildCommandGiven = false;

			b.finalPosition = BWAPI::TilePositions::None;

			b.status = ConstructionStatus::Unassigned;
		}
		// if that worker is not currently constructing
		// 일꾼이 build command 를 받으면 isConstructing = true 가 되고 건설을 하기위해 이동하는데,
		// isConstructing = false 가 되었다는 것은, build command 를 수행할 수 없어 게임에서 해당 임무가 취소되었다는 것이다
		else if (b.constructionWorker->isConstructing() == false)        
        {
            // if we haven't explored the build position, first we mush go there
			// 한번도 안가본 곳에는 build 커맨드 자체를 지시할 수 없으므로, 일단 그곳으로 이동하게 한다
            if (!isBuildingPositionExplored(b))
            {
                CommandUtil::move(b.constructionWorker,BWAPI::Position(b.finalPosition));
            }
			else if (b.buildCommandGiven == false)
            {
				//std::cout << b.type.c_str() << " build commanded to " << b.constructionWorker->getID() << ", buildCommandGiven true " << std::endl;

				// build command 
				b.constructionWorker->build(b.type, b.finalPosition);

				WorkerManager::Instance().setConstructionWorker(b.constructionWorker, b.type);

				// set the buildCommandGiven flag to true
				b.buildCommandGiven = true;

				b.lastBuildCommandGivenFrame = BWAPI::Broodwar->getFrameCount();

				b.lastConstructionWorkerID = b.constructionWorker->getID();
            }
			// if this is not the first time we've sent this guy to build this
			// 일꾼에게 build command 를 주었지만, 건설시작하기전에 도중에 자원이 미달하게 되었거나, 해당 장소에 다른 유닛들이 있어서 건설을 시작 못하게 되거나, Pylon 이나 Creep 이 없어진 경우 등이 발생할 수 있다
			// 이 경우, 해당 일꾼의 build command 를 해제하고, 건물 상태를 Unassigned 로 바꿔서, 다시 건물 위치를 정하고, 다른 일꾼을 지정하는 식으로 처리한다
			else
            {
				if (BWAPI::Broodwar->getFrameCount() - b.lastBuildCommandGivenFrame > 24) {

					//std::cout << b.type.c_str()
					//	<< " constructionWorker " << b.constructionWorker->getID()
					//	<< " buildCommandGiven " << b.buildCommandGiven
					//	<< " lastBuildCommandGivenFrame " << b.lastBuildCommandGivenFrame
					//	<< " lastConstructionWorkerID " << b.lastConstructionWorkerID
					//	<< " exists " << b.constructionWorker->exists()
					//	<< " isIdle " << b.constructionWorker->isIdle()
					//	<< " isConstructing " << b.constructionWorker->isConstructing()
					//	<< " isMorphing " << b.constructionWorker->isMorphing() << std::endl;

					//std::cout << b.type.c_str() << "(" << b.finalPosition.x << "," << b.finalPosition.y << ") buildCommandGiven -> but now Unassigned " << std::endl;

					// tell worker manager the unit we had is not needed now, since we might not be able
					// to get a valid location soon enough
					WorkerManager::Instance().setIdleWorker(b.constructionWorker);

					// free the previous location in reserved
					ConstructionPlaceFinder::Instance().freeTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

					// nullify its current builder unit
					b.constructionWorker = nullptr;

					// nullify its current builder unit
					b.finalPosition = BWAPI::TilePositions::None;

					// reset the build command given flag
					b.buildCommandGiven = false;

					// add the building back to be assigned
					b.status = ConstructionStatus::Unassigned;
				}
			}
        }
    }
}

// STEP 4: UPDATE DATA STRUCTURES FOR BUILDINGS STARTING CONSTRUCTION
void ConstructionManager::checkForStartedConstruction()
{
    // for each building unit which is being constructed
    for (auto & buildingThatStartedConstruction : BWAPI::Broodwar->self()->getUnits())
    {
        // filter out units which aren't buildings under construction
        if (!buildingThatStartedConstruction->getType().isBuilding() || !buildingThatStartedConstruction->isBeingConstructed())
        {
            continue;
        }

		//std::cout << "buildingThatStartedConstruction " << buildingThatStartedConstruction->getType().getName().c_str() << " isBeingConstructed at " << buildingThatStartedConstruction->getTilePosition().x << "," << buildingThatStartedConstruction->getTilePosition().y << std::endl;

        // check all our building status objects to see if we have a match and if we do, update it
        for (auto & b : constructionQueue)
        {
            if (b.status != ConstructionStatus::Assigned)
            {
                continue;
            }
        
            // check if the positions match.  Worker just started construction.
            if (b.finalPosition == buildingThatStartedConstruction->getTilePosition())
            {
				//std::cout << "construction " << b.type.getName().c_str() << " started at " << b.finalPosition.x << "," << b.finalPosition.y << std::endl;

                // the resources should now be spent, so unreserve them
                reservedMinerals -= buildingThatStartedConstruction->getType().mineralPrice();
                reservedGas      -= buildingThatStartedConstruction->getType().gasPrice();

                // flag it as started and set the buildingUnit
                b.underConstruction = true;

                b.buildingUnit = buildingThatStartedConstruction;

                // if we are zerg, make the buildingUnit nullptr since it's morphed or destroyed
				// Extractor 의 경우 destroyed 되고, 그외 건물의 경우 morphed 된다
                if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
                {
                    b.constructionWorker = nullptr;
                }
				// if we are protoss, give the worker back to worker manager
				else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
                {
                    WorkerManager::Instance().setIdleWorker(b.constructionWorker);

                    b.constructionWorker = nullptr;
                }

                // free this space
                ConstructionPlaceFinder::Instance().freeTiles(b.finalPosition,b.type.tileWidth(),b.type.tileHeight());

				// put it in the under construction vector
				b.status = ConstructionStatus::UnderConstruction;

				// only one building will match
                break;
            }
        }
    }
}

// STEP 5: IF WE ARE TERRAN, THIS MATTERS
// 테란은 건설을 시작한 후, 건설 도중에 일꾼이 죽을 수 있다. 이 경우, 건물에 대해 다시 다른 SCV를 할당한다
// 참고로, 프로토스 / 저그는 건설을 시작하면 일꾼 포인터를 nullptr 로 만들기 때문에 (constructionWorker = nullptr) 건설 도중에 죽은 일꾼을 신경쓸 필요 없다 
void ConstructionManager::checkForDeadTerranBuilders()
{
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran) {

		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Terran_SCV) <= 0) return;

		// for each of our buildings under construction
		for (auto & b : constructionQueue)
		{
			// if a terran building whose worker died mid construction, 
			// send the right click command to the buildingUnit to resume construction			
			if (b.status == ConstructionStatus::UnderConstruction) {

				if (b.buildingUnit->isCompleted()) continue;

				if (b.constructionWorker == nullptr || b.constructionWorker->exists() == false || b.constructionWorker->getHitPoints() <= 0 ) {

					//std::cout << "checkForDeadTerranBuilders - chooseConstuctionWorkerClosest for " << b.type.getName().c_str() << " to worker near " << b.finalPosition.x << "," << b.finalPosition.y << std::endl;

					// grab a worker unit from WorkerManager which is closest to this final position
					BWAPI::Unit workerToAssign = WorkerManager::Instance().chooseConstuctionWorkerClosestTo(b.type, b.finalPosition, true, b.lastConstructionWorkerID);

					if (workerToAssign)
					{
						//std::cout << "set ConstuctionWorker " << workerToAssign->getID() << std::endl;

						b.constructionWorker = workerToAssign;

						//b.status 는 계속 UnderConstruction 로 둔다. Assigned 로 바꾸면, 결국 Unassigned 가 되어서 새로 짓게 되기 때문이다
						//b.status = ConstructionStatus::Assigned;

						CommandUtil::rightClick(b.constructionWorker, b.buildingUnit);

						b.buildCommandGiven = true;

						b.lastBuildCommandGivenFrame = BWAPI::Broodwar->getFrameCount();

						b.lastConstructionWorkerID = b.constructionWorker->getID();
					}
				}
			}
		}
	}

}

// STEP 6: CHECK FOR COMPLETED BUILDINGS
void ConstructionManager::checkForCompletedBuildings()
{
    std::vector<ConstructionTask> toRemove;

    // for each of our buildings under construction
    for (auto & b : constructionQueue)
    {
        if (b.status != ConstructionStatus::UnderConstruction)
        {
            continue;       
        }

        // if the unit has completed
        if (b.buildingUnit->isCompleted())
        {
			//std::cout << "construction " << b.type.getName().c_str() << " completed at " << b.finalPosition.x << "," << b.finalPosition.y << std::endl;
			
			// if we are terran, give the worker back to worker manager
            if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
            {
                WorkerManager::Instance().setIdleWorker(b.constructionWorker);
            }

            // remove this unit from the under construction vector
            toRemove.push_back(b);
        }
    }

    removeCompletedConstructionTasks(toRemove);
}


void ConstructionManager::checkForDeadlockConstruction()
{
	std::vector<ConstructionTask> toCancel;
	for (auto & b : constructionQueue)
	{
		if (b.status != ConstructionStatus::UnderConstruction)
		{
			// BuildManager가 판단했을때 Construction 가능조건이 갖춰져서 ConstructionManager의 ConstructionQueue 에 들어갔는데, 
			// 선행 건물이 파괴되서 Construction을 수행할 수 없게 되었거나,
			// 일꾼이 다 사망하는 등 게임상황이 바뀌어서, 계속 ConstructionQueue 에 남아있게 되는 dead lock 상황이 된다 
			// 선행 건물을 BuildQueue에 추가해넣을지, 해당 ConstructionQueueItem 을 삭제할지 전략적으로 판단해야 한다
			BWAPI::UnitType unitType = b.type;
			BWAPI::UnitType producerType = b.type.whatBuilds().first;
			const std::map< BWAPI::UnitType, int >& requiredUnits = unitType.requiredUnits();
			BWTA::Region* desiredPositionRegion = BWTA::getRegion(b.desiredPosition);

			bool isDeadlockCase = false;

			// 건물을 생산하는 유닛이나, 유닛을 생산하는 건물이 존재하지 않고, 건설 예정이지도 않으면 dead lock
			if (BuildManager::Instance().isProducerWillExist(producerType) == false) {
				isDeadlockCase = true;
			}

			// Refinery 건물의 경우, 건물 지을 장소를 찾을 수 없게 되었거나, 건물 지을 수 있을거라고 판단했는데 이미 Refinery 가 지어져있는 경우, dead lock 
			if (!isDeadlockCase && unitType == InformationManager::Instance().getRefineryBuildingType())
			{
				bool hasAvailableGeyser = true;

				BWAPI::TilePosition testLocation;
				if (b.finalPosition != BWAPI::TilePositions::None && b.finalPosition != BWAPI::TilePositions::Invalid && b.finalPosition.isValid()) {
					testLocation = b.finalPosition;
				}
				else {
					testLocation = ConstructionPlaceFinder::Instance().getBuildLocationNear(b.type, b.desiredPosition);
				}

				// Refinery 를 지으려는 장소를 찾을 수 없으면 dead lock
				if (testLocation == BWAPI::TilePositions::None || testLocation == BWAPI::TilePositions::Invalid || testLocation.isValid() == false) {
					std::cout << "Construction Dead lock case -> Cann't find place to construct " << b.type.getName() << std::endl;
					hasAvailableGeyser = false;
				}
				else {
					// Refinery 를 지으려는 장소에 Refinery 가 이미 건설되어 있다면 dead lock 
					BWAPI::Unitset uot = BWAPI::Broodwar->getUnitsOnTile(testLocation);
					for (auto & u : uot) {
						if (u->getType().isRefinery() && u->exists() ) {
							hasAvailableGeyser = false;
							break;
						}
					}
					if (hasAvailableGeyser == false) {
						std::cout << "Construction Dead lock case -> Refinery Building was built already at " << testLocation.x << ", " << testLocation.y << std::endl;
					}
				}

				if (hasAvailableGeyser == false) {
					isDeadlockCase = true;
				}
			}

			// 정찰결과 혹은 전투결과, 건설 장소가 아군 점령 Region 이 아니고, 적군이 점령한 Region 이 되었으면 일반적으로는 현실적으로 dead lock 이 된다 
			// (포톤캐논 러시이거나, 적군 점령 Region 근처에서 테란 건물 건설하는 경우에는 예외일테지만..)
			if (!isDeadlockCase
				&& InformationManager::Instance().getOccupiedRegions(InformationManager::Instance().selfPlayer).find(desiredPositionRegion) == InformationManager::Instance().getOccupiedRegions(InformationManager::Instance().selfPlayer).end()
				&& InformationManager::Instance().getOccupiedRegions(InformationManager::Instance().enemyPlayer).find(desiredPositionRegion) != InformationManager::Instance().getOccupiedRegions(InformationManager::Instance().enemyPlayer).end())
			{
				isDeadlockCase = true;
			}

			// 선행 건물/유닛이 있는데 
			if (!isDeadlockCase && requiredUnits.size() > 0)
			{
				for (auto & u : requiredUnits)
				{
					BWAPI::UnitType requiredUnitType = u.first;

					if (requiredUnitType != BWAPI::UnitTypes::None) {

						// 선행 건물 / 유닛이 존재하지 않고, 생산 중이지도 않고
						if (BWAPI::Broodwar->self()->completedUnitCount(requiredUnitType) == 0
							&& BWAPI::Broodwar->self()->incompleteUnitCount(requiredUnitType) == 0)
						{
							// 선행 건물이 건설 예정이지도 않으면 dead lock
							if (requiredUnitType.isBuilding())
							{
								if (ConstructionManager::Instance().getConstructionQueueItemCount(requiredUnitType) == 0) {
									isDeadlockCase = true;
								}
							}
						}
					}
				}
			}

			if (isDeadlockCase) {
				toCancel.push_back(b);
			}
		}
	}

	for (auto & i : toCancel)
	{
		cancelConstructionTask(i.type, i.desiredPosition);
	}
}

// COMPLETED
bool ConstructionManager::isEvolvedBuilding(BWAPI::UnitType type) 
{
    if (type == BWAPI::UnitTypes::Zerg_Sunken_Colony ||
        type == BWAPI::UnitTypes::Zerg_Spore_Colony ||
        type == BWAPI::UnitTypes::Zerg_Lair ||
        type == BWAPI::UnitTypes::Zerg_Hive ||
        type == BWAPI::UnitTypes::Zerg_Greater_Spire)
    {
        return true;
    }

    return false;
}

bool ConstructionManager::isBuildingPositionExplored(const ConstructionTask & b) const
{
    BWAPI::TilePosition tile = b.finalPosition;

    // for each tile where the building will be built
    for (int x=0; x<b.type.tileWidth(); ++x)
    {
        for (int y=0; y<b.type.tileHeight(); ++y)
        {
            if (!BWAPI::Broodwar->isExplored(tile.x + x,tile.y + y))
            {
                return false;
            }
        }
    }

    return true;
}

int ConstructionManager::getReservedMinerals() 
{
    return reservedMinerals;
}

int ConstructionManager::getReservedGas() 
{
    return reservedGas;
}


ConstructionManager & ConstructionManager::Instance()
{
    static ConstructionManager instance;
    return instance;
}

std::vector<BWAPI::UnitType> ConstructionManager::buildingsQueued()
{
    std::vector<BWAPI::UnitType> buildingsQueued;

    for (const auto & b : constructionQueue)
    {
        if (b.status == ConstructionStatus::Unassigned || b.status == ConstructionStatus::Assigned)
        {
            buildingsQueued.push_back(b.type);
        }
    }

    return buildingsQueued;
}

// constructionQueue에 해당 type 의 Item 이 존재하는지 카운트한다. queryTilePosition 을 입력한 경우, 위치간 거리까지도 고려한다
int ConstructionManager::getConstructionQueueItemCount(BWAPI::UnitType queryType, BWAPI::TilePosition queryTilePosition)
{
	// queryTilePosition 을 입력한 경우, 거리의 maxRange. 타일단위
	int maxRange = 16;

	const BWAPI::Point<int, 32> queryTilePositionPoint(queryTilePosition.x, queryTilePosition.y);

	int count = 0;
	for (auto & b : constructionQueue)
	{
		if (b.type == queryType)
		{
			if (queryType.isBuilding() && queryTilePosition != BWAPI::TilePositions::None)
			{
				if (queryTilePositionPoint.getDistance(b.desiredPosition) <= maxRange) {
					count++;
				}
			}
			else {
				count++;
			}
		}
	}

	return count;
}

std::vector<ConstructionTask> * ConstructionManager::getConstructionQueue()
{
	return & constructionQueue;
}
