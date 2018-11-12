import java.util.HashMap;
import java.util.Map;

import bwapi.Race;
import bwapi.Unit;
import bwapi.UnitType;
import bwta.BWTA;
import bwta.BaseLocation;

public class WorkerManager {

	// Worker ~ Mineral Field 간 assign 관계를 저장하는 map
	private Map<Integer, Unit> workerMineralAssignment = new HashMap<Integer, Unit>();

	// 각각의 Mineral Field 에 assign 된 Worker 숫자 를 저장하는 map
	private Map<Integer, Integer> workerCountOnMineral = new HashMap<Integer, Integer>();

	private static WorkerManager instance = new WorkerManager();
	
	public static WorkerManager Instance() {
		return instance;
	}
	
	public WorkerManager() {
		for (Unit unit : MyBotModule.Broodwar.getAllUnits())
		{
			if ((unit.getType() == UnitType.Resource_Mineral_Field))
			{
				workerCountOnMineral.put(unit.getID(), new Integer(0));				
			}
		}
	}
	
	public void update() {
		
		// 각각의 Worker 에 대해서
		// 가장 가까운 아군 ResourceDepot 근처의, 가장 가까운 Mineral 에 채취하도록 하되 (거리 계산 필요)
		// Worker 들이 여러 Mineral 에 분산되도록 한다 (각각의 Mineral 에 할당된 worker 들의 숫자를 저장 / 최신화 해야 한다)
	
		// worker 는 일이 없을 때도 idle 상태가 되지만, 일을 수행하는 도중에도 잠깐 idle 상태가 된다
		for (Unit unit : MyBotModule.Broodwar.self().getUnits()){
	
			if (unit == null) continue;
			
			if (unit.getType().isWorker()) {
	
				// unit 이 idle 상태이고, 탄생한 이후이면 
				if (unit.isIdle() && unit.isCompleted())
				{
					System.out.println(unit.getType() + " " + unit.getID() + " is idle");
	
					// unit 에게 적절한 Mineral 을 찾아, 그 Mineral 로 Right Click 을 한다
					Unit bestMineral = getBestMineralTo(unit);
	
					if (bestMineral != null) {
						System.out.println("bestMineral from " + unit.getType() + " " + unit.getID()
							+ " is " + bestMineral.getType() + " " + bestMineral.getID() 
							+ " at " + bestMineral.getTilePosition().getX() + "," + bestMineral.getTilePosition().getY());
	
						unit.gather(bestMineral);
	
						// unit 과 Mineral 간 assign 정보를 업데이트한다
						workerMineralAssignment.put(new Integer(unit.getID()), bestMineral);
						// Mineral 별 assigned unit 숫자를 업데이트한다
						increaseWorkerCountOnMineral(bestMineral, 1);
					}
				}
			}
		}
	
		// Mineral 별 assigned unit 숫자를 화면에 표시
		for (Integer i : workerMineralAssignment.keySet()) {
			Unit mineral = workerMineralAssignment.get(i);
			if (workerCountOnMineral.containsKey(new Integer(mineral.getID()))) {
				MyBotModule.Broodwar.drawTextMap(mineral.getPosition().getX(), mineral.getPosition().getY() + 12, 
						"worker: " + workerCountOnMineral.get(mineral.getID()));
			}
		}

		/*
		// Worker 별 Mineral 지정을 콘솔에 표시
		System.out.println("\nworkerMineralAssignment size " + workerMineralAssignment.size());
		for (Integer i : workerMineralAssignment.keySet()) {
			System.out.println("worker " + i + " ~ mineral " + workerMineralAssignment.get(i).getID());
		}		
		*/		

	}
	
	public Unit getBestMineralTo(Unit worker)
	{
		if (worker == null) return null;
	
		// worker으로부터 가장 가까운 BaseLocation을 찾는다
		BaseLocation closestBaseLocation = null;
		// 128 * 128 타일사이즈의 맵에서 가장 먼 거리는 sqrt(128 * 32  * 128 * 32 + 128 * 32 * 128 * 32) = 5792.6 point 
		double closestDistance = 1000000000;
	
		for (BaseLocation baseLocation : BWTA.getBaseLocations()){
	
			if (baseLocation == null) continue;
	
			double distance = worker.getDistance(baseLocation.getPosition());
	
			if (distance < closestDistance)
			{
				closestBaseLocation = baseLocation;
				closestDistance = distance;
			}
		}
	
		if (closestBaseLocation == null) {
			return null;
		}
		
		System.out.println("closestBaseLocation from " + worker.getType() + " " + worker.getID()
			+ " is " + closestBaseLocation.getTilePosition().getX() + "," + closestBaseLocation.getTilePosition().getY());
	
		// 해당 BaseLocation 의 Mineral 들 중에서 worker 가 가장 적게 지정되어있는 것, 그중에서도 BaseLocation 으로부터 가장 가까운 것을 찾는다
		Unit bestMineral = null;
		double bestDistance = 1000000000;
		int bestNumAssigned = 1000000000;
	
		//BaseLocation.getMinerals() . 어두운 영역에 있으면, null 을 리턴
		//BaseLocation.getStaticMinerals() . 어두운 영역에 있으면, UnitTypes.Unknown 을 리턴
		for (Unit mineral : closestBaseLocation.getMinerals()){
			if (mineral == null) continue;
	
			// 해당 Mineral 에 지정된 worker 숫자
			int numAssigned = 0;
			if (workerCountOnMineral.containsKey(new Integer(mineral.getID())) ){
				Integer n = workerCountOnMineral.get(new Integer(mineral.getID()));
				numAssigned = n.intValue();
			}
			// 해당 Mineral 과 BaseLocation 간의 거리
			double dist = mineral.getDistance(closestBaseLocation.getPosition());
	
			if (numAssigned < bestNumAssigned)
			{
				bestMineral = mineral;
				bestDistance = dist;
				bestNumAssigned = numAssigned;
			}
			else if (numAssigned == bestNumAssigned)
			{
				if (dist < bestDistance)
				{
					bestMineral = mineral;
					bestDistance = dist;
					bestNumAssigned = numAssigned;
				}
			}
		}
	
		return bestMineral;
	}
	
	public void increaseWorkerCountOnMineral(Unit mineral, int num)
	{
		// Mineral 에 assign 된 worker 숫자를 변경한다
		if (workerCountOnMineral.containsKey(new Integer(mineral.getID())))
		{
			workerCountOnMineral.replace(new Integer(mineral.getID()), new Integer(workerCountOnMineral.get(mineral.getID()) + num));
		}
		else
		{
			workerCountOnMineral.put(new Integer(mineral.getID()), new Integer(num));
		}
	}
	
	public void onUnitDestroy(Unit unit)
	{
		if (unit == null) return;
	
		if (unit.getType().isWorker() && unit.getPlayer() == MyBotModule.Broodwar.self()) 
		{
			// 해당 일꾼과 Mineral 간 assign 정보를 삭제한다
			System.out.println("removeWorker " + unit.getID() + " from Mineral Worker ");
			increaseWorkerCountOnMineral(workerMineralAssignment.get(unit.getID()), -1);
			workerMineralAssignment.remove(unit.getID());
		}
	}
	
	public void onUnitMorph(Unit unit)
	{
		if (unit == null) return;
	
		// 저그 종족 일꾼이 건물로 morph 한 경우
		if (unit.getPlayer().getRace() == Race.Zerg && unit.getPlayer() == MyBotModule.Broodwar.self() && unit.getType().isBuilding())
		{
			// 해당 일꾼과 Mineral 간 assign 정보를 삭제한다
			System.out.println("removeWorker " + unit.getID() + " from Mineral Worker ");				
			increaseWorkerCountOnMineral(workerMineralAssignment.get(unit.getID()), -1);
			workerMineralAssignment.remove(unit.getID());
		}
	}
	

	
}