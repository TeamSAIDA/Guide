/*
+----------------------------------------------------------------------+
| BasicBot                                                             |
+----------------------------------------------------------------------+
| Samsung SDS - 2017 Algorithm Contest                                 |
+----------------------------------------------------------------------+
|                                                                      |
+----------------------------------------------------------------------+
| Author: Tekseon Shin  <tekseon.shin@gmail.com>                       |
| Author: Duckhwan Kim  <duckhwan1982.kim@gmail.com>                   |
+----------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------+
| UAlbertaBot                                                          |
+----------------------------------------------------------------------+
| University of Alberta - AIIDE StarCraft Competition                  |
+----------------------------------------------------------------------+
|                                                                      |
+----------------------------------------------------------------------+
| Author: David Churchill <dave.churchill@gmail.com>                   |
+----------------------------------------------------------------------+
*/

import java.util.ArrayList;
import java.util.Arrays;

import bwapi.Color;
import bwapi.DefaultBWListener;
import bwapi.Game;
import bwapi.Mirror;
import bwapi.Player;
import bwapi.Position;
import bwapi.Unit;
import bwapi.UnitType;
import bwapi.Flag.Enum;
import bwta.BWTA;


// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
// MyBotModule 설명 추가
/// MyBotModule 은 봇프로그램의 기본적인 뼈대 구조를 정의한 class 로서, 스타크래프트 경기 도중 발생하는 이벤트들을 GameCommander class 인스턴스에게 전달합니다.<br>
///
/// MyBotModule class는 수정을 하지 말고,<br>
/// 실제 봇프로그램 개발은 GameCommander class 를 수정하는 형태로 진행하도록 합니다.<br>
/// @see GameCommander
///
/// <br><br>
/// 알고리즘 경진대회 의 공정하고 효율적인 운영을 위해 Main, MyBotModule, UXManager 파일은 참가자들이 제출하는 소스코드를 무시하고 덮어쓴 후 빌드합니다 <br>
///
/// 알고리즘 경진대회 빌드서버가 사용하는 Main, MyBotModule, UXManager 파일을 예시적으로 MyBotModule 에 반영하였습니다 <br>
/// 실제 알고리즘 경진대회 빌드서버에서는 코드를 일부 수정해서 빌드하게 할 수 있습니다 <br>
///
/// 알고리즘 경진대회 빌드서버가 사용하는 Main 은 MyBotModule 을 실행시키는 기능을 수행합니다. <br>
/// 알고리즘 경진대회 빌드서버가 사용하는 MyBotModule 은 GameCommander 에게 이벤트를 전달하는 기능을 수행하며, 게임 속도 지연 여부 파악, 게임 무승부 상황 파악 등을 통해 게임을 강제 패배시키거나 강제 종료시키는 행동을 수행합니다. <br>
/// 알고리즘 경진대회 빌드서버가 사용하는 UX Manager 는 알고리즘 경진대회 운영, 사후 판정 등에 필요한 최소한의 내용만 화면에 표시합니다. <br>
/// 이 파일들은 InformationManager 등 다른 파일들과 Dependency가 없도록 개발되었기 때문에, <br>
/// 참가자들은 InformationManager 등 다른 파일들을 자유롭게 수정하실 수 있습니다. 
/// 
// BasicBot 1.1 Patch End //////////////////////////////////////////////////
public class MyBotModule extends DefaultBWListener {

	/// BWAPI 에 해당하는 내부 객체
	private Mirror mirror = new Mirror();
	
	/// 스타크래프트 대결 상황 전체에 대한 상황 파악 및 액션 실행을 제공하는 객체  <br>
	/// C언어에서 BWAPI::Broodwar 에 해당합니다
	public static Game Broodwar;

	/// 실제 봇프로그램
	/// @see GameCommander			
	private GameCommander gameCommander;

	
	
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 타임아웃 패배, 자동 패배 체크 관련 변수 및 메소드 선언
	
	private boolean isExceptionLostConditionSatisfied = false;	/// Exception 으로 인한 패배 체크 결과
	private int exceptionLostConditionSatisfiedFrame = 0;		/// Exception 패배 조건이 시작된 프레임 시점
	private int maxDurationForExceptionLostCondition = 20;		/// Exception 패배 조건이 만족된채 게임을 유지시키는 최대 프레임 수
	
	private boolean isToCheckGameLostCondition = true;			/// 자동 패배 체크 실행 여부
	private boolean isGameLostConditionSatisfied = false;		/// 자동 패배 체크 결과
	private int gameLostConditionSatisfiedFrame = 0;			/// 자동 패배 조건이 시작된 프레임 시점
	private int maxDurationForGameLostCondition = 100;			/// 자동 패배 조건이 만족된채 게임을 유지시키는 최대 프레임 수
		
	private boolean isToCheckTimeOut = true;					/// 타임 아웃 체크 실행 여부
	private int timeOutConditionSatisfiedFrame = 0;				/// 타임 아웃 조건이 시작된 프레임 시점
	private boolean isTimeOutConditionSatisfied = false;		/// 타임 아웃 체크 결과
	private int maxDurationForTimeOutLostCondition = 20;		/// 타임 아웃 조건이 만족된채 게임을 유지시키는 최대 프레임 수
	private ArrayList<Integer> timerLimits = new ArrayList<Integer>();			///< 타임 아웃 한계시간 (ms/frame)
	private ArrayList<Integer> timerLimitsBound = new ArrayList<Integer>();		///< 타임 아웃 초과한계횟수
	private ArrayList<Integer> timerLimitsExceeded = new ArrayList<Integer>();	///< 타임 아웃 초과횟수
	private long[] timeStartedAtFrame = new long[100000];		///< 해당 프레임을 시작한 시각
	private long[] timeElapsedAtFrame = new long[100000];		///< 해당 프레임에서 사용한 시간 (ms)		

	private boolean isToTestTimeOut = false;					///< 타임 아웃 체크 테스트 실행 여부
	private int timeOverTestDuration = 0;
	private int timeOverTestFrameCountLimit = 0;
	private int timeOverTestFrameCount = 0;						///< 타임 아웃 체크 테스트 실행 

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
	
	
	
	public void run() {
		mirror.getModule().setEventListener(this);
		mirror.startGame();
	}

	/// 경기가 시작될 때 일회적으로 발생하는 이벤트를 처리합니다
	@Override
	public void onStart() {

		Broodwar = mirror.getGame();
		
		gameCommander = new GameCommander();

		if (Broodwar.isReplay()) {
			return;
		}

		// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
		// 타임아웃 패배, 자동 패배 체크 관련 변수 초기화

		initializeLostConditionVariables();
		
		// BasicBot 1.1 Patch End //////////////////////////////////////////////////

		// Config 파일 관리가 번거롭고, 배포 및 사용시 Config 파일 위치를 지정해주는 것이 번거롭기 때문에, 
		// Config 를 파일로부터 읽어들이지 않고, Config 클래스의 값을 사용하도록 한다.
		if(Config.EnableCompleteMapInformation){
			Broodwar.enableFlag(Enum.CompleteMapInformation.getValue());
		}

		if(Config.EnableUserInput){
			Broodwar.enableFlag(Enum.UserInput.getValue());
		}

		Broodwar.setCommandOptimizationLevel(1);

		// Speedups for automated play, sets the number of milliseconds bwapi spends in each frame
		// Fastest: 42 ms/frame.  1초에 24 frame. 일반적으로 1초에 24frame을 기준 게임속도로 한다
		// Normal: 67 ms/frame. 1초에 15 frame
		// As fast as possible : 0 ms/frame. CPU가 할수있는 가장 빠른 속도. 
		Broodwar.setLocalSpeed(Config.SetLocalSpeed);
		// frameskip을 늘리면 화면 표시도 업데이트 안하므로 훨씬 빠르다
		Broodwar.setFrameSkip(Config.SetFrameSkip);

		System.out.println("Map analyzing started");
		BWTA.readMap();
		BWTA.analyze();
		BWTA.buildChokeNodes();
		System.out.println("Map analyzing finished");

		gameCommander.onStart();
	}

	///  경기가 종료될 때 일회적으로 발생하는 이벤트를 처리합니다
	@Override
	public void onEnd(boolean isWinner) {
		if (isWinner){
			System.out.println("I won the game");
		} else {
			System.out.println("I lost the game");
		}

		gameCommander.onEnd(isWinner);
		
        System.out.println("Match ended");
        System.exit(0);		
	}

	/// 경기 진행 중 매 프레임마다 발생하는 이벤트를 처리합니다
	@Override
	public void onFrame() {
		if (Broodwar.isReplay()) {
			return;
		}

		// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
		// 타임아웃 패배, 자동 패배 체크 추가

		// timeStartedAtFrame 를 갱신한다
		if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
			timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
		}
			
		// 타임아웃 체크 메모리 부족시 증설
		if ((int)timeStartedAtFrame.length < Broodwar.getFrameCount() + 10)
		{
			timeStartedAtFrame = Arrays.copyOf(timeStartedAtFrame, timeStartedAtFrame.length+10000);
			timeElapsedAtFrame = Arrays.copyOf(timeElapsedAtFrame, timeElapsedAtFrame.length+10000);
		}

		// Pause 상태에서는 timeStartedAtFrame 를 계속 갱신해서, timeElapsedAtFrame 이 제대로 계산되도록 한다
		if (Broodwar.isPaused()) {
			timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
		}
		else {
			
			try {
				gameCommander.onFrame();
			} 
			catch (Exception e) {
				
				Broodwar.sendText("[Error Stack Trace]");
				System.out.println("[Error Stack Trace]");
				for (StackTraceElement ste : e.getStackTrace()) {
					Broodwar.sendText(ste.toString());
					System.out.println(ste.toString());
				}
				Broodwar.sendText("GG");

				isExceptionLostConditionSatisfied = true;
				
				exceptionLostConditionSatisfiedFrame = Broodwar.getFrameCount();
			}
			
			if (isExceptionLostConditionSatisfied) {
				MyBotModule.Broodwar.drawTextScreen(250, 100, "I lost because of EXCEPTION");

				if (MyBotModule.Broodwar.getFrameCount() - exceptionLostConditionSatisfiedFrame >= maxDurationForExceptionLostCondition) {
					MyBotModule.Broodwar.leaveGame();
				}
			}
	    }
		
		// 화면 출력 및 사용자 입력 처리
		// 빌드서버에서는 Dependency가 없는 빌드서버 전용 UXManager 를 실행시킵니다
		UXManager.Instance().update();

		checkLostConditions();

		// BasicBot 1.1 Patch End //////////////////////////////////////////////////
	}

	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 타임아웃 패배, 자동 패배 체크 추가

	/// 유닛(건물/지상유닛/공중유닛)이 Create 될 때 발생하는 이벤트를 처리합니다
	@Override
	public void onUnitCreate(Unit unit){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			gameCommander.onUnitCreate(unit);
		} 
	}

	///  유닛(건물/지상유닛/공중유닛)이 Destroy 될 때 발생하는 이벤트를 처리합니다
	@Override
	public void onUnitDestroy(Unit unit){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			gameCommander.onUnitDestroy(unit);
		}
	}

	/// 유닛(건물/지상유닛/공중유닛)이 Morph 될 때 발생하는 이벤트를 처리합니다<br>
	/// Zerg 종족의 유닛은 건물 건설이나 지상유닛/공중유닛 생산에서 거의 대부분 Morph 형태로 진행됩니다
	@Override
	public void onUnitMorph(Unit unit){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			gameCommander.onUnitMorph(unit);
		} 
	}
	
	/// 유닛(건물/지상유닛/공중유닛)의 하던 일 (건물 건설, 업그레이드, 지상유닛 훈련 등)이 끝났을 때 발생하는 이벤트를 처리합니다
	@Override
	public void onUnitComplete(Unit unit){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			gameCommander.onUnitComplete(unit);
		}
	}

	/// 유닛(건물/지상유닛/공중유닛)의 소속 플레이어가 바뀔 때 발생하는 이벤트를 처리합니다<br>
	/// Gas Geyser에 어떤 플레이어가 Refinery 건물을 건설했을 때, Refinery 건물이 파괴되었을 때, Protoss 종족 Dark Archon 의 Mind Control 에 의해 소속 플레이어가 바뀔 때 발생합니다
	@Override
	public void onUnitRenegade(Unit unit){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			gameCommander.onUnitRenegade(unit);
		}
	}

	/// 유닛(건물/지상유닛/공중유닛)이 Discover 될 때 발생하는 이벤트를 처리합니다<br>
	/// 아군 유닛이 Create 되었을 때 라든가, 적군 유닛이 Discover 되었을 때 발생합니다
	@Override
	public void onUnitDiscover(Unit unit){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			gameCommander.onUnitDiscover(unit);
		}
	}

	/// 유닛(건물/지상유닛/공중유닛)이 Evade 될 때 발생하는 이벤트를 처리합니다<br>
	/// 유닛이 Destroy 될 때 발생합니다
	@Override
	public void onUnitEvade(Unit unit){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			gameCommander.onUnitEvade(unit);
		}
	}

	/// 유닛(건물/지상유닛/공중유닛)이 Show 될 때 발생하는 이벤트를 처리합니다<br>
	/// 아군 유닛이 Create 되었을 때 라든가, 적군 유닛이 Discover 되었을 때 발생합니다
	@Override
	public void onUnitShow(Unit unit){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			gameCommander.onUnitShow(unit);
		}
	}

	/// 유닛(건물/지상유닛/공중유닛)이 Hide 될 때 발생하는 이벤트를 처리합니다<br>
	/// 보이던 유닛이 Hide 될 때 발생합니다
	@Override
	public void onUnitHide(Unit unit){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			gameCommander.onUnitHide(unit);
		}
	}

	/// 핵미사일 발사가 감지되었을 때 발생하는 이벤트를 처리합니다
	@Override
	public void onNukeDetect(Position target){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}

			// 빌드서버에서는 향후 적용
			gameCommander.onNukeDetect(target);
		}
	}

	/// 다른 플레이어가 대결을 나갔을 때 발생하는 이벤트를 처리합니다
	@Override
	public void onPlayerLeft(Player player){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}
			
			// 빌드서버에서는 향후 적용
			gameCommander.onPlayerLeft(player);
		}
	}

	/// 게임을 저장할 때 발생하는 이벤트를 처리합니다
	@Override
	public void onSaveGame(String gameName){
		if (!Broodwar.isReplay()) {
			if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
				timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
			}

			// 빌드서버에서는 향후 적용
			gameCommander.onSaveGame(gameName);
		}
	}
	

	/// 텍스트를 입력 후 엔터를 하여 다른 플레이어들에게 텍스트를 전달하려 할 때 발생하는 이벤트를 처리합니다
	@Override
	public void onSendText(String text){		
		
		if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
			timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
		}
		
		ParseTextCommand(text);
		
		gameCommander.onSendText(text);

		// Display the text to the game
		Broodwar.sendText(text);
	}

	/// 다른 플레이어로부터 텍스트를 전달받았을 때 발생하는 이벤트를 처리합니다
	@Override
	public void onReceiveText(Player player, String text){
		if (timeStartedAtFrame[Broodwar.getFrameCount()] == 0) {
			timeStartedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis();
		}
		
		Broodwar.printf(player.getName() + " said \"" + text + "\"");

		gameCommander.onReceiveText(player, text);
	}
	
	private void initializeLostConditionVariables(){
		
		timerLimits.add(55);
		timerLimitsBound.add(320);
		timerLimitsExceeded.add(0);

		timerLimits.add(1000);
		timerLimitsBound.add(10);
		timerLimitsExceeded.add(0);

		timerLimits.add(10000);
		timerLimitsBound.add(2);
		timerLimitsExceeded.add(0);
	}
	
	/// 사용자가 입력한 text 를 parse 해서 처리합니다
	public void ParseTextCommand(String commandString)
	{
		// Make sure to use %s and pass the text as a parameter,
		// otherwise you may run into problems when you use the %(percent) character!
		Player self = Broodwar.self();

		if ("afap".equals(commandString)) {
			Broodwar.setLocalSpeed(0);
			Broodwar.setFrameSkip(0);
		} else if ("fast".equals(commandString)) {
			Broodwar.setLocalSpeed(24);
			Broodwar.setFrameSkip(0);
		} else if ("slow".equals(commandString)) {
			Broodwar.setLocalSpeed(42);
			Broodwar.setFrameSkip(0);
		} else if ("endthegame".equals(commandString)) {
			// Not needed if using setGUI(false).
			Broodwar.setGUI(false);
		}

		// 1개 프레임 처리에 10초 넘게 걸리도록 하는 것을 1번 해본다
		else if ("delay12000_1".equals(commandString)) {
			isToTestTimeOut = true;
			timeOverTestFrameCount = 0;
			timeOverTestDuration = 12000;
			timeOverTestFrameCountLimit = 1;
		}
		// 1개 프레임 처리에 10초 넘게 걸리도록 하는 것을 2번 해본다
		else if ("delay12000_2".equals(commandString)) {
			isToTestTimeOut = true;
			timeOverTestFrameCount = 0;
			timeOverTestDuration = 12000;
			timeOverTestFrameCountLimit = 2;
		}
		// 1개 프레임 처리에 1초 넘게 걸리도록 하는 것을 9번 해본다
		else if ("delay1200_9".equals(commandString)) {
			isToTestTimeOut = true;
			timeOverTestFrameCount = 0;
			timeOverTestDuration = 1200;
			timeOverTestFrameCountLimit = 9;
		}
		// 1개 프레임 처리에 1초 넘게 걸리도록 하는 것을 12번 해본다
		else if ("delay1200_12".equals(commandString)) {
			isToTestTimeOut = true;
			timeOverTestFrameCount = 0;
			timeOverTestDuration = 1200;
			timeOverTestFrameCountLimit = 12;
		}
		// 1개 프레임 처리에 55 millisecond 넘게 걸리도록 하는 것을 310번 해본다
		else if ("delay70_310".equals(commandString)) {
			isToTestTimeOut = true;
			timeOverTestFrameCount = 0;
			timeOverTestDuration = 70;
			timeOverTestFrameCountLimit = 310;
		}
		// 1개 프레임 처리에 55 millisecond 넘게 걸리도록 하는 것을 330번 이상 해본다
		else if ("delay70_330".equals(commandString)) {
			isToTestTimeOut = true;
			timeOverTestFrameCount = 0;
			timeOverTestDuration = 70;
			timeOverTestFrameCountLimit = 330;
		}
	}

	private void checkLostConditions() {
		
		// 패배조건 체크
		if (isToCheckGameLostCondition) {
			checkGameLostConditionAndLeaveGame();
		}

		// 타임아웃 테스트
		if (isToTestTimeOut) {
			doTimeOutDelay();
		}

		if (isToCheckTimeOut) {
			// 타임아웃 체크
			checkTimeOutConditionAndLeaveGame();
		}
	}

	// 현재 자동 패배조건 : 생산능력을 가진 건물이 하나도 없음 && 공격능력을 가진/가질수있는 건물이 하나도 없음 && 생산/공격/특수능력을 가진 비건물 유닛이 하나도 없음
	// 토너먼트 서버에서 게임을 무의미하게 제한시간까지 플레이시키는 경우가 없도록 하기 위함임
	// 향후 추가 검토 중 : '일꾼은 있지만 커맨드센터도 없고 보유 미네랄도 없고 지도에 미네랄이 하나도 없는 경우' 처럼 게임 승리를 이끌 가능성이 현실적으로 전혀 없는 경우까지 추가 체크
	public void checkGameLostConditionAndLeaveGame()
	{
		int canProduceBuildingCount = 0;
		int canAttackBuildingCount = 0;
		int canDoSomeThingNonBuildingUnitCount = 0;

		for (Unit unit : MyBotModule.Broodwar.self().getUnits()) {
			if (unit.getType().isBuilding()) {
					
				// 생산 가능 건물이 하나라도 있으면 게임 지속 가능.
				if (unit.getType().canProduce()) {
					canProduceBuildingCount++;
					break;
				}

				// 공격 가능 건물이 하나라도 있으면 게임 지속 가능. 크립콜로니는 현재는 공격능력을 갖고있지 않지만, 향후 공격능력을 가질 수 있는 건물이므로 카운트에 포함
				if (unit.getType().canAttack() || unit.getType() == UnitType.Zerg_Creep_Colony) {
					canAttackBuildingCount++;
					break;
				}
			}
			else {
				// 생산 능력을 가진 유닛이나 공격 능력을 가진 유닛, 특수 능력을 가진 유닛이 하나라도 있으면 게임 지속 가능
				// 즉, 라바, 퀸, 디파일러, 싸이언스베쓸, 다크아칸 등은 게임 승리를 이끌 가능성이 조금이라도 있음
				// 치료, 수송, 옵저버 능력만 있는 유닛만 있으면 게임 중지.
				// 즉, 메딕, 드랍쉽, 오버로드, 옵저버, 셔틀만 존재하면, 게임 승리를 이끌 능력이 없음
				if (unit.getType().canAttack() || unit.getType().canProduce() 
					|| (unit.getType().isSpellcaster() && unit.getType() != UnitType.Terran_Medic) 
					|| unit.getType() == UnitType.Zerg_Larva
					|| unit.getType() == UnitType.Zerg_Egg || unit.getType() == UnitType.Zerg_Lurker_Egg || unit.getType() == UnitType.Zerg_Cocoon ) 
				{
					canDoSomeThingNonBuildingUnitCount++;
					break;
				}
			}
		}

		//MyBotModule.Broodwar.drawTextScreen(250, 120, "canProduce Building Count        : " + canProduceBuildingCount);
		//MyBotModule.Broodwar.drawTextScreen(250, 130, "canAttack Building Count         : " + canAttackBuildingCount);
		//MyBotModule.Broodwar.drawTextScreen(250, 140, "canDoSomeThing NonBuilding Count : " + canDoSomeThingNonBuildingUnitCount);

		// 자동 패배조건 만족하게 된 프레임 기록
		if (canDoSomeThingNonBuildingUnitCount == 0 && canProduceBuildingCount == 0 && canAttackBuildingCount == 0 && isGameLostConditionSatisfied == false) {
			MyBotModule.Broodwar.sendText("I lost because I HAVE NO UNIT TO DEFEAT ENEMY PLAYER");
			MyBotModule.Broodwar.sendText("GG");
			System.out.println("I lost because I HAVE NO UNIT TO DEFEAT ENEMY PLAYER");

			isGameLostConditionSatisfied = true;
			gameLostConditionSatisfiedFrame = MyBotModule.Broodwar.getFrameCount();
		}
		// 자동 패배조건 벗어나게 되면 리셋
		else if (canDoSomeThingNonBuildingUnitCount != 0 || canProduceBuildingCount != 0 || canAttackBuildingCount != 0) {
			isGameLostConditionSatisfied = false;
		}

		// 자동 패배조건 만족 상황이 일정시간 동안 지속되었으면 게임 패배로 처리
		if (isGameLostConditionSatisfied) {

			MyBotModule.Broodwar.drawTextScreen(250, 100, "I lost because I HAVE NO UNIT TO DEFEAT ENEMY PLAYER");
			MyBotModule.Broodwar.drawTextScreen(250, 115, "I will leave game in " 
					+ (maxDurationForGameLostCondition - (MyBotModule.Broodwar.getFrameCount() - gameLostConditionSatisfiedFrame)) 
					+ " frames");

			if (MyBotModule.Broodwar.getFrameCount() - gameLostConditionSatisfiedFrame >= maxDurationForGameLostCondition) {
				MyBotModule.Broodwar.leaveGame();
			}
		}
	}



	void doTimeOutDelay() {
		if (timeOverTestFrameCount < timeOverTestFrameCountLimit) {

			// 10 프레임마다 1번씩 타임 딜레이를 실행한다
			if (Broodwar.getFrameCount() % 10 == 0) {
				long startTime = System.currentTimeMillis();
				while (System.currentTimeMillis() - startTime < timeOverTestDuration) {
				}
				timeOverTestFrameCount++;
			}
		}
		else {
			isToTestTimeOut = false;
		}	
	}

	/// 타임 아웃 조건을 체크하여, 조건 만족 시 GG 선언하고 게임을 나갑니다
	void checkTimeOutConditionAndLeaveGame(){
		if (Broodwar.getFrameCount() >= 10) {

			// 현재 프레임에서의 소요시간 기록
			timeElapsedAtFrame[Broodwar.getFrameCount()] = System.currentTimeMillis()
					- timeStartedAtFrame[Broodwar.getFrameCount()];

			long timeElapsedAtLastFrame = timeElapsedAtFrame[Broodwar.getFrameCount() - 1];

			// 현재 시각 표시
			Broodwar.drawTextScreen(260, 5, "FrameCount :");
			Broodwar.drawTextScreen(340, 5, ""+ Broodwar.getFrameCount());
			Broodwar.drawTextScreen(370, 5, "("
					+(int)(Broodwar.getFrameCount() / (23.8 * 60))
					+":"
					+(int)((int)(Broodwar.getFrameCount() / 23.8) % 60)
					+")"
			);
							
			// 타임아웃 체크 현황을 화면에 표시
			int y = 15;
			for (int t = 0; t < timerLimits.size(); ++t)
			{
				Broodwar.drawTextScreen(260, y, "> "+timerLimits.get(t)+" ms : "+timerLimitsExceeded.get(t)+" / "+timerLimitsBound.get(t));
				y += 10;
			}

			/*
			// 각 프레임별 소요시간 기록을 화면에 표시
			y = 100;
			for (int i = Broodwar.getFrameCount() - 9; i < Broodwar.getFrameCount(); ++i)
			{
				Broodwar.drawTextScreen(260, y, "["+i+"] : "+timeStartedAtFrame[i]+" "+timeElapsedAtFrame[i]+" ms");
				y += 10;
			}

			// 최대 많은 시간을 소요한 프레임을 화면에 표시
			y = 220;
			int maxI = 0;
			for (int i = 0; i < Broodwar.getFrameCount(); ++i)
			{
				if (timeElapsedAtFrame[maxI] < timeElapsedAtFrame[i]) {
					maxI = i;
				}
			}
			Broodwar.drawTextScreen(260, y, "["+maxI+"] : "+timeStartedAtFrame[maxI]+" "+timeElapsedAtFrame[maxI]+" ms");
			*/

			// 타임아웃 체크 및 게임 종료
			for (int t = 0; t < timerLimits.size(); ++t)
			{
				if (timeElapsedAtLastFrame > timerLimits.get(t))
				{
					timerLimitsExceeded.set(t, timerLimitsExceeded.get(t)+1);

					if (timerLimitsExceeded.get(t).equals(timerLimitsBound.get(t)))
					{
						isTimeOutConditionSatisfied = true;
						timeOutConditionSatisfiedFrame = Broodwar.getFrameCount();
						
						Broodwar.sendText("I lost because of TIMEOUT ("+timerLimitsBound.get(t)+" frames exceed "+timerLimits.get(t)+" ms/frame)");
						System.out.println("I lost because of TIMEOUT ("+timerLimitsBound.get(t)+" frames exceed "+timerLimits.get(t)+" ms/frame)");
						Broodwar.sendText("GG");
					}
				}
			}
			
			// 자동 패배조건 만족 상황이 일정시간 동안 지속되었으면 게임 패배로 처리
			if (isTimeOutConditionSatisfied) {

				MyBotModule.Broodwar.drawTextScreen(250, 100, "I lost because of TIMEOUT");

				if (MyBotModule.Broodwar.getFrameCount() - timeOutConditionSatisfiedFrame >= maxDurationForTimeOutLostCondition) {

					// 빌드서버에서는 실제로 게임을 종료시켜버립니다
					//MyBotModule.Broodwar.leaveGame();
				}
			}
		}

	}

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////

	
}