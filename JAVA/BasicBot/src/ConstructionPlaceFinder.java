import java.util.HashSet;
import java.util.List;
import java.util.Set;

import bwapi.Position;
import bwapi.Race;
import bwapi.TilePosition;
import bwapi.Unit;
import bwapi.UnitType;
import bwta.BWTA;
import bwta.BaseLocation;
import bwta.Chokepoint;
import bwta.Region;

/// 건설위치 탐색을 위한 class
public class ConstructionPlaceFinder {

	/// 건설위치 탐색 방법
	public enum ConstructionPlaceSearchMethod { 
		SpiralMethod,	///< 나선형으로 돌아가며 탐색 
		NewMethod 		///< 예비
	};
	
	/// 건물 건설 예정 타일을 저장해놓기 위한 2차원 배열<br>
	/// TilePosition 단위이기 때문에 보통 128*128 사이즈가 된다<br>
	/// 참고로, 건물이 이미 지어진 타일은 저장하지 않는다
	private boolean[][] reserveMap = new boolean[128][128];
	
	/// BaseLocation 과 Mineral / Geyser 사이의 타일들을 담는 자료구조. 여기에는 Addon 이외에는 건물을 짓지 않도록 합니다	
	private Set<TilePosition> tilesToAvoid = new HashSet<TilePosition>();
	
	private static ConstructionPlaceFinder instance = new ConstructionPlaceFinder();
	
	private static boolean isInitialized = false;
	
	/// static singleton 객체를 리턴합니다
	public static ConstructionPlaceFinder Instance() {
		if (isInitialized == false) {
			instance.setTilesToAvoid();
			isInitialized = true;
		}
		return instance;
	}

	/// seedPosition 및 seedPositionStrategy 파라메터를 활용해서 건물 건설 가능 위치를 탐색해서 리턴합니다<br>
	/// seedPosition 주위에서 가능한 곳을 선정하거나, seedPositionStrategy 에 따라 지형 분석결과 해당 지점 주위에서 가능한 곳을 선정합니다<br>
	/// seedPosition, seedPositionStrategy 을 입력하지 않으면, MainBaseLocation 주위에서 가능한 곳을 리턴합니다
	public final TilePosition getBuildLocationWithSeedPositionAndStrategy(UnitType buildingType, TilePosition seedPosition, BuildOrderItem.SeedPositionStrategy seedPositionStrategy)
	{
		// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
		// 빌드 실행 유닛 (일꾼/건물) 결정 로직이 seedLocation 이나 seedLocationStrategy 를 잘 반영하도록 수정

		TilePosition desiredPosition = TilePosition.None;

		// seedPosition 을 입력한 경우 그 근처에서 찾는다
		if (seedPosition != TilePosition.None  && seedPosition.isValid() )
		{
			//std::cout << "getBuildLocationNear " << seedPosition.x << ", " << seedPosition.y << std::endl;
			desiredPosition = getBuildLocationNear(buildingType, seedPosition);
		}
		// seedPosition 을 입력하지 않은 경우
		else {
			Chokepoint tempChokePoint;
			BaseLocation tempBaseLocation;
			TilePosition tempTilePosition = null;
			Region tempBaseRegion;
			int vx, vy;
			double d, t;
			int bx, by;

			switch (seedPositionStrategy) {

			case MainBaseLocation:
				desiredPosition = getBuildLocationNear(buildingType, InformationManager.Instance().getMainBaseLocation(MyBotModule.Broodwar.self()).getTilePosition());
				break;

			case MainBaseBackYard:
				tempBaseLocation = InformationManager.Instance().getMainBaseLocation(MyBotModule.Broodwar.self());
				tempChokePoint = InformationManager.Instance().getFirstChokePoint(MyBotModule.Broodwar.self());
				tempBaseRegion = BWTA.getRegion(tempBaseLocation.getPosition());

				//std::cout << "y";

				// (vx, vy) = BaseLocation 와 ChokePoint 간 차이 벡터 = 거리 d 와 각도 t 벡터. 단위는 position
				// 스타크래프트 좌표계 : 오른쪽으로 갈수록 x 가 증가 (데카르트 좌표계와 동일). 아래로 갈수록 y가 증가 (y축만 데카르트 좌표계와 반대)
				// 삼각함수 값은 데카르트 좌표계에서 계산하므로, vy를 부호 반대로 해서 각도 t 값을 구함 

				// FirstChokePoint 가 null 이면, MainBaseLocation 주위에서 가능한 곳을 리턴한다
				if (tempChokePoint == null) {
					//std::cout << "r";
					desiredPosition = getBuildLocationNear(buildingType, InformationManager.Instance().getMainBaseLocation(MyBotModule.Broodwar.self()).getTilePosition());
					break;
				}

				// BaseLocation 에서 ChokePoint 로의 벡터를 구한다
				vx = tempChokePoint.getCenter().getX() - tempBaseLocation.getPosition().getX();
				//std::cout << "vx : " << vx ;
				vy = (tempChokePoint.getCenter().getY() - tempBaseLocation.getPosition().getY()) * (-1);
				//std::cout << "vy : " << vy;
				d = Math.sqrt(vx * vx + vy * vy) * 0.5; // BaseLocation 와 ChokePoint 간 거리보다 조금 짧은 거리로 조정. BaseLocation가 있는 Region은 대부분 직사각형 형태이기 때문
				//std::cout << "d : " << d;
				t = Math.atan2(vy, vx + 0.0001); // 라디안 단위
				//std::cout << "t : " << t;

				// cos(t+90), sin(t+180) 등 삼각함수 Trigonometric functions of allied angles 을 이용. y축에 대해서는 반대부호로 적용

				// BaseLocation 에서 ChokePoint 반대쪽 방향의 Back Yard : 데카르트 좌표계에서 (cos(t+180) = -cos(t), sin(t+180) = -sin(t))
				bx = tempBaseLocation.getTilePosition().getX() - (int)(d * Math.cos(t) / Config.TILE_SIZE);
				by = tempBaseLocation.getTilePosition().getY() + (int)(d * Math.sin(t) / Config.TILE_SIZE);
				//std::cout << "i";
				tempTilePosition = new TilePosition(bx, by);
				// std::cout << "ConstructionPlaceFinder MainBaseBackYard tempTilePosition " << tempTilePosition.x << "," << tempTilePosition.y << std::endl;
				
				//std::cout << "k";
				// 해당 지점이 같은 Region 에 속하고 Buildable 한 타일인지 확인
				if (!tempTilePosition.isValid() || !MyBotModule.Broodwar.isBuildable(tempTilePosition.getX(), tempTilePosition.getY(), false) || tempBaseRegion != BWTA.getRegion(new Position(bx*Config.TILE_SIZE, by*Config.TILE_SIZE))) {
					//std::cout << "l";

					// BaseLocation 에서 ChokePoint 방향에 대해 오른쪽으로 90도 꺾은 방향의 Back Yard : 데카르트 좌표계에서 (cos(t-90) = sin(t),   sin(t-90) = - cos(t))
					bx = tempBaseLocation.getTilePosition().getX() + (int)(d * Math.sin(t) / Config.TILE_SIZE);
					by = tempBaseLocation.getTilePosition().getY() + (int)(d * Math.cos(t) / Config.TILE_SIZE);
					tempTilePosition = new TilePosition(bx, by);
					// std::cout << "ConstructionPlaceFinder MainBaseBackYard tempTilePosition " << tempTilePosition.x << "," << tempTilePosition.y << std::endl;
					//std::cout << "m";

					if (!tempTilePosition.isValid() || !MyBotModule.Broodwar.isBuildable(tempTilePosition.getX(), tempTilePosition.getY(), false)) {
						// BaseLocation 에서 ChokePoint 방향에 대해 왼쪽으로 90도 꺾은 방향의 Back Yard : 데카르트 좌표계에서 (cos(t+90) = -sin(t),   sin(t+90) = cos(t))
						bx = tempBaseLocation.getTilePosition().getX() - (int)(d * Math.sin(t) / Config.TILE_SIZE);
						by = tempBaseLocation.getTilePosition().getY() - (int)(d * Math.cos(t) / Config.TILE_SIZE);
						tempTilePosition = new TilePosition(bx, by);
						// std::cout << "ConstructionPlaceFinder MainBaseBackYard tempTilePosition " << tempTilePosition.x << "," << tempTilePosition.y << std::endl;

						if (!tempTilePosition.isValid() || !MyBotModule.Broodwar.isBuildable(tempTilePosition.getX(), tempTilePosition.getY(), false) || tempBaseRegion != BWTA.getRegion(new Position(bx*Config.TILE_SIZE, by*Config.TILE_SIZE))) {

							// BaseLocation 에서 ChokePoint 방향 절반 지점의 Back Yard : 데카르트 좌표계에서 (cos(t),   sin(t))
							bx = tempBaseLocation.getTilePosition().getX() + (int)(d * Math.cos(t) / Config.TILE_SIZE);
							by = tempBaseLocation.getTilePosition().getY() - (int)(d * Math.sin(t) / Config.TILE_SIZE);
							tempTilePosition = new TilePosition(bx, by);
							// std::cout << "ConstructionPlaceFinder MainBaseBackYard tempTilePosition " << tempTilePosition.x << "," << tempTilePosition.y << std::endl;
							//std::cout << "m";
						}

					}
				}
				//std::cout << "z";
				if (!tempTilePosition.isValid() || !MyBotModule.Broodwar.isBuildable(tempTilePosition.getX(), tempTilePosition.getY(), false)) {
					desiredPosition = getBuildLocationNear(buildingType, tempBaseLocation.getTilePosition());
				}
				else {
					desiredPosition = getBuildLocationNear(buildingType, tempTilePosition);
				}
				//std::cout << "w";
				// std::cout << "ConstructionPlaceFinder MainBaseBackYard desiredPosition " << desiredPosition.x << "," << desiredPosition.y << std::endl;
				break;

			case FirstExpansionLocation:
				tempBaseLocation = InformationManager.Instance().getFirstExpansionLocation(MyBotModule.Broodwar.self());
				if (tempBaseLocation != null) {
					desiredPosition = getBuildLocationNear(buildingType, tempBaseLocation.getTilePosition());
				}
				break;

			case FirstChokePoint:
				tempChokePoint = InformationManager.Instance().getFirstChokePoint(MyBotModule.Broodwar.self());
				if (tempChokePoint != null) {
					desiredPosition = getBuildLocationNear(buildingType, tempChokePoint.getCenter().toTilePosition());
				}
				break;

			case SecondChokePoint:
				tempChokePoint = InformationManager.Instance().getSecondChokePoint(MyBotModule.Broodwar.self());
				if (tempChokePoint != null) {
					desiredPosition = getBuildLocationNear(buildingType, tempChokePoint.getCenter().toTilePosition());
				}
				break;
			}
		}

		return desiredPosition;

		// BasicBot 1.1 Patch End //////////////////////////////////////////////////
	}

	/// desiredPosition 근처에서 건물 건설 가능 위치를 탐색해서 리턴합니다<br>
	/// desiredPosition 주위에서 가능한 곳을 찾아 반환합니다<br>
	/// desiredPosition 이 valid 한 곳이 아니라면, desiredPosition 를 MainBaseLocation 로 해서 주위를 찾는다<br>
	/// Returns a suitable TilePosition to build a given building type near specified TilePosition aroundTile.<br>
	/// Returns BWAPI::TilePositions::None, if suitable TilePosition is not exists (다른 유닛들이 자리에 있어서, Pylon, Creep, 건물지을 타일 공간이 전혀 없는 경우 등)
	public final TilePosition getBuildLocationNear(UnitType buildingType, TilePosition desiredPosition)
	{
		if (buildingType.isRefinery())
		{
			//std::cout << "getRefineryPositionNear "<< std::endl;

			return getRefineryPositionNear(desiredPosition);
		}

		if (MyBotModule.Broodwar.self().getRace() == Race.Protoss) {
			// special easy case of having no pylons
			if (buildingType.requiresPsi() && MyBotModule.Broodwar.self().completedUnitCount(UnitType.Protoss_Pylon) == 0)
			{
				return TilePosition.None;
			}
		}

		if (desiredPosition == TilePosition.None || desiredPosition == TilePosition.Unknown || desiredPosition == TilePosition.Invalid || desiredPosition.isValid() == false)
		{
			desiredPosition = InformationManager.Instance().getMainBaseLocation(MyBotModule.Broodwar.self()).getTilePosition();
		}

		TilePosition testPosition = TilePosition.None;

		// TODO 과제 : 건설 위치 탐색 방법은 ConstructionPlaceSearchMethod::SpiralMethod 로 하는데, 더 좋은 방법은 생각해볼 과제이다
		int constructionPlaceSearchMethod = ConstructionPlaceSearchMethod.SpiralMethod.ordinal();
		
		// 일반적인 건물에 대해서는 건물 크기보다 Config::Macro::BuildingSpacing 칸 만큼 상하좌우로 더 넓게 여유공간을 두어서 빈 자리를 검색한다
		int buildingGapSpace = Config.BuildingSpacing;

		// ResourceDepot (Nexus, Command Center, Hatchery),
		// Protoss_Pylon, Terran_Supply_Depot, 
		// Protoss_Photon_Cannon, Terran_Bunker, Terran_Missile_Turret, Zerg_Creep_Colony 는 다른 건물 바로 옆에 붙여 짓는 경우가 많으므로 
		// buildingGapSpace을 다른 Config 값으로 설정하도록 한다
		if (buildingType.isResourceDepot()) {
			buildingGapSpace = Config.BuildingResourceDepotSpacing;		
		}
		else if (buildingType == UnitType.Protoss_Pylon) {
			int numPylons = MyBotModule.Broodwar.self().completedUnitCount(UnitType.Protoss_Pylon);
			
			// Protoss_Pylon 은 특히 최초 2개 건설할때는 Config::Macro::BuildingPylonEarlyStageSpacing 값으로 설정한다
			if (numPylons < 3) {
				buildingGapSpace = Config.BuildingPylonEarlyStageSpacing;
			}
			else {
				buildingGapSpace = Config.BuildingPylonSpacing;
			}
		}
		else if (buildingType == UnitType.Terran_Supply_Depot) {
			buildingGapSpace = Config.BuildingSupplyDepotSpacing;
		}
		else if (buildingType == UnitType.Protoss_Photon_Cannon || buildingType == UnitType.Terran_Bunker 
			|| buildingType == UnitType.Terran_Missile_Turret || buildingType == UnitType.Zerg_Creep_Colony) {
			buildingGapSpace = Config.BuildingDefenseTowerSpacing;
		}

		while (buildingGapSpace >= 0) {

			testPosition = getBuildLocationNear(buildingType, desiredPosition, buildingGapSpace, constructionPlaceSearchMethod);

			// std::cout << "ConstructionPlaceFinder testPosition " << testPosition.x << "," << testPosition.y << std::endl;

			if (testPosition != TilePosition.None && testPosition != TilePosition.Invalid)
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

		return TilePosition.None;
	}

	/// 해당 buildingType 이 건설될 수 있는 위치를 desiredPosition 근처에서 탐색해서 탐색결과를 리턴합니다<br>
	/// buildingGapSpace를 반영해서 canBuildHereWithSpace 를 사용해서 체크<br>
	/// 못찾는다면 BWAPI::TilePositions::None 을 리턴합니다<br>
	/// TODO 과제 : 건물을 계획없이 지을수 있는 곳에 짓는 것을 계속 하다보면, 유닛이 건물 사이에 갇히는 경우가 발생할 수 있는데, 이를 방지하는 방법은 생각해볼 과제입니다
	public final TilePosition getBuildLocationNear(UnitType buildingType, TilePosition desiredPosition, int buildingGapSpace, int constructionPlaceSearchMethod)
	{
		// std::cout << std::endl << "getBuildLocationNear " << buildingType.getName().c_str() << " " << desiredPosition.x << "," << desiredPosition.y 
		//	<< " gap " << buildingGapSpace << " method " << constructionPlaceSearchMethod << std::endl;

		//returns a valid build location near the desired tile position (x,y).
		TilePosition resultPosition = TilePosition.None;
		TilePosition tempPosition;
		ConstructionTask b = new ConstructionTask(buildingType, desiredPosition);

		// maxRange 를 설정하지 않거나, maxRange 를 128으로 설정하면 지도 전체를 다 탐색하는데, 매우 느려질뿐만 아니라, 대부분의 경우 불필요한 탐색이 된다
		// maxRange 는 16 ~ 64가 적당하다
		int maxRange = 32; // maxRange = BWAPI::Broodwar->mapWidth()/4;
		boolean isPossiblePlace = false;
			
		if (constructionPlaceSearchMethod == ConstructionPlaceSearchMethod.SpiralMethod.ordinal())
		{
			// desiredPosition 으로부터 시작해서 spiral 하게 탐색하는 방법
			// 처음에는 아래 방향 (0,1) -> 오른쪽으로(1,0) -> 위로(0,-1) -> 왼쪽으로(-1,0) -> 아래로(0,1) -> ..
			int currentX = desiredPosition.getX();
			int currentY = desiredPosition.getY();
			int spiralMaxLength = 1;
			int numSteps = 0;
			boolean isFirstStep = true;

			int spiralDirectionX = 0;
			int spiralDirectionY = 1;
			while (spiralMaxLength < maxRange)
			{
				if (currentX >= 0 && currentX < MyBotModule.Broodwar.mapWidth() && currentY >= 0 && currentY < MyBotModule.Broodwar.mapHeight()) {

					isPossiblePlace = canBuildHereWithSpace(new TilePosition(currentX, currentY), b, buildingGapSpace);

					if (isPossiblePlace) {
						resultPosition = new TilePosition(currentX, currentY);
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
		else if (constructionPlaceSearchMethod == ConstructionPlaceSearchMethod.NewMethod.ordinal()) {
		}

		return resultPosition;
	}

	/// 해당 위치에 건물 건설이 가능한지 여부를 buildingGapSpace 조건을 포함해서 판단하여 리턴합니다<br>
	/// Broodwar 의 canBuildHere, isBuildableTile, isReservedTile 를 체크합니다
	public final boolean canBuildHereWithSpace(TilePosition position, final ConstructionTask b, int buildingGapSpace)
	{
		//if we can't build here, we of course can't build here with space
		if (!canBuildHere(position, b))
		{
			return false;
		}

		// height and width of the building
		int width = b.getType().tileWidth();
		int height = b.getType().tileHeight();

		// define the rectangle of the building spot
		// 건물 크기보다 상하좌우로 더 큰 사각형
		int startx;
		int starty;
		int endx;
		int endy;

		boolean horizontalOnly = false;

		// Refinery 의 경우 GapSpace를 체크할 필요 없다
		if (b.getType().isRefinery())
		{
		}
		// Addon 타입의 건물일 경우에는, 그 Addon 건물 왼쪽에 whatBuilds 건물이 있는지를 체크한다
		if (b.getType().isAddon())
		{
			final UnitType builderType = b.getType().whatBuilds().first;

			TilePosition builderTile = new TilePosition(position.getX() - builderType.tileWidth(), position.getY() + 2 - builderType.tileHeight());

			startx = builderTile.getX() - buildingGapSpace;
			starty = builderTile.getY() - buildingGapSpace;
			endx = position.getX() + width + buildingGapSpace;
			endy = position.getY() + height + buildingGapSpace;

			// builderTile에 Lifted 건물이 아니고 whatBuilds 건물이 아닌 건물이 있는지 체크
			for (int i = 0; i <= builderType.tileWidth(); ++i)
			{
				for (int j = 0; j <= builderType.tileHeight(); ++j)
				{
					for (Unit unit : MyBotModule.Broodwar.getUnitsOnTile(builderTile.getX() + i, builderTile.getY() + j))
					{
						if ((unit.getType() != builderType) && (!unit.isLifted()))
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
			if (b.getType() == UnitType.Terran_Command_Center ||
				b.getType() == UnitType.Terran_Factory ||
				b.getType() == UnitType.Terran_Starport ||
				b.getType() == UnitType.Terran_Science_Facility)
			{
				width += 2;
			}

			// 상하좌우에 buildingGapSpace 만큼 간격을 띄운다
			if (horizontalOnly == false)
			{
				startx = position.getX() - buildingGapSpace;
				starty = position.getY() - buildingGapSpace;
				endx = position.getX() + width + buildingGapSpace;
				endy = position.getY() + height + buildingGapSpace;
			}
			// 좌우로만 buildingGapSpace 만큼 간격을 띄운다
			else {
				startx = position.getX() - buildingGapSpace;
				starty = position.getY();
				endx = position.getX() + width + buildingGapSpace;
				endy = position.getY() + height;
			}

			// 테란종족 건물의 경우 다른 건물의 Addon 공간을 확보해주기 위해, 왼쪽 2칸은 반드시 GapSpace가 되도록 한다
			if (b.getType().getRace() == Race.Terran) {
				if (buildingGapSpace < 2) {
					startx = position.getX() - 2;
					endx = position.getX() + width + buildingGapSpace;
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
					if (b.getType().isResourceDepot() == false && b.getType().isAddon() == false) {
						if (isTilesToAvoid(x, y)) {
							return false;
						}
					}
				}
			}
		}

		// if this rectangle doesn't fit on the map we can't build here
		if (startx < 0 || starty < 0 || endx > MyBotModule.Broodwar.mapWidth() || endx < position.getX() + width || endy > MyBotModule.Broodwar.mapHeight())
		{
			return false;
		}

		return true;
	}

	/// 해당 위치에 건물 건설이 가능한지 여부를 리턴합니다 <br>
	/// Broodwar 의 canBuildHere 및 _reserveMap 와 isOverlapsWithBaseLocation 을 체크
	public final boolean canBuildHere(TilePosition position, final ConstructionTask b)
	{
		/*if (!b.type.isRefinery() && !InformationManager::Instance().tileContainsUnit(position))
		{
		return false;
		}*/
		
		// This function checks for creep, power, and resource distance requirements in addition to the tiles' buildability and possible units obstructing the build location.
//		if (!MyBotModule.Broodwar.canBuildHere(position, b.getType(), b.getConstructionWorker()))
		if (!MyBotModule.Broodwar.canBuildHere(position, b.getType()))
		{
			return false;
		}
		
		// check the reserve map
		for (int x = position.getX() ; x < position.getX() + b.getType().tileWidth(); x++)
		{
			for (int y = position.getY() ; y < position.getY() + b.getType().tileHeight(); y++)
			{
				//if (reserveMap.get(x).get(y))
				if (reserveMap[x][y])
				{
					return false;
				}
			}
		}

		// if it overlaps a base location return false
		// ResourceDepot 건물이 아닌 다른 건물은 BaseLocation 위치에 짓지 못하도록 한다
		if (isOverlapsWithBaseLocation(position, b.getType()))
		{
			return false;
		}

		return true;
	}

	/// seedPosition 근처에서 Refinery 건물 건설 가능 위치를 탐색해서 리턴합니다 <br>
	/// 지도상의 여러 가스 광산 (Resource_Vespene_Geyser) 중 예약되어있지 않은 곳(isReservedTile), 다른 섬이 아닌 곳, 이미 Refinery 가 지어져있지않은 곳 중<br> 
	/// seedPosition 과 가장 가까운 곳을 리턴합니다
	public final TilePosition getRefineryPositionNear(TilePosition seedPosition)
	{
		// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
		// Refinery 건물 건설 위치 탐색 로직 버그 수정 및 속도 개선 : seedPosition 주위에서만 geyser를 찾도록, 이미 Refinery가 지어져있는지 체크하지 않도록 수정
		
		if (seedPosition == TilePosition.None || seedPosition == TilePosition.Unknown || seedPosition == TilePosition.Invalid || seedPosition.isValid() == false)
		{
			seedPosition = InformationManager.Instance().getMainBaseLocation(MyBotModule.Broodwar.self()).getTilePosition();
		}
		
		TilePosition closestGeyser = TilePosition.None;
		double minGeyserDistanceFromSeedPosition = 100000000;

		// 전체 geyser 중에서 seedPosition 으로부터 16 TILE_SIZE 거리 이내에 있는 것을 찾는다
		for (Unit geyser : MyBotModule.Broodwar.getStaticGeysers())
		{
			// geyser->getPosition() 을 하면, Unknown 으로 나올 수 있다.
			// 반드시 geyser->getInitialPosition() 을 사용해야 한다
			Position geyserPos = geyser.getInitialPosition();
			TilePosition geyserTilePos = geyser.getInitialTilePosition();

			// 이미 예약되어있는가
			if (isReservedTile(geyserTilePos.getX(), geyserTilePos.getY())) {
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
				closestGeyser = geyser.getInitialTilePosition();
			}
		}
		
		return closestGeyser;

		// BasicBot 1.1 Patch End //////////////////////////////////////////////////
	}

	/// 해당 위치가 BaseLocation 과 겹치는지 여부를 리턴합니다<br>
	/// BaseLocation 에는 ResourceDepot 건물만 건설하고, 다른 건물은 건설하지 않기 위함입니다
	public final boolean isOverlapsWithBaseLocation(TilePosition tile, UnitType type)
	{
		// if it's a resource depot we don't care if it overlaps
		if (type.isResourceDepot())
		{
			return false;
		}

		// dimensions of the proposed location
		int tx1 = tile.getX();
		int ty1 = tile.getY();
		int tx2 = tx1 + type.tileWidth();
		int ty2 = ty1 + type.tileHeight();

		// for each base location
		for (BaseLocation base : BWTA.getBaseLocations())
		{
			// dimensions of the base location
			int bx1 = base.getTilePosition().getX();
			int by1 = base.getTilePosition().getY();
			int bx2 = bx1 + InformationManager.Instance().getBasicResourceDepotBuildingType().tileWidth();
			int by2 = by1 + InformationManager.Instance().getBasicResourceDepotBuildingType().tileHeight();

			// conditions for non-overlap are easy
			boolean noOverlap = (tx2 < bx1) || (tx1 > bx2) || (ty2 < by1) || (ty1 > by2);

			// if the reverse is true, return true
			if (!noOverlap)
			{
				return true;
			}
		}

		// otherwise there is no overlap
		return false;
	}

	/// 건물 건설 가능 타일인지 여부를 리턴합니다
	public final boolean isBuildableTile(final ConstructionTask b, int x, int y)
	{
		TilePosition tp = new TilePosition(x, y);
		if (!tp.isValid())
		{
			return false;
		}

		// 맵 데이터 뿐만 아니라 빌딩 데이터를 모두 고려해서 isBuildable 체크
		//if (BWAPI::Broodwar->isBuildable(x, y) == false)
		if (MyBotModule.Broodwar.isBuildable(x, y, true) == false)
		{
			return false;
		}

		// constructionWorker 이외의 다른 유닛이 있으면 false를 리턴한다
		for (Unit unit : MyBotModule.Broodwar.getUnitsOnTile(x, y))
		{
			if ((b.getConstructionWorker() != null) && (unit != b.getConstructionWorker()))
			{
				return false;
			}
		}

		return true;
	}

	/// 건물 건설 예정 타일로 예약해서, 다른 건물을 중복해서 짓지 않도록 합니다
	public void reserveTiles(TilePosition position, int width, int height)
	{
		/*int rwidth = reserveMap.size();
		int rheight = reserveMap.get(0).size();
		for (int x = position.getX(); x < position.getX() + width && x < rwidth; x++)
		{
			for (int y = position.getY() ; y < position.getY() + height && y < rheight; y++)
			{
				reserveMap.get(x).set(y, true);
				// C++ : reserveMap[x][y] = true;
			}
		}*/
		int rwidth = reserveMap.length;
		int rheight = reserveMap[0].length;
		for (int x = position.getX(); x < position.getX() + width && x < rwidth; x++)
		{
			for (int y = position.getY() ; y < position.getY() + height && y < rheight; y++)
			{
				//reserveMap.get(x).set(y, true);
				reserveMap[x][y] = true;
				// C++ : reserveMap[x][y] = true;
			}
		}
	}
	
	/// 건물 건설 예정 타일로 예약했던 것을 해제합니다
	public void freeTiles(TilePosition position, int width, int height)
	{
		/*int rwidth = reserveMap.size();
		int rheight = reserveMap.get(0).size();

		for (int x = position.getX(); x < position.getX() + width && x < rwidth; x++)
		{
			for (int y = position.getY() ; y < position.getY() + height && y < rheight; y++)
			{
				reserveMap.get(x).set(y, false);
				// C++ : reserveMap[x][y] = false;
			}
		}*/
		int rwidth = reserveMap.length;
		int rheight = reserveMap[0].length;

		for (int x = position.getX(); x < position.getX() + width && x < rwidth; x++)
		{
			for (int y = position.getY() ; y < position.getY() + height && y < rheight; y++)
			{
				//reserveMap.get(x).set(y, false);
				reserveMap[x][y] = false;
				// C++ : reserveMap[x][y] = false;
			}
		}
	}

	// 건물 건설 예약되어있는 타일인지 체크
	public final boolean isReservedTile(int x, int y)
	{
		/*int rwidth = reserveMap.size();
		int rheight = reserveMap.get(0).size();
		if (x < 0 || y < 0 || x >= rwidth || y >= rheight)
		{
			return false;
		}

		return reserveMap.get(x).get(y);*/
		int rwidth = reserveMap.length;
		int rheight = reserveMap[0].length;
		if (x < 0 || y < 0 || x >= rwidth || y >= rheight)
		{
			return false;
		}

		return reserveMap[x][y];
	}

	/// reserveMap을 리턴합니다
	public boolean[][] getReserveMap() {
		return reserveMap;
	}

	/// (x, y) 가 BaseLocation 과 Mineral / Geyser 사이의 타일에 해당하는지 여부를 리턴합니다
	public final boolean isTilesToAvoid(int x, int y)
	{
		for (TilePosition t : tilesToAvoid) {
			if (t.getX() == x && t.getY() == y) {
				return true;
			}
		}

		return false;
	}

	/// BaseLocation 과 Mineral / Geyser 사이의 타일들을 찾아 _tilesToAvoid 에 저장합니다<br>
	/// BaseLocation 과 Geyser 사이, ResourceDepot 건물과 Mineral 사이 공간으로 건물 건설 장소를 정하면<br> 
	/// 일꾼 유닛들이 장애물이 되어서 건설 시작되기까지 시간이 오래걸리고, 지어진 건물이 장애물이 되어서 자원 채취 속도도 느려지기 때문에, 이 공간은 건물을 짓지 않는 공간으로 두기 위함입니다
	public void setTilesToAvoid()
	{
		// ResourceDepot 건물의 width = 4 타일, height = 3 타일
		// Geyser 의            width = 4 타일, height = 2 타일
		// Mineral 의           width = 2 타일, height = 1 타일

		for (BaseLocation base : BWTA.getBaseLocations())
		{
			// Island 일 경우 건물 지을 공간이 절대적으로 좁기 때문에 건물 안짓는 공간을 두지 않는다
			if (base.isIsland()) continue;
			if (BWTA.isConnected(base.getTilePosition(), InformationManager.Instance().getMainBaseLocation(MyBotModule.Broodwar.self()).getTilePosition()) == false) continue;

			// dimensions of the base location
			int bx0 = base.getTilePosition().getX();
			int by0 = base.getTilePosition().getY();
			int bx4 = base.getTilePosition().getX() + 4;
			int by3 = base.getTilePosition().getY() + 3;

			// BaseLocation 과 Geyser 사이의 타일을 BWTA::getShortestPath 를 사용해서 구한 후 _tilesToAvoid 에 추가
			for (Unit geyser : base.getGeysers())
			{
				TilePosition closeGeyserPosition = geyser.getInitialTilePosition();

				// dimensions of the closest geyser
				int gx0 = closeGeyserPosition.getX();
				int gy0 = closeGeyserPosition.getY();
				int gx4 = closeGeyserPosition.getX() + 4;
				int gy2 = closeGeyserPosition.getY() + 2;

				for (int i = bx0; i < bx4; i++) {
					for (int j = by0; j < by3; j++) {
						for (int k = gx0; k < gx4; k++) {
							for (int l = gy0; l < gy2; l++) {
								List<TilePosition> tileList = (List<TilePosition>) BWTA.getShortestPath(new TilePosition(i, j), new TilePosition(k, l));
								for (TilePosition t : tileList) {
									tilesToAvoid.add(t);									
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
			for (Unit mineral : base.getMinerals())
			{
				TilePosition closeMineralPosition = mineral.getInitialTilePosition();

				// dimensions of the closest mineral
				int mx0 = closeMineralPosition.getX();
				int my0 = closeMineralPosition.getY();
				int mx2 = mx0 + 2;
				int my1 = my0 + 1;

				for (int i = bx0; i < bx4; i++) {
					for (int j = by0; j < by3; j++) {
						for (int k = mx0; k < mx2; k++) {
							List<TilePosition> tileList = (List<TilePosition>) BWTA.getShortestPath(new TilePosition(i, j), new TilePosition(k, my0));
							for (TilePosition t : tileList) {
								tilesToAvoid.add(t);								
							}
						}
					}
				}
			}
		}
	}
	
	/// BaseLocation 과 Mineral / Geyser 사이의 타일들의 목록을 리턴합니다		
	public Set<TilePosition> getTilesToAvoid() {
		return tilesToAvoid;
	}
	
}