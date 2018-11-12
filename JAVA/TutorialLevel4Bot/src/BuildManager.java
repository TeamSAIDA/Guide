import bwapi.Race;
import bwapi.TilePosition;
import bwapi.Unit;
import bwapi.UnitType;

public class BuildManager {

	private static BuildManager instance = new BuildManager();

	public static BuildManager Instance() {
		return instance;
	}

	public void update() {
		
		//constructBuildings();

		//buildCombatUnits();

		buildWorkerUnits();
	}
	
	
	private void buildWorkerUnits()
	{
		// 자원이 50이상 있으면 일꾼 유닛을 훈련한다
		if (MyBotModule.Broodwar.self().minerals() >= 50) {
			buildWorkerUnit();
		}
	}
	
	private void buildWorkerUnit()
	{
		Unit producer = null;
	
		UnitType targetUnitType = UnitType.None;
	
		if (MyBotModule.Broodwar.self().getRace() == Race.Protoss) {
			targetUnitType = UnitType.Protoss_Probe;
		}
		else if (MyBotModule.Broodwar.self().getRace() == Race.Terran) {
			targetUnitType = UnitType.Terran_SCV;
		}
		else if (MyBotModule.Broodwar.self().getRace() == Race.Zerg) {
			targetUnitType = UnitType.Zerg_Drone;
		}
			
		// ResourceDepot 건물이 일꾼 유닛을 생산 가능한 상태이면 생산을 명령한다
		for (Unit unit : MyBotModule.Broodwar.self().getUnits())
		{
			if (unit.getType().isResourceDepot() ){
	
				if (MyBotModule.Broodwar.canMake(targetUnitType, unit) && unit.isTraining() == false && unit.isMorphing() == false) {
	
					producer = unit;
	
					if (MyBotModule.Broodwar.self().getRace() != Race.Zerg) {
						producer.train(targetUnitType);
					}
					else {
						producer.morph(targetUnitType);
					}
					break;
				}
			}
		}
	}
	
	private void buildCombatUnits()
	{
		UnitType targetUnitType = UnitType.None;
	
		// 자원이 100이상 있으면 먼저 전투 유닛을 훈련한다
		if (MyBotModule.Broodwar.self().minerals() >= 100) {
			if (MyBotModule.Broodwar.self().getRace() == Race.Protoss) {
				targetUnitType = UnitType.Protoss_Zealot;
			}
			else if (MyBotModule.Broodwar.self().getRace() == Race.Terran) {
				targetUnitType = UnitType.Terran_Marine;
			}
			else if (MyBotModule.Broodwar.self().getRace() == Race.Zerg) {
				targetUnitType = UnitType.Zerg_Zergling;
			}
	
			trainUnit(targetUnitType);
		}
	}
	
	private void trainUnit(UnitType targetUnitType)
	{
		Unit producer = null;
		UnitType producerUnitType = targetUnitType.whatBuilds().first;
	
		// targetUnitType을 생산 가능한 상태가 되면 생산을 명령한다
		for (Unit unit : MyBotModule.Broodwar.self().getUnits())
		{
			if (unit.getType() == producerUnitType) {
				if (MyBotModule.Broodwar.canMake(targetUnitType, unit) && unit.isTraining() == false && unit.isMorphing() == false) {
	
					producer = unit;
	
					if (MyBotModule.Broodwar.self().getRace() != Race.Zerg) {
						producer.train(targetUnitType);
					}
					else {
						producer.morph(targetUnitType);
					}
					break;
				}
			}
		}
	}
	
	
	private void constructBuildings()
	{
		UnitType targetUnitType = UnitType.None;
	
		// 자원이 200이상 있으면 전투유닛 생산 건물을 건설 한다
		if (MyBotModule.Broodwar.self().minerals() >= 200) {
			if (MyBotModule.Broodwar.self().getRace() == Race.Protoss) {
				targetUnitType = UnitType.Protoss_Gateway;
			}
			else if (MyBotModule.Broodwar.self().getRace() == Race.Terran) {
				targetUnitType = UnitType.Terran_Barracks;
			}
			else if (MyBotModule.Broodwar.self().getRace() == Race.Zerg) {
				targetUnitType = UnitType.Zerg_Spawning_Pool;
			}
			constructBuilding(targetUnitType);
		}
	
		// 자원이 100이상 있고, 서플라이가 부족해지면 SupplyProvider 에 해당하는 유닛을 만든다
		// 서플라이 숫자는 스타크래프트 게임에서 표시되는 숫자의 2배로 계산해야한다
		if (MyBotModule.Broodwar.self().minerals() >= 100
			&& MyBotModule.Broodwar.self().supplyUsed() + 6 > MyBotModule.Broodwar.self().supplyTotal()) {
			if (MyBotModule.Broodwar.self().getRace() == Race.Protoss) {
				targetUnitType = UnitType.Protoss_Pylon;
				constructBuilding(targetUnitType);
			}
			else if (MyBotModule.Broodwar.self().getRace() == Race.Terran) {
				targetUnitType = UnitType.Terran_Supply_Depot;
				constructBuilding(targetUnitType);
			}
			else if (MyBotModule.Broodwar.self().getRace() == Race.Zerg) {
				targetUnitType = UnitType.Zerg_Overlord;
				trainUnit(targetUnitType);
			}
		}
	}
	
	private void constructBuilding(UnitType targetBuildingType)
	{
		// 일꾼 중 미네랄을 운반하고 있지 않은 일꾼 하나를 producer로 선정한다
		Unit producer = null;
		UnitType producerUnitType = targetBuildingType.whatBuilds().first;
	
		for (Unit unit : MyBotModule.Broodwar.self().getUnits())
		{
			if (unit.getType() == producerUnitType) {
				if (MyBotModule.Broodwar.canMake(targetBuildingType, unit)
					&& unit.isCompleted() 
					&& unit.isCarryingMinerals() == false
					&& unit.isConstructing() == false) {
	
					producer = unit;
					break;
				}
			}
		}
	
		if (producer == null) {
			return;
		}
	
		// 건물을 건설할 위치를 Start Location 근처에서 찾는다
		// 처음에는 Start Location 반경 4타일에 대해 찾아보고, 
		// 다음에는 Start Location 반경 8타일에 대해 찾아보는 식으로 범위를 넓혀나간다
		TilePosition seedPosition = InformationManager.Instance().mainBaseLocations.get(InformationManager.Instance().selfPlayer).getTilePosition();
		TilePosition desiredPosition = TilePosition.None;
		int maxRange = 32;
		boolean constructionPlaceFound = false;
	
		for (int range = 4; range <= maxRange; range *= 2) {
			for (int i = seedPosition.getX() - range; i < seedPosition.getX() + range; i++) {
				for (int j = seedPosition.getY() - range; j < seedPosition.getY() + range; j++) {
					desiredPosition = new TilePosition(i,j);
					if (MyBotModule.Broodwar.canBuildHere(desiredPosition, targetBuildingType, producer, true))	{
						constructionPlaceFound = true;
						break;
					}
				}
				if (constructionPlaceFound) break;
			}
			if (constructionPlaceFound) break;
		}
	
		if (constructionPlaceFound == true && desiredPosition != TilePosition.None) {
			producer.build(targetBuildingType, desiredPosition);
		}
	}
	
	

	
};