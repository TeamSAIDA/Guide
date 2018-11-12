import java.util.HashMap;
import java.util.Map;

import bwapi.Player;
import bwapi.Race;
import bwapi.Unit;
import bwta.BWTA;
import bwta.BaseLocation;

public class InformationManager {
	private static InformationManager instance = new InformationManager();

	public Player selfPlayer;
	public Player enemyPlayer;
	public Race selfRace;
	public Race enemyRace;

	public Map<Player, BaseLocation> mainBaseLocations;

	public static InformationManager Instance() {
		return instance;
	}

	public Player getSelf() {
		return selfPlayer;
	}

	public InformationManager() {
		selfPlayer = MyBotModule.Broodwar.self();
		enemyPlayer = MyBotModule.Broodwar.enemy();
		selfRace = selfPlayer.getRace();
		enemyRace = enemyPlayer.getRace();
		
		mainBaseLocations = new HashMap<Player, BaseLocation>();
		mainBaseLocations.put(selfPlayer, BWTA.getStartLocation(MyBotModule.Broodwar.self()));
		mainBaseLocations.put(enemyPlayer, null);
	}

	public void update() {
		// enemy 의 종족을 아직 모르는 경우
		if (enemyRace == Race.Unknown) {
			for (Unit unit : MyBotModule.Broodwar.enemy().getUnits())
			{
				enemyRace = unit.getType().getRace();
				break;
			}
		}

		// enemy 의 startLocation을 아직 모르는 경우
		if (mainBaseLocations.get(enemyPlayer) == null) {

			for (BaseLocation startLocation : BWTA.getStartLocations()) {
				
				for (Unit unit : MyBotModule.Broodwar.enemy().getUnits())
				{
					if (unit.getType().isBuilding()) {
						if (BWTA.getRegion(unit.getTilePosition()) == BWTA.getRegion(startLocation.getTilePosition())) {
							mainBaseLocations.put(enemyPlayer, startLocation);
							break;
						}
					}
				}
			}
		}
	}
}