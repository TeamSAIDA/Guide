import bwapi.Color;
import bwapi.DefaultBWListener;
import bwapi.Game;
import bwapi.Mirror;
import bwapi.Player;
import bwapi.Position;
import bwapi.Unit;
import bwapi.Flag.Enum;
import bwta.BWTA;

public class MyBotModule extends DefaultBWListener {
	
	private Mirror mirror = new Mirror();
	public static Game Broodwar;

	public void run() {
		mirror.getModule().setEventListener(this);
		mirror.startGame();
	}

	@Override
	public void onStart() {
		Broodwar = mirror.getGame();
		
		if (Broodwar.isReplay()) {
			return;
		}

		// 전체 지도 및 상대편 이벤트들 다 파악하는 모드
		//game.enableFlag(Enum.CompleteMapInformation.getValue());

		// 키보드/마우스로 게임 플레이를 진행할 수 있는 모드
		Broodwar.enableFlag(Enum.UserInput.getValue());

		// 동일한 게임 명령은 하나로 처리해서 CPU 부담을 줄여줌
		Broodwar.setCommandOptimizationLevel(1);

		// Speedups for automated play, sets the number of milliseconds bwapi spends in each frame
		// Fastest: 42 ms/frame.  1초에 24 frame. 일반적으로 1초에 24frame을 기준 게임속도로 한다
		// Normal: 67 ms/frame. 1초에 15 frame
		// As fast as possible : 0 ms/frame. CPU가 할수있는 가장 빠른 속도. 
		Broodwar.setLocalSpeed(20);
		// frameskip을 늘리면 화면 표시도 업데이트 안하므로 훨씬 빠르다
		Broodwar.setFrameSkip(0);

		System.out.println("Map analyzing started");
		BWTA.readMap();
		BWTA.analyze();
		BWTA.buildChokeNodes();
		System.out.println("Map analyzing finished");		
	}

	@Override
	public void onEnd(boolean isWinner) {
		if (isWinner){
			System.out.println("I won the game");
		} else {
			System.out.println("I lost the game");
		}
		
        System.out.println("Match ended");
        System.exit(0);
	}

	@Override
	public void onFrame() {
		
		// 대결 시작한지 500 frame 이 되었을 때 1번만 표시
		if (Broodwar.getFrameCount() == 500) {

			// 명령 프롬프트에 표시
			System.out.println("Hello Starcraft command prompt");

			// 게임 화면에 표시
			Broodwar.printf("Hello Starcraft game screen");
		}

		
		if (Broodwar.isReplay()) {
			return;
		}
	}

	@Override
	public void onSendText(String text){
		// Display the text to the game
		Broodwar.sendText(text);
		
		Broodwar.printf(text);
	}

	@Override
	public void onReceiveText(Player player, String text){
		Broodwar.printf(player.getName() + " said \"" + text + "\"");
	}

	@Override
	public void onPlayerLeft(Player player){
		Broodwar.printf(player.getName() + " left the game.");
	}

	@Override
	public void onNukeDetect(Position target){
		if (target != Position.Unknown)	{
			Broodwar.drawCircleMap(target, 40, Color.Red, true);
			Broodwar.printf("Nuclear Launch Detected at " + target);
		} else {
			Broodwar.printf("Nuclear Launch Detected");
		}
	}

	@Override
	public void onUnitCreate(Unit unit){
		if (unit.getPlayer().isNeutral() == false) {
			Broodwar.printf(unit.getType() + " " + unit.getID() + " created at " + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY());
		}	
	}

	@Override
	public void onUnitMorph(Unit unit){
		if (unit.getPlayer().isNeutral() == false) {
			Broodwar.printf(unit.getType() + " " + unit.getID() + " morphed at " + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY());
		}	
	}

	@Override
	public void onUnitDestroy(Unit unit){
		if (unit.getPlayer().isNeutral() == false) {
			Broodwar.printf(unit.getType() + " " + unit.getID() + " destroyed at " + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY());
		}	
	}

	@Override
	public void onUnitShow(Unit unit){
		if (unit.getPlayer().isNeutral() == false) {
			Broodwar.printf(unit.getType() + " " + unit.getID() + " showed at " + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY());
		}	
	}

	@Override
	public void onUnitHide(Unit unit){
		if (unit.getPlayer().isNeutral() == false) {
			Broodwar.printf(unit.getType() + " " + unit.getID() + " hid at " + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY());
		}	
	}

	@Override
	public void onUnitRenegade(Unit unit){
		if (unit.getPlayer().isNeutral() == false) {
			Broodwar.printf(unit.getType() + " " + unit.getID() + " renegaded at " + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY());
		}	
	}

	@Override
	public void onUnitDiscover(Unit unit){
		if (unit.getPlayer().isNeutral() == false) {
			Broodwar.printf(unit.getType() + " " + unit.getID() + " discovered at " + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY());
		}	
	}

	@Override
	public void onUnitEvade(Unit unit){
		if (unit.getPlayer().isNeutral() == false) {
			Broodwar.printf(unit.getType() + " " + unit.getID() + " evaded at " + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY());
		}	
	}

	@Override
	public void onUnitComplete(Unit unit){
		if (unit.getPlayer().isNeutral() == false) {
			Broodwar.printf(unit.getType() + " " + unit.getID() + " completed at " + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY());
		}	
	}

	@Override
	public void onSaveGame(String gameName){
		Broodwar.printf("The game was saved to \"" + gameName + "\".");
	}
}