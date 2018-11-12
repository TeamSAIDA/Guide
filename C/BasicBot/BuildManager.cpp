#include "BuildManager.h"

using namespace MyBot;

BuildManager::BuildManager() 
{
}

// 빌드오더 큐에 있는 것에 대해 생산 / 건설 / 리서치 / 업그레이드를 실행한다
void BuildManager::update()
{
	// 1초(24프레임)에 4번 정도만 실행해도 충분하다
	if (BWAPI::Broodwar->getFrameCount() % 6 != 0) return;

	if (buildQueue.isEmpty()) {
		return;
	}

	// Dead Lock 을 체크해서 제거한다
	checkBuildOrderQueueDeadlockAndAndFixIt();
	// Dead Lock 제거후 Empty 될 수 있다
	if (buildQueue.isEmpty()) {
		return;
	}

	// the current item to be used
	BuildOrderItem currentItem = buildQueue.getHighestPriorityItem();

	//std::cout << "current HighestPriorityItem" << currentItem.metaType.getName() << std::endl;
	
	// while there is still something left in the buildQueue
	while (!buildQueue.isEmpty()) 
	{
		bool isOkToRemoveQueue = true;
		
		// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
		// 빌드 실행 유닛 (일꾼/건물) 결정 로직이 seedLocation 이나 seedLocationStrategy 를 잘 반영하도록 수정

		// seedPosition 을 도출한다
		BWAPI::Position seedPosition = BWAPI::Positions::None;
		if (currentItem.seedLocation != BWAPI::TilePositions::None && currentItem.seedLocation != BWAPI::TilePositions::Invalid 
			&& currentItem.seedLocation != BWAPI::TilePositions::Unknown && currentItem.seedLocation.isValid()) {
			seedPosition = BWAPI::Position(currentItem.seedLocation);
		}
		else {
			seedPosition = getSeedPositionFromSeedLocationStrategy(currentItem.seedLocationStrategy);
		}

		// this is the unit which can produce the currentItem
		BWAPI::Unit producer = getProducer(currentItem.metaType, seedPosition, currentItem.producerID);

		// BasicBot 1.1 Patch End //////////////////////////////////////////////////

		/*
		if (currentItem.metaType.isUnit() && currentItem.metaType.getUnitType().isBuilding() ) {
			if (producer) {
				std::cout << "Build " << currentItem.metaType.getName() << " producer : " << producer->getType().getName() << " ID : " << producer->getID() << std::endl;
			}
			else {
				std::cout << "Build " << currentItem.metaType.getName() << " producer nullptr" << std::endl;
			}
		}
		*/

		BWAPI::Unit secondProducer = nullptr;
		bool canMake;

		// 건물을 만들수 있는 유닛(일꾼)이나, 유닛을 만들수 있는 유닛(건물 or 유닛)이 있으면
		if (producer != nullptr) {

			// check to see if we can make it right now
			// 지금 해당 유닛을 건설/생산 할 수 있는지에 대해 자원, 서플라이, 테크 트리, producer 만을 갖고 판단한다
			canMake = canMakeNow(producer, currentItem.metaType);
			
			/*
			if (currentItem.metaType.isUnit() && currentItem.metaType.getUnitType().isBuilding() ) {
				std::cout << "Build " << currentItem.metaType.getName() << " canMakeNow : " << canMake << std::endl;
			}
			*/

			// 프로토스 종족 유닛 중 Protoss_Archon / Protoss_Dark_Archon 은 기존 Protoss_High_Templar / Protoss_Dark_Templar 두 유닛을 합체시키는 기술을 써서 만들기 때문에 
			// secondProducer 을 추가로 찾아 확인한다
			if (canMake) {
				if (currentItem.metaType.isUnit()) {
					if (currentItem.metaType.getUnitType() == BWAPI::UnitTypes::Protoss_Archon || currentItem.metaType.getUnitType() == BWAPI::UnitTypes::Protoss_Dark_Archon) {
						secondProducer = getAnotherProducer(producer, producer->getPosition());
						if (secondProducer == nullptr) {
							canMake = false;
						}
					}
				}
			}
		}

		// if we can make the current item, create it
		if (producer != nullptr && canMake == true)
		{			
			MetaType t = currentItem.metaType;

			if (t.isUnit())
			{
				if (t.getUnitType().isBuilding()) {

					// 저그 종족 건물 중 Zerg_Lair, Zerg_Hive, Zerg_Greater_Spire, Zerg_Sunken_Colony, Zerg_Spore_Colony 는 기존 건물을 Morph 시켜 만든다
					// Morph를 시작하면 isMorphing = true, isBeingConstructed = true, isConstructing = true 가 되고
					// 완성되면 isMorphing = false, isBeingConstructed = false, isConstructing = false, isCompleted = true 가 된다
					if (t.getUnitType().getRace() == BWAPI::Races::Zerg && t.getUnitType().whatBuilds().first.isBuilding())
					{
						producer->morph(t.getUnitType());
					}
					// 테란 Addon 건물의 경우 (Addon 건물을 지을수 있는지는 getProducer 함수에서 이미 체크완료)
					// 모건물이 Addon 건물 짓기 전에는 canBuildAddon = true, isConstructing = false, canCommand = true 이다가 
					// Addon 건물을 짓기 시작하면 canBuildAddon = false, isConstructing = true, canCommand = true 가 되고 (Addon 건물 건설 취소는 가능하나 Train 등 커맨드는 불가능)
					// 완성되면 canBuildAddon = false, isConstructing = false 가 된다
					else if (t.getUnitType().isAddon()) {
						
						//std::cout << "addon build start " << std::endl;
						
						producer->buildAddon(t.getUnitType());
						// 테란 Addon 건물의 경우 정상적으로 buildAddon 명령을 내려도 SCV가 모건물 근처에 있을 때 한동안 buildAddon 명령이 취소되는 경우가 있어서
						// 모건물이 isConstructing = true 상태로 바뀐 것을 확인한 후 buildQueue 에서 제거해야한다
						if (producer->isConstructing() == false) {
							isOkToRemoveQueue = false;
						}
						//std::cout << "8";
					}
					// 그외 대부분 건물의 경우
					else 
					{
						// ConstructionPlaceFinder 를 통해 건설 가능 위치 desiredPosition 를 알아내서
						// ConstructionManager 의 ConstructionTask Queue에 추가를 해서 desiredPosition 에 건설을 하게 한다. 
						// ConstructionManager 가 건설 도중에 해당 위치에 건설이 어려워지면 다시 ConstructionPlaceFinder 를 통해 건설 가능 위치를 desiredPosition 주위에서 찾을 것이다
						BWAPI::TilePosition desiredPosition = getDesiredPosition(t.getUnitType(), currentItem.seedLocation, currentItem.seedLocationStrategy);

						// std::cout << "BuildManager "
						//	<< currentItem.metaType.getUnitType().getName().c_str() << " desiredPosition " << desiredPosition.x << "," << desiredPosition.y << std::endl;

						if (desiredPosition != BWAPI::TilePositions::None) {
							// Send the construction task to the construction manager
							ConstructionManager::Instance().addConstructionTask(t.getUnitType(), desiredPosition);
						}
						else {
							// 건물 가능 위치가 없는 경우는, Protoss_Pylon 가 없거나, Creep 이 없거나, Refinery 가 이미 다 지어져있거나, 정말 지을 공간이 주위에 없는 경우인데,
							// 대부분의 경우 Pylon 이나 Hatchery가 지어지고 있는 중이므로, 다음 frame 에 건물 지을 공간을 다시 탐색하도록 한다. 
							std::cout << "There is no place to construct " << currentItem.metaType.getUnitType().getName().c_str()
								<< " strategy " << currentItem.seedLocationStrategy
								<< " seedPosition " << currentItem.seedLocation.x << "," << currentItem.seedLocation.y
								<< " desiredPosition " << desiredPosition.x << "," << desiredPosition.y << std::endl;

							isOkToRemoveQueue = false;
						}
					}
				}
				// 지상유닛 / 공중유닛의 경우
				else {
					// 저그 지상유닛 / 공중유닛
					if (t.getUnitType().getRace() == BWAPI::Races::Zerg)
					{
						// 저그 종족 유닛의 거의 대부분은 Morph 시켜 만든다
						if (t.getUnitType() != BWAPI::UnitTypes::Zerg_Infested_Terran)
						{
							producer->morph(t.getUnitType());
						}
						// 저그 종족 유닛 중 Zerg_Infested_Terran 은 Train 시켜 만든다
						else {
							producer->train(t.getUnitType());
						}
					}
					// 프로토스 지상유닛 / 공중유닛
					else if (t.getUnitType().getRace() == BWAPI::Races::Protoss)
					{						
						// 프로토스 종족 유닛 중 Protoss_Archon 은 기존 Protoss_High_Templar 두 유닛을 합체시키는 기술을 써서 만든다 
						if (t.getUnitType() == BWAPI::UnitTypes::Protoss_Archon)
						{
							producer->useTech(BWAPI::TechTypes::Archon_Warp, secondProducer);
						}
						// 프로토스 종족 유닛 중 Protoss_Dark_Archon 은 기존 Protoss_Dark_Templar 두 유닛을 합체시키는 기술을 써서 만든다 
						else if (t.getUnitType() == BWAPI::UnitTypes::Protoss_Dark_Archon)
						{
							producer->useTech( BWAPI::TechTypes::Dark_Archon_Meld, secondProducer);
						}
						else {
							producer->train(t.getUnitType());
						}
					}
					// 테란 지상유닛 / 공중유닛
					else {
						producer->train(t.getUnitType());
					}
				}
			}
			// if we're dealing with a tech research
			else if (t.isTech())
			{
				producer->research(t.getTechType());
			}
			else if (t.isUpgrade())
			{
				producer->upgrade(t.getUpgradeType());
			}

			//std::cout << std::endl << " build " << t.getName() << " started " << std::endl;
			
			// remove it from the buildQueue
			if (isOkToRemoveQueue) {
				buildQueue.removeCurrentItem();
			}
			
			// don't actually loop around in here
			break;
		}
		// otherwise, if we can skip the current item
		else if (buildQueue.canSkipCurrentItem())
		{
			// skip it and get the next one
			buildQueue.skipCurrentItem();
			currentItem = buildQueue.getNextItem();				
		}
		else 
		{
			// so break out
			break;
		}
	}
}

BWAPI::Unit BuildManager::getProducer(MetaType t, BWAPI::Position closestTo, int producerID)
{
    // get the type of unit that builds this
    BWAPI::UnitType producerType = t.whatBuilds();

    // make a set of all candidate producers
    BWAPI::Unitset candidateProducers;
    for (auto & unit : BWAPI::Broodwar->self()->getUnits())
    {
		if (unit == nullptr) continue;

		// reasons a unit can not train the desired type
		if (unit->getType() != producerType)                    { continue; }
		if (!unit->exists())	                                { continue; }
		if (!unit->isCompleted())                               { continue; }
		if (unit->isTraining())                                 { continue; }
		if (!unit->isPowered())                                 { continue; }
		// if unit is lifted, unit should land first
		if (unit->isLifted())                                   { continue; }

		if (producerID != -1 && unit->getID() != producerID)	{ continue; }
        
		// if the type requires an addon and the producer doesn't have one
		typedef std::pair<BWAPI::UnitType, int> ReqPair;
		for (const ReqPair & pair : t.getUnitType().requiredUnits())
		{
			BWAPI::UnitType requiredType = pair.first;
			if (requiredType.isAddon())
			{
				if (!unit->getAddon() || (unit->getAddon()->getType() != requiredType))
				{
					continue;
				}
			}
		}

        // if the type is an addon 
        if (t.getUnitType().isAddon())
        {
            // if the unit already has an addon, it can't make one
            if (unit->getAddon() != nullptr)					{ continue; }

			// 모건물은 건설되고 있는 중에는 isCompleted = false, isConstructing = true, canBuildAddon = false 이다가
			// 건설이 완성된 후 몇 프레임동안은 isCompleted = true 이지만, canBuildAddon = false 인 경우가 있다
			if (!unit->canBuildAddon() )						{ continue; }

            // if we just told this unit to build an addon, then it will not be building another one
            // this deals with the frame-delay of telling a unit to build an addon and it actually starting to build
            if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Build_Addon 
                && (BWAPI::Broodwar->getFrameCount() - unit->getLastCommandFrame() < 10)) 
            { 
                continue; 
            }

            bool isBlocked = false;

            // if the unit doesn't have space to build an addon, it can't make one
			BWAPI::TilePosition addonPosition(
				unit->getTilePosition().x + unit->getType().tileWidth(), 
				unit->getTilePosition().y + unit->getType().tileHeight() - t.getUnitType().tileHeight());
            
            for (int i=0; i<t.getUnitType().tileWidth(); ++i)
            {
                for (int j=0; j<t.getUnitType().tileHeight(); ++j)
                {
					BWAPI::TilePosition tilePos(addonPosition.x + i, addonPosition.y + j);

                    // if the map won't let you build here, we can't build it.  
					// 맵 타일 자체가 건설 불가능한 타일인 경우 + 기존 건물이 해당 타일에 이미 있는경우
                    if (!BWAPI::Broodwar->isBuildable(tilePos, true))
                    {
                        isBlocked = true;
                    }

                    // if there are any units on the addon tile, we can't build it
					// 아군 유닛은 Addon 지을 위치에 있어도 괜찮음. (적군 유닛은 Addon 지을 위치에 있으면 건설 안되는지는 아직 불확실함)
                    BWAPI::Unitset uot = BWAPI::Broodwar->getUnitsOnTile(tilePos.x, tilePos.y);
					for (auto & u : uot) {
						//std::cout << std::endl << "Construct " << t.getName() 
						//	<< " beside "<< unit->getType().getName() << "(" << unit->getID() <<")" 
						//	<< ", units on Addon Tile " << tilePos.x << "," << tilePos.y << " is " << u->getType().getName() << "(ID : " << u->getID() << " Player : " << u->getPlayer()->getName() << ")" 
						//	<< std::endl;
						if (u->getPlayer() != InformationManager::Instance().selfPlayer) {
							isBlocked = false;
						}
					}					
				}
            }

            if (isBlocked)
            {
                continue;
            }
        }

        // if we haven't cut it, add it to the set of candidates
        candidateProducers.insert(unit);
    }

    return getClosestUnitToPosition(candidateProducers, closestTo);
}

// Protoss_Archon / Protoss_Dark_Archon 를 만들기 위해 producer 와 같은 type의, producer 가 아닌 다른 Unit 중에서 가장 가까운 Unit을 찾는다
BWAPI::Unit BuildManager::getAnotherProducer(BWAPI::Unit producer, BWAPI::Position closestTo)
{
	if (producer == nullptr) return nullptr;

	BWAPI::Unit closestUnit = nullptr;

	BWAPI::Unitset candidateProducers;
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit == nullptr)									{ continue; }
		if (unit->getType() != producer->getType())             { continue; }
		if (unit->getID() == producer->getID())                 { continue; }
		if (!unit->isCompleted())                               { continue; }
		if (unit->isTraining())                                 { continue; }
		if (!unit->exists())                                    { continue; }
		if (unit->getHitPoints()+unit->getEnergy()<=0)          { continue; }

		candidateProducers.insert(unit);
	}

	return getClosestUnitToPosition(candidateProducers, closestTo);
}

BWAPI::Unit BuildManager::getClosestUnitToPosition(const BWAPI::Unitset & units, BWAPI::Position closestTo)
{
	if (units.size() == 0)
    {
        return nullptr;
    }

	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 빌드 실행 유닛 (일꾼/건물) 결정 로직이 seedLocation 이나 seedLocationStrategy 를 잘 반영하도록 수정

	// if we don't care where the unit is return the first one we have
	if (closestTo == BWAPI::Positions::None || closestTo == BWAPI::Positions::Invalid || closestTo == BWAPI::Positions::Unknown || closestTo.isValid() == false)
    {
        return *(units.begin());
    }

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////

    BWAPI::Unit closestUnit = nullptr;
    double minDist(1000000000);

	for (auto & unit : units) 
    {
		if (unit == nullptr) continue;

		double distance = unit->getDistance(closestTo);

		//std::cout << "distance to " << unit->getType().getName() << " is " << distance << std::endl;

		if (!closestUnit || distance < minDist) 
        {
			closestUnit = unit;
			minDist = distance;
		}
	}

    return closestUnit;
}

// 지금 해당 유닛을 건설/생산 할 수 있는지에 대해 자원, 서플라이, 테크 트리, producer 만을 갖고 판단한다
// 해당 유닛이 건물일 경우 건물 지을 위치의 적절 여부 (탐색했었던 타일인지, 건설 가능한 타일인지, 주위에 Pylon이 있는지, Creep이 있는 곳인지 등) 는 판단하지 않는다
bool BuildManager::canMakeNow(BWAPI::Unit producer, MetaType t)
{
	if (producer == nullptr) {
		return false;
	}

	bool canMake = hasEnoughResources(t);

	if (canMake)
	{
		if (t.isUnit())
		{
			// BWAPI::Broodwar->canMake : Checks all the requirements include resources, supply, technology tree, availability, and required units
			canMake = BWAPI::Broodwar->canMake(t.getUnitType(), producer);
		}
		else if (t.isTech())
		{
			canMake = BWAPI::Broodwar->canResearch(t.getTechType(), producer);
		}
		else if (t.isUpgrade())
		{
			canMake = BWAPI::Broodwar->canUpgrade(t.getUpgradeType(), producer);
		}
	}

	return canMake;
}

// 건설 가능 위치를 찾는다
// seedLocationStrategy 가 SeedPositionSpecified 인 경우에는 그 근처만 찾아보고, SeedPositionSpecified 이 아닌 경우에는 seedLocationStrategy 를 조금씩 바꿔가며 계속 찾아본다.
// (MainBase -> MainBase 주위 -> MainBase 길목 -> MainBase 가까운 앞마당 -> MainBase 가까운 앞마당의 길목 -> 다른 멀티 위치 -> 탐색 종료)
BWAPI::TilePosition BuildManager::getDesiredPosition(BWAPI::UnitType unitType, BWAPI::TilePosition seedPosition, BuildOrderItem::SeedPositionStrategy seedPositionStrategy)
{
	BWAPI::TilePosition desiredPosition = ConstructionPlaceFinder::Instance().getBuildLocationWithSeedPositionAndStrategy(unitType, seedPosition, seedPositionStrategy);

	/*
	 std::cout << "ConstructionPlaceFinder getBuildLocationWithSeedPositionAndStrategy "
		<< unitType.getName().c_str()
		<< " strategy " << seedPositionStrategy
		<< " seedPosition " << seedPosition.x << "," << seedPosition.y
		<< " desiredPosition " << desiredPosition.x << "," << desiredPosition.y << std::endl;
	*/

	// desiredPosition 을 찾을 수 없는 경우
	bool findAnotherPlace = true;
	while (desiredPosition == BWAPI::TilePositions::None) {

		switch (seedPositionStrategy) {
		case BuildOrderItem::SeedPositionStrategy::MainBaseLocation:
			seedPositionStrategy = BuildOrderItem::SeedPositionStrategy::MainBaseBackYard;
			break;
		case BuildOrderItem::SeedPositionStrategy::MainBaseBackYard:
			seedPositionStrategy = BuildOrderItem::SeedPositionStrategy::FirstChokePoint;
			break;
		case BuildOrderItem::SeedPositionStrategy::FirstChokePoint:
			seedPositionStrategy = BuildOrderItem::SeedPositionStrategy::FirstExpansionLocation;
			break;
		case BuildOrderItem::SeedPositionStrategy::FirstExpansionLocation:
			seedPositionStrategy = BuildOrderItem::SeedPositionStrategy::SecondChokePoint;
			break;
		case BuildOrderItem::SeedPositionStrategy::SecondChokePoint:
		case BuildOrderItem::SeedPositionStrategy::SeedPositionSpecified:
		default:
			findAnotherPlace = false;
			break;
		}

		// 다른 곳을 더 찾아본다
		if (findAnotherPlace) {
			desiredPosition = ConstructionPlaceFinder::Instance().getBuildLocationWithSeedPositionAndStrategy(unitType, seedPosition, seedPositionStrategy);
			/*
			 std::cout << "ConstructionPlaceFinder getBuildLocationWithSeedPositionAndStrategy "
				<< unitType.getName().c_str()
				<< " strategy " << seedPositionStrategy
				<< " seedPosition " << seedPosition.x << "," << seedPosition.y
				<< " desiredPosition " << desiredPosition.x << "," << desiredPosition.y << std::endl;
			*/
		}
		// 다른 곳을 더 찾아보지 않고, 끝낸다
		else {
			break;
		}
	}

	return desiredPosition;
}

// 사용가능 미네랄 = 현재 보유 미네랄 - 사용하기로 예약되어있는 미네랄
int BuildManager::getAvailableMinerals()
{
	return BWAPI::Broodwar->self()->minerals() - ConstructionManager::Instance().getReservedMinerals();
}

// 사용가능 가스 = 현재 보유 가스 - 사용하기로 예약되어있는 가스
int BuildManager::getAvailableGas()
{
	return BWAPI::Broodwar->self()->gas() - ConstructionManager::Instance().getReservedGas();
}

// return whether or not we meet resources, including building reserves
bool BuildManager::hasEnoughResources(MetaType type) 
{
	// return whether or not we meet the resources
	return (type.mineralPrice() <= getAvailableMinerals()) && (type.gasPrice() <= getAvailableGas());
}


// selects a unit of a given type
BWAPI::Unit BuildManager::selectUnitOfType(BWAPI::UnitType type, BWAPI::Position closestTo) 
{
	// if we have none of the unit type, return nullptr right away
	if (BWAPI::Broodwar->self()->completedUnitCount(type) == 0) 
	{
		return nullptr;
	}

	BWAPI::Unit unit = nullptr;

	// if we are concerned about the position of the unit, that takes priority
    if (closestTo != BWAPI::Positions::None) 
    {
		double minDist(1000000000);

		for (auto & u : BWAPI::Broodwar->self()->getUnits()) 
        {
			if (u->getType() == type) 
            {
				double distance = u->getDistance(closestTo);
				if (!unit || distance < minDist) {
					unit = u;
					minDist = distance;
				}
			}
		}

	// if it is a building and we are worried about selecting the unit with the least
	// amount of training time remaining
	} 
    else if (type.isBuilding()) 
    {
		for (auto & u : BWAPI::Broodwar->self()->getUnits()) 
        {
			if (u->getType() == type && u->isCompleted() && !u->isTraining() && !u->isLifted() &&u->isPowered()) {

				return u;
			}
		}
		// otherwise just return the first unit we come across
	} 
    else 
    {
		for (auto & u : BWAPI::Broodwar->self()->getUnits()) 
		{
			if (u->getType() == type && u->isCompleted() && u->getHitPoints() > 0 && !u->isLifted() &&u->isPowered()) 
			{
				return u;
			}
		}
	}

	// return what we've found so far
	return nullptr;
}


BuildManager & BuildManager::Instance()
{
	static BuildManager instance;
	return instance;
}

BuildOrderQueue * BuildManager::getBuildQueue()
{
	return & buildQueue;
}

BWAPI::Position	BuildManager::getSeedPositionFromSeedLocationStrategy(BuildOrderItem::SeedPositionStrategy seedPositionStrategy)
{
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 빌드 실행 유닛 (일꾼/건물) 결정 로직이 seedLocation 이나 seedLocationStrategy 를 잘 반영하도록 수정

	BWAPI::Position seedPosition = BWAPI::Positions::None;
	BWTA::Chokepoint* tempChokePoint;
	BWTA::BaseLocation* tempBaseLocation;
	BWAPI::TilePosition tempTilePosition;
	BWTA::Region* tempBaseRegion;
	int vx, vy;
	double d, t;
	int bx, by;

	switch (seedPositionStrategy) {

	case BuildOrderItem::SeedPositionStrategy::MainBaseLocation:
		tempBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self());

		if (tempBaseLocation) {
			seedPosition = tempBaseLocation->getPosition();
		}
		break;

	case BuildOrderItem::SeedPositionStrategy::MainBaseBackYard:
		tempBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self());
		tempChokePoint = InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->self());
		tempBaseRegion = BWTA::getRegion(tempBaseLocation->getPosition());

		//std::cout << "y";

		// (vx, vy) = BaseLocation 와 ChokePoint 간 차이 벡터 = 거리 d 와 각도 t 벡터. 단위는 position
		// 스타크래프트 좌표계 : 오른쪽으로 갈수록 x 가 증가 (데카르트 좌표계와 동일). 아래로 갈수록 y가 증가 (y축만 데카르트 좌표계와 반대)
		// 삼각함수 값은 데카르트 좌표계에서 계산하므로, vy를 부호 반대로 해서 각도 t 값을 구함 

		// MainBaseLocation 이 null 이거나, ChokePoint 가 null 이면, MainBaseLocation 주위에서 가능한 곳을 리턴한다
		if (tempBaseLocation != nullptr && tempChokePoint != nullptr) {

			// BaseLocation 에서 ChokePoint 로의 벡터를 구한다
			vx = tempChokePoint->getCenter().x - tempBaseLocation->getPosition().x;
			//std::cout << "vx : " << vx ;
			vy = (tempChokePoint->getCenter().y - tempBaseLocation->getPosition().y) * (-1);
			//std::cout << "vy : " << vy;
			d = std::sqrt(vx * vx + vy * vy) * 0.5; // BaseLocation 와 ChokePoint 간 거리보다 조금 짧은 거리로 조정. BaseLocation가 있는 Region은 대부분 직사각형 형태이기 때문
			//std::cout << "d : " << d;
			t = std::atan2(vy, vx + 0.0001); // 라디안 단위
			//std::cout << "t : " << t;

			// cos(t+90), sin(t+180) 등 삼각함수 Trigonometric functions of allied angles 을 이용. y축에 대해서는 반대부호로 적용

			// BaseLocation 에서 ChokePoint 반대쪽 방향의 Back Yard : 데카르트 좌표계에서 (cos(t+180) = -cos(t), sin(t+180) = -sin(t))
			bx = tempBaseLocation->getTilePosition().x - (int)(d * std::cos(t) / TILE_SIZE);
			by = tempBaseLocation->getTilePosition().y + (int)(d * std::sin(t) / TILE_SIZE);
			//std::cout << "i";
			tempTilePosition = BWAPI::TilePosition(bx, by);
			// std::cout << "ConstructionPlaceFinder MainBaseBackYard tempTilePosition " << tempTilePosition.x << "," << tempTilePosition.y << std::endl;

			//std::cout << "k";
			// 해당 지점이 같은 Region 에 속하고 Buildable 한 타일인지 확인
			if (!tempTilePosition.isValid() || !BWAPI::Broodwar->isBuildable(tempTilePosition.x, tempTilePosition.y, false) || tempBaseRegion != BWTA::getRegion(BWAPI::Position(bx*TILE_SIZE, by*TILE_SIZE))) {
				//std::cout << "l";

				// BaseLocation 에서 ChokePoint 방향에 대해 오른쪽으로 90도 꺾은 방향의 Back Yard : 데카르트 좌표계에서 (cos(t-90) = sin(t),   sin(t-90) = - cos(t))
				bx = tempBaseLocation->getTilePosition().x + (int)(d * std::sin(t) / TILE_SIZE);
				by = tempBaseLocation->getTilePosition().y + (int)(d * std::cos(t) / TILE_SIZE);
				tempTilePosition = BWAPI::TilePosition(bx, by);
				// std::cout << "ConstructionPlaceFinder MainBaseBackYard tempTilePosition " << tempTilePosition.x << "," << tempTilePosition.y << std::endl;
				//std::cout << "m";

				if (!tempTilePosition.isValid() || !BWAPI::Broodwar->isBuildable(tempTilePosition.x, tempTilePosition.y, false)) {
					// BaseLocation 에서 ChokePoint 방향에 대해 왼쪽으로 90도 꺾은 방향의 Back Yard : 데카르트 좌표계에서 (cos(t+90) = -sin(t),   sin(t+90) = cos(t))
					bx = tempBaseLocation->getTilePosition().x - (int)(d * std::sin(t) / TILE_SIZE);
					by = tempBaseLocation->getTilePosition().y - (int)(d * std::cos(t) / TILE_SIZE);
					tempTilePosition = BWAPI::TilePosition(bx, by);
					// std::cout << "ConstructionPlaceFinder MainBaseBackYard tempTilePosition " << tempTilePosition.x << "," << tempTilePosition.y << std::endl;

					if (!tempTilePosition.isValid() || !BWAPI::Broodwar->isBuildable(tempTilePosition.x, tempTilePosition.y, false) || tempBaseRegion != BWTA::getRegion(BWAPI::Position(bx*TILE_SIZE, by*TILE_SIZE))) {

						// BaseLocation 에서 ChokePoint 방향 절반 지점의 Back Yard : 데카르트 좌표계에서 (cos(t),   sin(t))
						bx = tempBaseLocation->getTilePosition().x + (int)(d * std::cos(t) / TILE_SIZE);
						by = tempBaseLocation->getTilePosition().y - (int)(d * std::sin(t) / TILE_SIZE);
						tempTilePosition = BWAPI::TilePosition(bx, by);
						// std::cout << "ConstructionPlaceFinder MainBaseBackYard tempTilePosition " << tempTilePosition.x << "," << tempTilePosition.y << std::endl;
						//std::cout << "m";
					}

				}
			}

			//std::cout << "z";
			if (tempTilePosition.isValid() == false 
				|| BWAPI::Broodwar->isBuildable(tempTilePosition.x, tempTilePosition.y, false) == false ) {
				seedPosition = BWAPI::Position(tempTilePosition);
			}
			else {
				seedPosition = tempBaseLocation->getPosition();
			}
		}

		//std::cout << "w";
		// std::cout << "ConstructionPlaceFinder MainBaseBackYard desiredPosition " << desiredPosition.x << "," << desiredPosition.y << std::endl;
		break;

	case BuildOrderItem::SeedPositionStrategy::FirstExpansionLocation:
		tempBaseLocation = InformationManager::Instance().getFirstExpansionLocation(BWAPI::Broodwar->self());
		if (tempBaseLocation) {
			seedPosition = tempBaseLocation->getPosition();
		}
		break;

	case BuildOrderItem::SeedPositionStrategy::FirstChokePoint:
		tempChokePoint = InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->self());
		if (tempChokePoint) {
			seedPosition = tempChokePoint->getCenter();
		}
		break;

	case BuildOrderItem::SeedPositionStrategy::SecondChokePoint:
		tempChokePoint = InformationManager::Instance().getSecondChokePoint(BWAPI::Broodwar->self());
		if (tempChokePoint) {
			seedPosition = tempChokePoint->getCenter();
		}
		break;
	}

	return seedPosition;

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

bool BuildManager::isProducerWillExist(BWAPI::UnitType producerType)
{
	bool isProducerWillExist = true;

	if (BWAPI::Broodwar->self()->completedUnitCount(producerType) == 0
		&& BWAPI::Broodwar->self()->incompleteUnitCount(producerType) == 0)
	{
		// producer 가 건물 인 경우 : 건물이 건설 중인지 추가 파악
		// 만들려는 unitType = Addon 건물. Lair. Hive. Greater Spire. Sunken Colony. Spore Colony. 프로토스 및 테란의 지상유닛 / 공중유닛. 
		if (producerType.isBuilding()) {
			if (ConstructionManager::Instance().getConstructionQueueItemCount(producerType) == 0) {
				isProducerWillExist = false;
			}
		}
		// producer 가 건물이 아닌 경우 : producer 가 생성될 예정인지 추가 파악
		// producerType : 일꾼. Larva. Hydralisk, Mutalisk 
		else {
			// Larva 는 시간이 지나면 Hatchery, Lair, Hive 로부터 생성되기 때문에 해당 건물이 있는지 추가 파악
			if (producerType == BWAPI::UnitTypes::Zerg_Larva)
			{
				if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hatchery) == 0
					&& BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Zerg_Hatchery) == 0
					&& BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 0
					&& BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 0
					&& BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) == 0
					&& BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Zerg_Hive) == 0)
				{
					if (ConstructionManager::Instance().getConstructionQueueItemCount(BWAPI::UnitTypes::Zerg_Hatchery) == 0
						&& ConstructionManager::Instance().getConstructionQueueItemCount(BWAPI::UnitTypes::Zerg_Lair) == 0
						&& ConstructionManager::Instance().getConstructionQueueItemCount(BWAPI::UnitTypes::Zerg_Hive) == 0)
					{
						isProducerWillExist = false;
					}
				}
			}
			// Hydralisk, Mutalisk 는 Egg 로부터 생성되기 때문에 추가 파악
			else if (producerType.getRace() == BWAPI::Races::Zerg) {
				bool isInEgg = false;
				for (auto & unit : BWAPI::Broodwar->self()->getUnits())
				{
					if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg && unit->getBuildType() == producerType) {
						isInEgg = true;
					}
					// 갓태어난 유닛은 아직 반영안되어있을 수 있어서, 추가 카운트를 해줘야함 
					if (unit->getType() == producerType && unit->isConstructing()) {
						isInEgg = true;
					}
				}
				if (isInEgg == false) {
					isProducerWillExist = false;
				}
			}
			else {
				isProducerWillExist = false;
			}
		}
	}

	return isProducerWillExist;
}

void BuildManager::checkBuildOrderQueueDeadlockAndAndFixIt()
{
	// 빌드오더를 수정할 수 있는 프레임인지 먼저 판단한다
	// this will be true if any unit is on the first frame if it's training time remaining
	// this can cause issues for the build order search system so don't plan a search on these frames
	bool canPlanBuildOrderNow = true;
	for (const auto & unit : BWAPI::Broodwar->self()->getUnits()) {
		if (unit->getRemainingTrainTime() == 0) {
			continue;
		}

		//BWAPI::UnitType trainType = unit->getLastCommand().getUnitType();
		BWAPI::UnitCommand unitCommand = unit->getLastCommand();
		BWAPI::UnitCommandType unitCommandType = unitCommand.getType();
		if (unitCommandType != BWAPI::UnitCommandTypes::None) {
			if (unitCommand.getUnit() != nullptr) {
				BWAPI::UnitType trainType = unitCommand.getUnit()->getType();

				if (unit->getRemainingTrainTime() == trainType.buildTime()) {
					canPlanBuildOrderNow = false;
					break;
				}
			}
		}
	}
	if (!canPlanBuildOrderNow) {
		return;
	}

	// BuildQueue 의 HighestPriority 에 있는 BuildQueueItem 이 skip 불가능한 것인데, 선행조건이 충족될 수 없거나, 실행이 앞으로도 계속 불가능한 경우, dead lock 이 발생한다
	// 선행 건물을 BuildQueue에 추가해넣을지, 해당 BuildQueueItem 을 삭제할지 전략적으로 판단해야 한다
	BuildOrderQueue * buildQueue = BuildManager::Instance().getBuildQueue();
	if (!buildQueue->isEmpty())
	{
		BuildOrderItem currentItem = buildQueue->getHighestPriorityItem();

		//if (buildQueue->canSkipCurrentItem() == false)
		if (currentItem.blocking == true)
		{
			bool isDeadlockCase = false;

			// producerType을 먼저 알아낸다
			BWAPI::UnitType producerType = currentItem.metaType.whatBuilds();

			// 건물이나 유닛의 경우
			if (currentItem.metaType.isUnit())
			{
				BWAPI::UnitType unitType = currentItem.metaType.getUnitType();
				BWAPI::TechType requiredTechType = unitType.requiredTech();
				const std::map< BWAPI::UnitType, int >& requiredUnits = unitType.requiredUnits();
				int requiredSupply = unitType.supplyRequired();

				/*
				std::cout << "To make " << unitType.getName()
				<< ", producerType " << producerType.getName()
				<< " completedUnitCount " << BWAPI::Broodwar->self()->completedUnitCount(producerType)
				<< " incompleteUnitCount " << BWAPI::Broodwar->self()->incompleteUnitCount(producerType)
				<< std::endl;
				*/

				// 건물을 생산하는 유닛이나, 유닛을 생산하는 건물이 존재하지 않고, 훈련/건설 예정이지도 않으면 dead lock
				if (isProducerWillExist(producerType) == false) {
					isDeadlockCase = true;
				}
				
				// Refinery 건물의 경우, Refinery 가 건설되지 않은 Geyser가 있는 경우에만 가능
				if (!isDeadlockCase && unitType.isRefinery())
				{
					bool hasAvailableGeyser = true;

					// Refinery가 지어질 수 있는 장소를 찾아본다
					BWAPI::TilePosition testLocation = getDesiredPosition(unitType, currentItem.seedLocation, currentItem.seedLocationStrategy);
										
					// Refinery 를 지으려는 장소를 찾을 수 없으면 dead lock
					if (testLocation == BWAPI::TilePositions::None || testLocation == BWAPI::TilePositions::Invalid || testLocation.isValid() == false) {
						std::cout << "Build Order Dead lock case -> Cann't find place to construct " << unitType.getName() << std::endl;
						hasAvailableGeyser = false;
					}
					else {
						// Refinery 를 지으려는 장소에 Refinery 가 이미 건설되어 있다면 dead lock 
						BWAPI::Unitset uot = BWAPI::Broodwar->getUnitsOnTile(testLocation);
						for (auto & u : uot) {
							if (u->getType().isRefinery() && u->exists()) {
								hasAvailableGeyser = false;

								// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
								// 콘솔 출력 추가. 하지 않아도 됨

								std::cout << "Build Order Dead lock case -> Refinery Building was built already at " << testLocation.x << ", " << testLocation.y << std::endl;

								// BasicBot 1.1 Patch End ////////////////////////////////////////////////

								break;
							}
						}
					}

					if (hasAvailableGeyser == false) {
						isDeadlockCase = true;
					}
				}
				
				// 선행 기술 리서치가 되어있지 않고, 리서치 중이지도 않으면 dead lock
				if (!isDeadlockCase && requiredTechType != BWAPI::TechTypes::None)
				{
					if (BWAPI::Broodwar->self()->hasResearched(requiredTechType) == false) {
						if (BWAPI::Broodwar->self()->isResearching(requiredTechType) == false) {
							isDeadlockCase = true;
						}
					}
				}

				// 선행 건물/유닛이 있는데 
				if (!isDeadlockCase && requiredUnits.size() > 0)
				{
					for (auto & u : requiredUnits)
					{
						BWAPI::UnitType requiredUnitType = u.first;

						if (requiredUnitType != BWAPI::UnitTypes::None) {

							/*
							std::cout << "pre requiredUnitType " << requiredUnitType.getName()
							<< " completedUnitCount " << BWAPI::Broodwar->self()->completedUnitCount(requiredUnitType)
							<< " incompleteUnitCount " << BWAPI::Broodwar->self()->incompleteUnitCount(requiredUnitType)
							<< std::endl;
							*/

							// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
							// Zerg_Mutalisk 나 Zerg_Scourge 를 만들려고하는데 Zerg_Greater_Spire 만 있는 경우 deadlock 으로 판정하는 버그 수정

							// 만들려는 유닛이 Zerg_Mutalisk 이거나 Zerg_Scourge 이고, 선행 유닛이 Zerg_Spire 인 경우, Zerg_Greater_Spire 가 있으면 dead lock 이 아니다
							if ((unitType == BWAPI::UnitTypes::Zerg_Mutalisk || unitType == BWAPI::UnitTypes::Zerg_Scourge)
								&& requiredUnitType == BWAPI::UnitTypes::Zerg_Spire
								&& BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Greater_Spire) > 0)
							{
								isDeadlockCase = false;
							}
							else

							// BasicBot 1.1 Patch End //////////////////////////////////////////////////

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
								// 선행 유닛이 Larva 인 Zerg 유닛의 경우, Larva, Hatchery, Lair, Hive 가 하나도 존재하지 않고, 건설 예정이지 않은 경우에 dead lock
								else if (requiredUnitType == BWAPI::UnitTypes::Zerg_Larva)
								{
									if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hatchery) == 0
										&& BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Zerg_Hatchery) == 0
										&& BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 0
										&& BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 0
										&& BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) == 0
										&& BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Zerg_Hive) == 0)
									{
										if (ConstructionManager::Instance().getConstructionQueueItemCount(BWAPI::UnitTypes::Zerg_Hatchery) == 0
											&& ConstructionManager::Instance().getConstructionQueueItemCount(BWAPI::UnitTypes::Zerg_Lair) == 0
											&& ConstructionManager::Instance().getConstructionQueueItemCount(BWAPI::UnitTypes::Zerg_Hive) == 0)
										{
											isDeadlockCase = true;
										}
									}
								}
							}
						}
					}
				}

				// 건물이 아닌 지상/공중 유닛인 경우, 서플라이가 400 꽉 찼으면 dead lock
				if (!isDeadlockCase && !unitType.isBuilding()
					&& BWAPI::Broodwar->self()->supplyTotal() == 400 && BWAPI::Broodwar->self()->supplyUsed() + unitType.supplyRequired() > 400)
				{
					isDeadlockCase = true;
				}

				// 건물이 아닌 지상/공중 유닛인 경우, 서플라이가 부족하면 dead lock 이지만, 서플라이 부족하다고 지상/공중유닛 빌드를 취소하기보다는 빨리 서플라이를 짓도록 하기 위해, 
				// 이것은 StrategyManager 등에서 따로 처리하도록 한다 

				// Pylon 이 필요한 건물인 경우, 해당 지역 주위에 먼저 지어져야 하는데, Pylon 이 해당 지역 주위에 없으면 dead lock
				// Pylon 범위를 정확하게 파악하거나  만들어질 예정인것, 건설진행중인 것은 일단 체크하지 않는다
				if (!isDeadlockCase && unitType.isBuilding() && unitType.requiresPsi() 
					&& currentItem.seedLocationStrategy == BuildOrderItem::SeedPositionStrategy::SeedPositionSpecified)
				{
					bool hasFoundPylon = false;
					BWAPI::Unitset ourUnits = BWAPI::Broodwar->getUnitsInRadius(BWAPI::Position(currentItem.seedLocation), 4 * TILE_SIZE);

					for (auto & u : ourUnits) {
						if (u->getPlayer() == BWAPI::Broodwar->self() 
							&& u->getType() == BWAPI::UnitTypes::Protoss_Pylon) {
							hasFoundPylon = true;
						}
					}

					if (hasFoundPylon == false) {
						isDeadlockCase = true;
					}
				}

				// Creep 이 필요한 건물인 경우, 해당 지역 주위에 Hatchery나 Creep Colony 등이 있어야 하는데, 해당 지역 주위에 없으면 dead lock
				// Creep 범위를 정확하게 파악하거나  만들어질 예정인것, 건설진행중인 것은 일단 체크하지 않는다
				if (!isDeadlockCase && unitType.isBuilding() && unitType.requiresCreep()
					&& currentItem.seedLocationStrategy == BuildOrderItem::SeedPositionStrategy::SeedPositionSpecified)
				{
					bool hasFoundCreepGenerator = false;
					BWAPI::Unitset ourUnits = BWAPI::Broodwar->getUnitsInRadius(BWAPI::Position(currentItem.seedLocation), 4 * TILE_SIZE);

					for (auto & u : ourUnits) {
						if (u->getPlayer() == BWAPI::Broodwar->self()
							&& (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery || u->getType() == BWAPI::UnitTypes::Zerg_Lair || u->getType() == BWAPI::UnitTypes::Zerg_Hive
								|| u->getType() == BWAPI::UnitTypes::Zerg_Creep_Colony || u->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony || u->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony) )
						{
							hasFoundCreepGenerator = true;
						}
					}

					if (hasFoundCreepGenerator == false) {
						isDeadlockCase = true;
					}
				}

			}
			// 테크의 경우, 해당 리서치를 이미 했거나, 이미 하고있거나, 리서치를 하는 건물 및 선행건물이 존재하지않고 건설예정이지도 않으면 dead lock
			else if (currentItem.metaType.isTech())
			{
				BWAPI::TechType techType = currentItem.metaType.getTechType();
				BWAPI::UnitType requiredUnitType = techType.requiredUnit();

				/*
				std::cout << "To research " << techType.getName()
				<< ", hasResearched " << BWAPI::Broodwar->self()->hasResearched(techType)
				<< ", isResearching " << BWAPI::Broodwar->self()->isResearching(techType)
				<< ", producerType " << producerType.getName()
				<< " completedUnitCount " << BWAPI::Broodwar->self()->completedUnitCount(producerType)
				<< " incompleteUnitCount " << BWAPI::Broodwar->self()->incompleteUnitCount(producerType)
				<< std::endl;
				*/

				if (BWAPI::Broodwar->self()->hasResearched(techType) || BWAPI::Broodwar->self()->isResearching(techType)) {
					isDeadlockCase = true;
				}
				else if (BWAPI::Broodwar->self()->completedUnitCount(producerType) == 0
					&& BWAPI::Broodwar->self()->incompleteUnitCount(producerType) == 0)
				{
					if (ConstructionManager::Instance().getConstructionQueueItemCount(producerType) == 0) {

						// 테크 리서치의 producerType이 Addon 건물인 경우, Addon 건물 건설이 명령 내려졌지만 시작되기 직전에는 getUnits, completedUnitCount, incompleteUnitCount 에서 확인할 수 없다
						// producerType의 producerType 건물에 의해 Addon 건물 건설의 명령이 들어갔는지까지 확인해야 한다
						if (producerType.isAddon()) {

							bool isAddonConstructing = false;

							BWAPI::UnitType producerTypeOfProducerType = producerType.whatBuilds().first;

							if (producerTypeOfProducerType != BWAPI::UnitTypes::None) {

								for (auto & unit : BWAPI::Broodwar->self()->getUnits())
								{
									if (unit == nullptr) continue;
									if (unit->getType() != producerTypeOfProducerType)	{ continue; }

									// 모건물이 완성되어있고, 모건물이 해당 Addon 건물을 건설중인지 확인한다
									if (unit->isCompleted() && unit->isConstructing() && unit->getBuildType() == producerType) {
										isAddonConstructing = true;
										break;
									}
								}
							}

							if (isAddonConstructing == false) {
								isDeadlockCase = true;
							}
						}
						else {
							isDeadlockCase = true;
						}
					}
				}
				else if (requiredUnitType != BWAPI::UnitTypes::None) {
					/*
					std::cout << "To research " << techType.getName()
					<< ", requiredUnitType " << requiredUnitType.getName()
					<< " completedUnitCount " << BWAPI::Broodwar->self()->completedUnitCount(requiredUnitType)
					<< " incompleteUnitCount " << BWAPI::Broodwar->self()->incompleteUnitCount(requiredUnitType)
					<< std::endl;
					*/

					if (BWAPI::Broodwar->self()->completedUnitCount(requiredUnitType) == 0
						&& BWAPI::Broodwar->self()->incompleteUnitCount(requiredUnitType) == 0) {
						if (ConstructionManager::Instance().getConstructionQueueItemCount(requiredUnitType) == 0) {
							isDeadlockCase = true;
						}
					}
				}
			}
			// 업그레이드의 경우, 해당 업그레이드를 이미 했거나, 이미 하고있거나, 업그레이드를 하는 건물 및 선행건물이 존재하지도 않고 건설예정이지도 않으면 dead lock
			else if (currentItem.metaType.isUpgrade())
			{
				BWAPI::UpgradeType upgradeType = currentItem.metaType.getUpgradeType();
				int maxLevel = BWAPI::Broodwar->self()->getMaxUpgradeLevel(upgradeType);
				int currentLevel = BWAPI::Broodwar->self()->getUpgradeLevel(upgradeType);
				BWAPI::UnitType requiredUnitType = upgradeType.whatsRequired();

				/*
				std::cout << "To upgrade " << upgradeType.getName()
				<< ", maxLevel " << maxLevel
				<< ", currentLevel " << currentLevel
				<< ", isUpgrading " << BWAPI::Broodwar->self()->isUpgrading(upgradeType)
				<< ", producerType " << producerType.getName()
				<< " completedUnitCount " << BWAPI::Broodwar->self()->completedUnitCount(producerType)
				<< " incompleteUnitCount " << BWAPI::Broodwar->self()->incompleteUnitCount(producerType)
				<< ", requiredUnitType " << requiredUnitType.getName()
				<< std::endl;
				*/

				if (currentLevel >= maxLevel || BWAPI::Broodwar->self()->isUpgrading(upgradeType)) {
					isDeadlockCase = true;
				}
				else if (BWAPI::Broodwar->self()->completedUnitCount(producerType) == 0
					&& BWAPI::Broodwar->self()->incompleteUnitCount(producerType) == 0) {
					if (ConstructionManager::Instance().getConstructionQueueItemCount(producerType) == 0) {

						// 업그레이드의 producerType이 Addon 건물인 경우, Addon 건물 건설이 시작되기 직전에는 getUnits, completedUnitCount, incompleteUnitCount 에서 확인할 수 없다
						// producerType의 producerType 건물에 의해 Addon 건물 건설이 시작되었는지까지 확인해야 한다						
						if (producerType.isAddon()) {

							bool isAddonConstructing = false;

							BWAPI::UnitType producerTypeOfProducerType = producerType.whatBuilds().first;

							if (producerTypeOfProducerType != BWAPI::UnitTypes::None) {

								for (auto & unit : BWAPI::Broodwar->self()->getUnits())
								{
									if (unit == nullptr) continue;
									if (unit->getType() != producerTypeOfProducerType)	{ continue; }
									// 모건물이 완성되어있고, 모건물이 해당 Addon 건물을 건설중인지 확인한다
									if (unit->isCompleted() && unit->isConstructing() && unit->getBuildType() == producerType) {
										isAddonConstructing = true;
										break;
									}
								}
							}

							if (isAddonConstructing == false) {
								isDeadlockCase = true;
							}
						}
						else {
							isDeadlockCase = true;
						}
					}
				}
				else if (requiredUnitType != BWAPI::UnitTypes::None) {
					if (BWAPI::Broodwar->self()->completedUnitCount(requiredUnitType) == 0
						&& BWAPI::Broodwar->self()->incompleteUnitCount(requiredUnitType) == 0) {
						if (ConstructionManager::Instance().getConstructionQueueItemCount(requiredUnitType) == 0) {
							isDeadlockCase = true;
						}
					}
				}
			}

			if (!isDeadlockCase) {
				// producerID 를 지정했는데, 해당 ID 를 가진 유닛이 존재하지 않으면 dead lock
				if (currentItem.producerID != -1) {
					boolean isProducerAlive = false;
					for (auto & unit : BWAPI::Broodwar->self()->getUnits()) {
						if (unit != nullptr && unit->getID() == currentItem.producerID && unit->exists() && unit->getHitPoints() > 0) {
							isProducerAlive = true;
							break;
						}
					}
					if (isProducerAlive == false) {
						isDeadlockCase = true;
					}
				}
			}


			if (isDeadlockCase) {
				std::cout << std::endl << "Build Order Dead lock case -> remove BuildOrderItem " << currentItem.metaType.getName() << std::endl;

				buildQueue->removeCurrentItem();
			}

		}
	}

}



