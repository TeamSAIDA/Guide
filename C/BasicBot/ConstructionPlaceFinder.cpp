#include "ConstructionPlaceFinder.h"

using namespace MyBot;

ConstructionPlaceFinder::ConstructionPlaceFinder()
{
	_reserveMap = std::vector< std::vector<bool> >(BWAPI::Broodwar->mapWidth(), std::vector<bool>(BWAPI::Broodwar->mapHeight(), false));

	_tilesToAvoid = std::set< BWAPI::TilePosition >();

	setTilesToAvoid();
}

ConstructionPlaceFinder & ConstructionPlaceFinder::Instance() 
{
    static ConstructionPlaceFinder instance;
    return instance;
}

BWAPI::TilePosition	ConstructionPlaceFinder::getBuildLocationWithSeedPositionAndStrategy(BWAPI::UnitType buildingType, BWAPI::TilePosition seedPosition, BuildOrderItem::SeedPositionStrategy seedPositionStrategy) const
{
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 빌드 실행 유닛 (일꾼/건물) 결정 로직이 seedLocation 이나 seedLocationStrategy 를 잘 반영하도록 수정

	BWAPI::TilePosition desiredPosition = BWAPI::TilePositions::None;

	// seedPosition 을 입력한 경우 그 근처에서 찾는다
	if (seedPosition != BWAPI::TilePositions::None  && seedPosition.isValid() )
	{
		//std::cout << "getBuildLocationNear " << seedPosition.x << ", " << seedPosition.y << std::endl;
		desiredPosition = getBuildLocationNear(buildingType, seedPosition);
	}
	// seedPosition 을 입력하지 않은 경우
	else {

		BWTA::Chokepoint* tempChokePoint;
		BWTA::BaseLocation* tempBaseLocation;
		BWAPI::TilePosition tempTilePosition;
		BWTA::Region* tempBaseRegion;
		int vx, vy;
		double d, t;
		int bx, by;


		switch (seedPositionStrategy) {

		case BuildOrderItem::SeedPositionStrategy::MainBaseLocation:
			desiredPosition = getBuildLocationNear(buildingType, InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getTilePosition());
			break;

		case BuildOrderItem::SeedPositionStrategy::MainBaseBackYard:
			tempBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self());
			tempChokePoint = InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->self());
			tempBaseRegion = BWTA::getRegion(tempBaseLocation->getPosition());

			//std::cout << "y";

			// (vx, vy) = BaseLocation 와 ChokePoint 간 차이 벡터 = 거리 d 와 각도 t 벡터. 단위는 position
			// 스타크래프트 좌표계 : 오른쪽으로 갈수록 x 가 증가 (데카르트 좌표계와 동일). 아래로 갈수록 y가 증가 (y축만 데카르트 좌표계와 반대)
			// 삼각함수 값은 데카르트 좌표계에서 계산하므로, vy를 부호 반대로 해서 각도 t 값을 구함 

			// FirstChokePoint 가 null 이면, MainBaseLocation 주위에서 가능한 곳을 리턴한다
			if (tempChokePoint == nullptr) {
				//std::cout << "r";
				desiredPosition = getBuildLocationNear(buildingType, InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getTilePosition());
				break;
			}

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
			if (!tempTilePosition.isValid() || !BWAPI::Broodwar->isBuildable(tempTilePosition.x, tempTilePosition.y, false)) {
				desiredPosition = getBuildLocationNear(buildingType, tempBaseLocation->getTilePosition());
			}
			else {
				desiredPosition = getBuildLocationNear(buildingType, tempTilePosition);
			}
			//std::cout << "w";
			// std::cout << "ConstructionPlaceFinder MainBaseBackYard desiredPosition " << desiredPosition.x << "," << desiredPosition.y << std::endl;
			break;

		case BuildOrderItem::SeedPositionStrategy::FirstExpansionLocation:
			tempBaseLocation = InformationManager::Instance().getFirstExpansionLocation(BWAPI::Broodwar->self());
			if (tempBaseLocation) {
				desiredPosition = getBuildLocationNear(buildingType, tempBaseLocation->getTilePosition());
			}
			break;

		case BuildOrderItem::SeedPositionStrategy::FirstChokePoint:
			tempChokePoint = InformationManager::Instance().getFirstChokePoint(BWAPI::Broodwar->self());
			if (tempChokePoint) {
				desiredPosition = getBuildLocationNear(buildingType, BWAPI::TilePosition(tempChokePoint->getCenter()));
			}
			break;

		case BuildOrderItem::SeedPositionStrategy::SecondChokePoint:
			tempChokePoint = InformationManager::Instance().getSecondChokePoint(BWAPI::Broodwar->self());
			if (tempChokePoint) {
				desiredPosition = getBuildLocationNear(buildingType, BWAPI::TilePosition(tempChokePoint->getCenter()));
			}
			break;

		}
	}

	return desiredPosition;

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

BWAPI::TilePosition	ConstructionPlaceFinder::getBuildLocationNear(BWAPI::UnitType buildingType, BWAPI::TilePosition desiredPosition) const
{
	if (buildingType.isRefinery())
	{
		//std::cout << "getRefineryPositionNear "<< std::endl;

		return getRefineryPositionNear(desiredPosition);
	}

	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss) {
		// special easy case of having no pylons
		if (buildingType.requiresPsi() && BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Pylon) == 0)
		{
			return BWAPI::TilePositions::None;
		}
	}

	if (desiredPosition == BWAPI::TilePositions::None || desiredPosition == BWAPI::TilePositions::Unknown || desiredPosition == BWAPI::TilePositions::Invalid || desiredPosition.isValid() == false)
	{
		desiredPosition = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getTilePosition();
	}

	BWAPI::TilePosition testPosition = BWAPI::TilePositions::None;

	// TODO 과제 : 건설 위치 탐색 방법은 ConstructionPlaceSearchMethod::SpiralMethod 로 하는데, 더 좋은 방법은 생각해볼 과제이다
	size_t constructionPlaceSearchMethod = ConstructionPlaceSearchMethod::SpiralMethod;
	
	// 일반적인 건물에 대해서는 건물 크기보다 Config::Macro::BuildingSpacing 칸 만큼 상하좌우로 더 넓게 여유공간을 두어서 빈 자리를 검색한다
	int buildingGapSpace = Config::Macro::BuildingSpacing;

	// ResourceDepot (Nexus, Command Center, Hatchery),
	// Protoss_Pylon, Terran_Supply_Depot, 
	// Protoss_Photon_Cannon, Terran_Bunker, Terran_Missile_Turret, Zerg_Creep_Colony 는 다른 건물 바로 옆에 붙여 짓는 경우가 많으므로 
	// buildingGapSpace을 다른 Config 값으로 설정하도록 한다
	if (buildingType.isResourceDepot()) {
		buildingGapSpace = Config::Macro::BuildingResourceDepotSpacing;		
	}
	else if (buildingType == BWAPI::UnitTypes::Protoss_Pylon) {
		int numPylons = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Pylon);
		
		// Protoss_Pylon 은 특히 최초 2개 건설할때는 Config::Macro::BuildingPylonEarlyStageSpacing 값으로 설정한다
		if (numPylons < 3) {
			buildingGapSpace = Config::Macro::BuildingPylonEarlyStageSpacing;
		}
		else {
			buildingGapSpace = Config::Macro::BuildingPylonSpacing;
		}
	}
	else if (buildingType == BWAPI::UnitTypes::Terran_Supply_Depot) {
		buildingGapSpace = Config::Macro::BuildingSupplyDepotSpacing;
	}
	else if (buildingType == BWAPI::UnitTypes::Protoss_Photon_Cannon || buildingType == BWAPI::UnitTypes::Terran_Bunker 
		|| buildingType == BWAPI::UnitTypes::Terran_Missile_Turret || buildingType == BWAPI::UnitTypes::Zerg_Creep_Colony) {
		buildingGapSpace = Config::Macro::BuildingDefenseTowerSpacing;
	}

	while (buildingGapSpace >= 0) {

		testPosition = getBuildLocationNear(buildingType, desiredPosition, buildingGapSpace, constructionPlaceSearchMethod);

		// std::cout << "ConstructionPlaceFinder testPosition " << testPosition.x << "," << testPosition.y << std::endl;

		if (testPosition != BWAPI::TilePositions::None && testPosition != BWAPI::TilePositions::Invalid)
			return testPosition;
				
		// 찾을 수 없다면, buildingGapSpace 값을 줄여서 다시 탐색한다
		// buildingGapSpace 값이 1이면 지상유닛이 못지나가는 경우가 많아  제외하도록 한다 
		// 4 -> 3 -> 2 -> 0 -> 탐색 종료
		//      3 -> 2 -> 0 -> 탐색 종료 
		//           1 -> 0 -> 탐색 종료
		if (buildingGapSpace > 2) {
			buildingGapSpace -= 1;
		}
		else if (buildingGapSpace == 2){
			buildingGapSpace = 0;
		}
		else if (buildingGapSpace == 1){
			buildingGapSpace = 0;
		}
		else {
			break;
		}
	}

	return BWAPI::TilePositions::None;
}

BWAPI::TilePosition	ConstructionPlaceFinder::getBuildLocationNear(BWAPI::UnitType buildingType, BWAPI::TilePosition desiredPosition, int buildingGapSpace, size_t constructionPlaceSearchMethod) const
{
	// std::cout << std::endl << "getBuildLocationNear " << buildingType.getName().c_str() << " " << desiredPosition.x << "," << desiredPosition.y 
	//	<< " gap " << buildingGapSpace << " method " << constructionPlaceSearchMethod << std::endl;

	//returns a valid build location near the desired tile position (x,y).
	BWAPI::TilePosition resultPosition = BWAPI::TilePositions::None;
	BWAPI::TilePosition tempPosition;
	ConstructionTask b(buildingType, desiredPosition);

	// maxRange 를 설정하지 않거나, maxRange 를 128으로 설정하면 지도 전체를 다 탐색하는데, 매우 느려질뿐만 아니라, 대부분의 경우 불필요한 탐색이 된다
	// maxRange 는 16 ~ 64가 적당하다
	int maxRange = 32; // maxRange = BWAPI::Broodwar->mapWidth()/4;
	bool isPossiblePlace = false;
		
	if (constructionPlaceSearchMethod == ConstructionPlaceSearchMethod::SpiralMethod)
	{
		// desiredPosition 으로부터 시작해서 spiral 하게 탐색하는 방법
		// 처음에는 아래 방향 (0,1) -> 오른쪽으로(1,0) -> 위로(0,-1) -> 왼쪽으로(-1,0) -> 아래로(0,1) -> ..
		int currentX = desiredPosition.x;
		int currentY = desiredPosition.y;
		int spiralMaxLength = 1;
		int numSteps = 0;
		boolean isFirstStep = true;

		int spiralDirectionX = 0;
		int spiralDirectionY = 1;
		while (spiralMaxLength < maxRange)
		{
			if (currentX >= 0 && currentX < BWAPI::Broodwar->mapWidth() && currentY >= 0 && currentY <BWAPI::Broodwar->mapHeight()) {

				isPossiblePlace = canBuildHereWithSpace(BWAPI::TilePosition(currentX, currentY), b, buildingGapSpace);

				if (isPossiblePlace) {
					resultPosition = BWAPI::TilePosition(currentX, currentY);
					break;
				}
			}

			currentX = currentX + spiralDirectionX;
			currentY = currentY + spiralDirectionY;
			numSteps++;

			// 다른 방향으로 전환한다
			if (numSteps == spiralMaxLength)
			{
				numSteps = 0;

				if (!isFirstStep)
					spiralMaxLength++;

				isFirstStep = !isFirstStep;

				if (spiralDirectionX == 0)
				{
					spiralDirectionX = spiralDirectionY;
					spiralDirectionY = 0;
				}
				else
				{
					spiralDirectionY = -spiralDirectionX;
					spiralDirectionX = 0;
				}
			}
		}
	}
	else if (constructionPlaceSearchMethod == ConstructionPlaceSearchMethod::NewMethod) {
	}

	return resultPosition;
}


bool ConstructionPlaceFinder::canBuildHereWithSpace(BWAPI::TilePosition position, const ConstructionTask & b, int buildingGapSpace) const
{
	//if we can't build here, we of course can't build here with space
	if (!canBuildHere(position, b))
	{
		return false;
	}

	// height and width of the building
	int width(b.type.tileWidth());
	int height(b.type.tileHeight());

	// define the rectangle of the building spot
	// 건물 크기보다 상하좌우로 더 큰 사각형
	int startx;
	int starty;
	int endx;
	int endy;

	bool horizontalOnly = false;

	// Refinery 의 경우 GapSpace를 체크할 필요 없다
	if (b.type.isRefinery())
	{
	}
	// Addon 타입의 건물일 경우에는, 그 Addon 건물 왼쪽에 whatBuilds 건물이 있는지를 체크한다
	if (b.type.isAddon())
	{
		const BWAPI::UnitType builderType = b.type.whatBuilds().first;

		BWAPI::TilePosition builderTile(position.x - builderType.tileWidth(), position.y + 2 - builderType.tileHeight());

		startx = builderTile.x - buildingGapSpace;
		starty = builderTile.y - buildingGapSpace;
		endx = position.x + width + buildingGapSpace;
		endy = position.y + height + buildingGapSpace;

		// builderTile에 Lifted 건물이 아니고 whatBuilds 건물이 아닌 건물이 있는지 체크
		for (int i = 0; i <= builderType.tileWidth(); ++i)
		{
			for (int j = 0; j <= builderType.tileHeight(); ++j)
			{
				for (auto & unit : BWAPI::Broodwar->getUnitsOnTile(builderTile.x + i, builderTile.y + j))
				{
					if ((unit->getType() != builderType) && (!unit->isLifted()))
					{
						return false;
					}
				}
			}
		}
	}
	else 
	{
		//make sure we leave space for add-ons. These types of units can have addon:
		if (b.type == BWAPI::UnitTypes::Terran_Command_Center ||
			b.type == BWAPI::UnitTypes::Terran_Factory ||
			b.type == BWAPI::UnitTypes::Terran_Starport ||
			b.type == BWAPI::UnitTypes::Terran_Science_Facility)
		{
			width += 2;
		}

		// 상하좌우에 buildingGapSpace 만큼 간격을 띄운다
		if (horizontalOnly == false)
		{
			startx = position.x - buildingGapSpace;
			starty = position.y - buildingGapSpace;
			endx = position.x + width + buildingGapSpace;
			endy = position.y + height + buildingGapSpace;
		}
		// 좌우로만 buildingGapSpace 만큼 간격을 띄운다
		else {
			startx = position.x - buildingGapSpace;
			starty = position.y;
			endx = position.x + width + buildingGapSpace;
			endy = position.y + height;
		}

		// 테란종족 건물의 경우 다른 건물의 Addon 공간을 확보해주기 위해, 왼쪽 2칸은 반드시 GapSpace가 되도록 한다
		if (b.type.getRace() == BWAPI::Races::Terran) {
			if (buildingGapSpace < 2) {
				startx = position.x - 2;
				endx = position.x + width + buildingGapSpace;
			}
		}

		// 건물이 차지할 공간 뿐 아니라 주위의 buildingGapSpace 공간까지 다 비어있는지, 건설가능한 타일인지, 예약되어있는것은 아닌지, TilesToAvoid 에 해당하지 않는지 체크
		for (int x = startx; x < endx; x++)
		{
			for (int y = starty; y < endy; y++)
			{
				// if we can't build here, or space is reserved, we can't build here
				if (isBuildableTile(b, x, y) == false)
				{
					return false;
				}

				if (isReservedTile(x, y)) {
					return false;
				}

				// ResourceDepot / Addon 건물이 아닌 일반 건물의 경우, BaseLocation 과 Geyser 사이 타일 (TilesToAvoid) 에는 건물을 짓지 않는다
				if (b.type.isResourceDepot() == false && b.type.isAddon() == false) {
					if (isTilesToAvoid(x, y)) {
						return false;
					}
				}
			}
		}
	}

	// if this rectangle doesn't fit on the map we can't build here
	if (startx < 0 || starty < 0 || endx > BWAPI::Broodwar->mapWidth() || endx < position.x + width || endy > BWAPI::Broodwar->mapHeight())
	{
		return false;
	}

	return true;
}

bool ConstructionPlaceFinder::canBuildHere(BWAPI::TilePosition position, const ConstructionTask & b) const
{
	/*if (!b.type.isRefinery() && !InformationManager::Instance().tileContainsUnit(position))
	{
	return false;
	}*/

	// This function checks for creep, power, and resource distance requirements in addition to the tiles' buildability and possible units obstructing the build location.
	if (!BWAPI::Broodwar->canBuildHere(position, b.type, b.constructionWorker))
	{
		return false;
	}
	
	// check the reserve map
	for (int x = position.x; x < position.x + b.type.tileWidth(); x++)
	{
		for (int y = position.y; y < position.y + b.type.tileHeight(); y++)
		{
			if (_reserveMap[x][y])
			{
				return false;
			}
		}
	}

	// if it overlaps a base location return false
	// ResourceDepot 건물이 아닌 다른 건물은 BaseLocation 위치에 짓지 못하도록 한다
	if (isOverlapsWithBaseLocation(position, b.type))
	{
		return false;
	}

	return true;
}

BWAPI::TilePosition ConstructionPlaceFinder::getRefineryPositionNear(BWAPI::TilePosition seedPosition) const
{
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// Refinery 건물 건설 위치 탐색 로직 버그 수정 및 속도 개선 : seedPosition 주위에서만 geyser를 찾도록, 이미 Refinery가 지어져있는지 체크하지 않도록 수정

	if (seedPosition == BWAPI::TilePositions::None || seedPosition == BWAPI::TilePositions::Unknown || seedPosition == BWAPI::TilePositions::Invalid || seedPosition.isValid() == false)
	{
		seedPosition = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getTilePosition();
	}
	
	BWAPI::TilePosition closestGeyser = BWAPI::TilePositions::None;
	double minGeyserDistanceFromSeedPosition = 100000000;

	// 전체 geyser 중에서 seedPosition 으로부터 16 TILE_SIZE 거리 이내에 있는 것을 찾는다
	for (auto & geyser : BWAPI::Broodwar->getStaticGeysers())
	{
		// geyser->getPosition() 을 하면, Unknown 으로 나올 수 있다.
		// 반드시 geyser->getInitialPosition() 을 사용해야 한다
		BWAPI::Position geyserPos = geyser->getInitialPosition();
		BWAPI::TilePosition geyserTilePos = geyser->getInitialTilePosition();

		// 이미 예약되어있는가
		if (isReservedTile(geyserTilePos.x, geyserTilePos.y)) {
			continue;
		}

		// geyser->getType() 을 하면, Unknown 이거나, Resource_Vespene_Geyser 이거나, Terran_Refinery 와 같이 건물명이 나오고, 
		// 건물이 파괴되어도 자동으로 Resource_Vespene_Geyser 로 돌아가지 않는다
		// geyser 위치에 있는 유닛들에 대해 isRefinery() 로 체크를 해봐야 한다

		// seedPosition 으로부터 16 TILE_SIZE 거리 이내에 있는가
		// Fighting Spirit 맵처럼 seedPosition 으로부터 동일한 거리 내에 geyser 가 여러개 있을 수 있는 경우 Refinery 건물을 짓기 위해서는 seedPosition 을 정확하게 입력해야 한다
		double thisDistance = geyserTilePos.getDistance(seedPosition);
		if (thisDistance <= 16 && thisDistance < minGeyserDistanceFromSeedPosition)
		{
			minGeyserDistanceFromSeedPosition = thisDistance;
			closestGeyser = geyser->getInitialTilePosition();
			break;
		}
	}

	return closestGeyser;

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

// 해당 tile 이 BaseLocation (ResourceDepot 건물을 지을 장소) 과 겹치는지 체크한다
bool ConstructionPlaceFinder::isOverlapsWithBaseLocation(BWAPI::TilePosition tile, BWAPI::UnitType type) const
{
	// if it's a resource depot we don't care if it overlaps
	if (type.isResourceDepot())
	{
		return false;
	}

	// dimensions of the proposed location
	int tx1 = tile.x;
	int ty1 = tile.y;
	int tx2 = tx1 + type.tileWidth();
	int ty2 = ty1 + type.tileHeight();

	// for each base location
	for (BWTA::BaseLocation * base : BWTA::getBaseLocations())
	{
		// dimensions of the base location
		int bx1 = base->getTilePosition().x;
		int by1 = base->getTilePosition().y;
		int bx2 = bx1 + InformationManager::Instance().getBasicResourceDepotBuildingType().tileWidth();
		int by2 = by1 + InformationManager::Instance().getBasicResourceDepotBuildingType().tileHeight();

		// conditions for non-overlap are easy
		bool noOverlap = (tx2 < bx1) || (tx1 > bx2) || (ty2 < by1) || (ty1 > by2);

		// if the reverse is true, return true
		if (!noOverlap)
		{
			return true;
		}
	}

	// otherwise there is no overlap
	return false;
}

//returns true if this tile is currently isBuildableTile, takes into account units on tile
// Broodwar 의 isBuildable, 기존 빌딩들, 다른 유닛들을 체크
bool ConstructionPlaceFinder::isBuildableTile(const ConstructionTask & b, int x, int y) const
{
	BWAPI::TilePosition tp(x, y);
	if (!tp.isValid())
	{
		return false;
	}

	// 맵 데이터 뿐만 아니라 빌딩 데이터를 모두 고려해서 isBuildable 체크
	//if (BWAPI::Broodwar->isBuildable(x, y) == false)
	if (BWAPI::Broodwar->isBuildable(x, y, true) == false)
	{
		return false;
	}

	// constructionWorker 이외의 다른 유닛이 있으면 false를 리턴한다
	for (auto & unit : BWAPI::Broodwar->getUnitsOnTile(x, y))
	{
		if ((b.constructionWorker != nullptr) && (unit != b.constructionWorker))
		{
			return false;
		}
	}

	return true;
}

void ConstructionPlaceFinder::reserveTiles(BWAPI::TilePosition position, int width, int height)
{
	int rwidth = _reserveMap.size();
	int rheight = _reserveMap[0].size();
	for (int x = position.x; x < position.x + width && x < rwidth; x++)
	{
		for (int y = position.y; y < position.y + height && y < rheight; y++)
		{
			_reserveMap[x][y] = true;
		}
	}
}

void ConstructionPlaceFinder::freeTiles(BWAPI::TilePosition position, int width, int height)
{
	int rwidth = _reserveMap.size();
	int rheight = _reserveMap[0].size();

	for (int x = position.x; x < position.x + width && x < rwidth; x++)
	{
		for (int y = position.y; y < position.y + height && y < rheight; y++)
		{
			_reserveMap[x][y] = false;
		}
	}
}

// 건물 건설 예약되어있는 타일인지 체크
bool ConstructionPlaceFinder::isReservedTile(int x, int y) const
{
	int rwidth = _reserveMap.size();
	int rheight = _reserveMap[0].size();
	if (x < 0 || y < 0 || x >= rwidth || y >= rheight)
	{
		return false;
	}

	return _reserveMap[x][y];
}


std::vector< std::vector<bool> > & ConstructionPlaceFinder::getReserveMap() {
	return _reserveMap;
}

bool ConstructionPlaceFinder::isTilesToAvoid(int x, int y) const
{
	for (auto & t : _tilesToAvoid) {
		if (t.x == x && t.y == y) {
			return true;
		}
	}

	return false;
}


void ConstructionPlaceFinder::setTilesToAvoid()
{
	// ResourceDepot 건물의 width = 4 타일, height = 3 타일
	// Geyser 의            width = 4 타일, height = 2 타일
	// Mineral 의           width = 2 타일, height = 1 타일

	for (BWTA::BaseLocation * base : BWTA::getBaseLocations())
	{
		// Island 일 경우 건물 지을 공간이 절대적으로 좁기 때문에 건물 안짓는 공간을 두지 않는다
		if (base->isIsland()) continue;
		if (BWTA::isConnected(base->getTilePosition(), InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getTilePosition()) == false) continue;

		// dimensions of the base location
		int bx0 = base->getTilePosition().x;
		int by0 = base->getTilePosition().y;
		int bx4 = base->getTilePosition().x + 4;
		int by3 = base->getTilePosition().y + 3;

		// BaseLocation 과 Geyser 사이의 타일을 BWTA::getShortestPath 를 사용해서 구한 후 _tilesToAvoid 에 추가
		for (auto & geyser : base->getGeysers())
		{
			BWAPI::TilePosition closeGeyserPosition = geyser->getInitialTilePosition();

			// dimensions of the closest geyser
			int gx0 = closeGeyserPosition.x;
			int gy0 = closeGeyserPosition.y;
			int gx4 = closeGeyserPosition.x + 4;
			int gy2 = closeGeyserPosition.y + 2;

			for (int i = bx0; i < bx4; i++) {
				for (int j = by0; j < by3; j++) {
					for (int k = gx0; k < gx4; k++) {
						for (int l = gy0; l < gy2; l++) {
							std::vector<BWAPI::TilePosition> tileList = BWTA::getShortestPath(BWAPI::TilePosition(i, j), BWAPI::TilePosition(k, l));
							for (auto & t : tileList) {
								_tilesToAvoid.insert(t);
							}
						}
					}
				}
			}

			/*
			// Geyser 가 Base Location 의 어느방향에 있는가에 따라 최소한의 타일만 판단해서 tilesToAvoid 에 추가하는 방법도 있다
			//
			//    11시방향   12시방향  1시방향
			//
			//     9시방향             3시방향
			//
			//     7시방향    6시방향  5시방향
			int whichPosition = 0;

			// dimensions of the tilesToAvoid
			int vx0 = 0;
			int vx1 = 0;
			int vy0 = 0;
			int vy1 = 0;

			// 11시 방향
			if (gx0 < bx0 && gy0 < by0) {
				vx0 = gx0 + 1; // Geyser 의 중앙
				vy0 = gy0;     // Geyser 의 상단
				vx1 = bx0 + 3; // ResourceDepot 의 중앙
				vy1 = by0;     // ResourceDepot의 상단
			}
			// 9시 방향
			else if (gx0 < bx0 && gy0 <= by3) {
				vx0 = gx4; // Geyser 의 오른쪽끝
				vy0 = gy0; // Geyser 의 상단
				vx1 = bx0; // ResourceDepot 의 왼쪽끝
				vy1 = gy2; // Geyser 의 하단 
			}
			// 7시 방향
			else if (gx0 < bx0 && gy2 > by3) {
				vx0 = gx0 + 1; // Geyser 의 상단 중앙
				vy0 = by3;     // ResourceDepot 의 하단
				vx1 = bx0 + 3; // ResourceDepot 의 하단 중앙
				vy1 = gy0;     // Geyser 의 상단
			}
			// 6시 방향
			else if (gx0 < bx4 && gy0 > by3) {
				vx0 = bx0 + 1; // ResourceDepot 의 하단 중앙
				vy0 = by3;     // ResourceDepot 의 하단 
				vx1 = gx0 + 3; // Geyser 의 상단 중앙
				vy1 = gy0;     // Geyser 의 상단
			}
			// 12시 방향
			else if (gx0 < bx4 && gy0 < by0) {
				vx0 = gx0;     // Geyser 의 하단 왼쪽끝
				vy0 = gy2; 
				vx1 = gx0 + 3; // Geyser 의 중앙
				vy1 = by0;     // ResourceDepot 의 상단
			}
			// 1시 방향
			else if (gx0 > bx0 && gy0 < by0) {
				vx0 = bx0 + 2; // ResourceDepot 의 상단 중앙
				vy0 = gy0 + 1; // Geyser 의 하단
				vx1 = gx0 + 2; // Geyser 의 중앙
				vy1 = by0 + 1; // ResourceDepot 의 상단
			}
			// 5시 방향
			else if (gx0 > bx0 && gy0 >= by3) {
				vx0 = bx0 + 2; // ResourceDepot 의 하단 중앙
				vy0 = by0 + 2; // ResourceDepot 의 하단
				vx1 = gx0 + 2; // Geyser 의 중앙
				vy1 = gy0 + 1; // Geyser 의 하단
			}
			// 3시 방향
			else if (gx0 > bx0 && gy0 >= by0) {
				vx0 = bx4; // ResourceDepot 의 오른쪽끝
				vy0 = gy0; // Geyser 의 상단
				vx1 = gx0; // Geyser 의 왼쪽 끝
				vy1 = gy2; // Geyser 의 하단
			}

			for (int i = vx0; i < vx1; i++) {
				for (int j = vy0; j < vy1; j++) {
					_tilesToAvoid.insert(BWAPI::TilePosition(i, j));
				}
			}
			*/

		}

		// BaseLocation 과 Mineral 사이의 타일을 BWTA::getShortestPath 를 사용해서 구한 후 _tilesToAvoid 에 추가
		for (auto & mineral : base->getMinerals())
		{
			BWAPI::TilePosition closeMineralPosition = mineral->getInitialTilePosition();

			// dimensions of the closest mineral
			int mx0 = closeMineralPosition.x;
			int my0 = closeMineralPosition.y;
			int mx2 = mx0 + 2;
			int my1 = my0 + 1;

			for (int i = bx0; i < bx4; i++) {
				for (int j = by0; j < by3; j++) {
					for (int k = mx0; k < mx2; k++) {
						std::vector<BWAPI::TilePosition> tileList = BWTA::getShortestPath(BWAPI::TilePosition(i, j), BWAPI::TilePosition(k, my0));
						for (auto & t : tileList) {
							_tilesToAvoid.insert(t);
						}
					}
				}
			}
		}
	}
}

std::set< BWAPI::TilePosition > & ConstructionPlaceFinder::getTilesToAvoid() {
	return _tilesToAvoid;
}
