import java.util.ArrayList;
import java.util.Deque;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.Vector;

import bwapi.Bullet;
import bwapi.BulletType;
import bwapi.Color;
import bwapi.Force;
import bwapi.Player;
import bwapi.Position;
import bwapi.TilePosition;
import bwapi.Unit;
import bwapi.UnitType;
import bwta.BWTA;
import bwta.BaseLocation;
import bwta.Chokepoint;
import bwta.Polygon;
import bwta.Region;

/// 봇 프로그램 개발의 편의성 향상을 위해 게임 화면에 추가 정보들을 표시하는 class<br>
/// 여러 Manager 들로부터 정보를 조회하여 Screen 혹은 Map 에 정보를 표시합니다
public class UXManager {

	private final Character brown = '';
	private final char red = '';
	private final char teal = '';
//	private final char blue = '';
	private final char purple = '';
	private final char white = '';
	
	private boolean hasSavedBWTAInfo = false;
	private int[][] blue = null;
	private int[][] cyan = null;
	private int[][] orange = null;
	private List<Position> yellow = new ArrayList<Position>();
	private List<Position> green1 = new ArrayList<Position>();
	private List<Position> green2 = new ArrayList<Position>();
	private List<Position> red1 = new ArrayList<Position>();
	private List<Position> red2 = new ArrayList<Position>();
	
	private final int dotRadius = 2;
	
	private String bulletTypeName = "";
	private String tempUnitName = "";
	
	private static UXManager instance = new UXManager();
	
	/// static singleton 객체를 리턴합니다
	public static UXManager Instance() {
		return instance;
	}
	
	/// 경기가 시작될 때 일회적으로 추가 정보를 출력합니다
	public void onStart() {
	}

	/// 경기 진행 중 매 프레임마다 추가 정보를 출력하고 사용자 입력을 처리합니다
	public void update() {
		drawGameInformationOnScreen(5, 5);

		if (Config.DrawEnemyUnitInfo) {
			drawUnitStatisticsOnScreen(400, 20);
		}

		if (Config.DrawBWTAInfo) {
			drawBWTAResultOnMap();
		}

		if (Config.DrawMapGrid) {
			drawMapGrid();
		}

		// 빌드오더큐 : 빌드 실행 전
		if (Config.DrawProductionInfo) {
			drawBuildOrderQueueOnScreen(80, 60);
		}

		// 빌드 실행 상황 : 건물 건설, 유닛 생산, 업그레이드, 리서치
		if (Config.DrawProductionInfo) {
			drawBuildStatusOnScreen(200, 60);
		}

		// 건물 건설 큐. 건물 건설 상황
		if (Config.DrawBuildingInfo) {
			drawConstructionQueueOnScreenAndMap(200, 150);
		}

		// 건물이 건설될 위치
		if (Config.DrawReservedBuildingTiles) {
			// 건물 건설 장소 예약 지점
			drawReservedBuildingTilesOnMap();
			// 건물 건설 불가 구역 (미네랄/가스/베이스 사이)
			drawTilesToAvoidOnMap();
		}

		if (Config.DrawUnitHealthBars) {
			drawUnitExtendedInformationOnMap();
			drawUnitIdOnMap();
		}

		if (Config.DrawWorkerInfo) {
			// 각 일꾼들의 임무 상황
			drawWorkerStateOnScreen(5, 60);

			// 베이스캠프당 일꾼 수
			drawWorkerCountOnMap();
		}

		// 일꾼 자원채취 임무 상황
		if (Config.DrawResourceInfo) {
			drawWorkerMiningStatusOnMap();
		}

		// 정찰
		if (Config.DrawScoutInfo) {
			drawScoutInformation(220,330);
		}

		// 공격
		if (Config.DrawUnitTargetInfo) {
			drawUnitTargetOnMap();

			// 미사일, 럴커의 보이지않는 공격등을 표시
			drawBulletsOnMap();
		}
		
		// draw tile position of mouse cursor
		if (Config.DrawMouseCursorInfo) {
			int mouseX = MyBotModule.Broodwar.getMousePosition().getX() + MyBotModule.Broodwar.getScreenPosition().getX();
			int mouseY = MyBotModule.Broodwar.getMousePosition().getY() + MyBotModule.Broodwar.getScreenPosition().getY();
			MyBotModule.Broodwar.drawTextMap(mouseX + 20, mouseY, "(" + (int)(mouseX/Config.TILE_SIZE) + ", " +  (int)(mouseY/Config.TILE_SIZE) + ")");
		}

	}

	// 게임 개요 정보를 Screen 에 표시합니다
	public void drawGameInformationOnScreen(int x, int y) {
		MyBotModule.Broodwar.drawTextScreen(x, y, white + "Players : ");
		MyBotModule.Broodwar.drawTextScreen(x + 50, y, MyBotModule.Broodwar.self().getTextColor() + MyBotModule.Broodwar.self().getName() + "(" + InformationManager.Instance().selfRace + ") " + white + " vs.  " + 
				InformationManager.Instance().enemyPlayer.getTextColor() + InformationManager.Instance().enemyPlayer.getName() + "(" + InformationManager.Instance().enemyRace + ")");
		y += 12;

		MyBotModule.Broodwar.drawTextScreen(x, y, white + "Map : ");
		MyBotModule.Broodwar.drawTextScreen(x + 50, y, white + MyBotModule.Broodwar.mapFileName() + " (" + MyBotModule.Broodwar.mapWidth() + " x " +  MyBotModule.Broodwar.mapHeight() + " size)");
		MyBotModule.Broodwar.setTextSize();
		y += 12;

		MyBotModule.Broodwar.drawTextScreen(x, y, white + "Time : ");
		MyBotModule.Broodwar.drawTextScreen(x + 50, y, "" + white + MyBotModule.Broodwar.getFrameCount());
		MyBotModule.Broodwar.drawTextScreen(x + 90, y, "" + white + (int)(MyBotModule.Broodwar.getFrameCount() / (23.8 * 60)) + ":" + (int)((int)(MyBotModule.Broodwar.getFrameCount() / 23.8) % 60));
	}

	/// APM (Action Per Minute) 숫자를 Screen 에 표시합니다
	public void drawAPM(int x, int y) {
		int bwapiAPM = MyBotModule.Broodwar.getAPM();
		MyBotModule.Broodwar.drawTextScreen(x, y, "APM : " + bwapiAPM);
	}

	/// Players 정보를 Screen 에 표시합니다
	public void drawPlayers() {
		for (Player p : MyBotModule.Broodwar.getPlayers()) {
			MyBotModule.Broodwar.sendText("Player [" + p.getID() + "]: " + p.getName() + " is in force: " + p.getForce().getName());
		}
	}

	/// Player 들의 팀 (Force) 들의 정보를 Screen 에 표시합니다
	public void drawForces() {
		for (Force f :  MyBotModule.Broodwar.getForces()) {
			MyBotModule.Broodwar.sendText("Force " + f.getName() + " has the following players:");
			for (Player p : f.getPlayers()) {
				MyBotModule.Broodwar.sendText("  - Player [" + p.getID() + "]: " + p.getName());
			}
		}
	}

	/// Unit 의 HitPoint 등 추가 정보를 Map 에 표시합니다
	public void drawUnitExtendedInformationOnMap() {
		int verticalOffset = -10;

		if(InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer) != null)
		{
			// draw enemy units
			Iterator<Integer> it = InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getUnitAndUnitInfoMap().keySet().iterator();
			
			// C++ : for (final Unit kv : InformationManager.Instance().getUnitData(MyBotModule.game.enemy()).getUnits())
			while(it.hasNext())
			{
				final UnitInfo ui= InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getUnitAndUnitInfoMap().get(it.next());
	
				UnitType type = ui.getType();
				int hitPoints = ui.getLastHealth();
				int shields = ui.getLastShields();
	
				Position pos = ui.getLastPosition();
	
				int left = pos.getX() - type.dimensionLeft();
				int right = pos.getX() + type.dimensionRight();
				int top = pos.getY() - type.dimensionUp();
				int bottom = pos.getY() + type.dimensionDown();
	
				// 적 유닛이면 주위에 박스 표시
				if (!MyBotModule.Broodwar.isVisible(ui.getLastPosition().toTilePosition())) {
					MyBotModule.Broodwar.drawBoxMap(new Position(left, top), new Position(right, bottom), Color.Grey, false);
					MyBotModule.Broodwar.drawTextMap(new Position(left + 3, top + 4), ui.getType().toString());
				}
	
				// 유닛의 HitPoint 남아있는 비율 표시
				if (!type.isResourceContainer() && type.maxHitPoints() > 0)
				{
					double hpRatio = (double)hitPoints / (double)type.maxHitPoints();
	
					Color hpColor = Color.Green;
					if (hpRatio < 0.66) hpColor = Color.Orange;
					if (hpRatio < 0.33) hpColor = Color.Red;
	
					int ratioRight = left + (int)((right - left) * hpRatio);
					int hpTop = top + verticalOffset;
					int hpBottom = top + 4 + verticalOffset;
	
					MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), Color.Grey, true);
					MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(ratioRight, hpBottom), hpColor, true);
					MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), Color.Black, false);
	
					int ticWidth = 3;
	
					for (int i = left; i < right - 1; i += ticWidth) {
						MyBotModule.Broodwar.drawLineMap(new Position(i, hpTop), new Position(i, hpBottom), Color.Black);
					}
				}
	
				// 유닛의 Shield 남아있는 비율 표시
				if (!type.isResourceContainer() && type.maxShields() > 0) {
					double shieldRatio = (double)shields / (double)type.maxShields();
	
					int ratioRight = left + (int)((right - left) * shieldRatio);
					int hpTop = top - 3 + verticalOffset;
					int hpBottom = top + 1 + verticalOffset;
	
					MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), Color.Grey, true);
					MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(ratioRight, hpBottom), Color.Blue, true);
					MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), Color.Black, false);
	
					int ticWidth = 3;
	
					for (int i = left; i < right - 1; i += ticWidth) {
						MyBotModule.Broodwar.drawLineMap(new Position(i, hpTop), new Position(i, hpBottom), Color.Black);
					}
				}
			}
		}

		// draw neutral units and our units
		for (Unit unit : MyBotModule.Broodwar.getAllUnits()) {
			if (unit.getPlayer() == InformationManager.Instance().enemyPlayer) {
				continue;
			}

			final Position pos = unit.getPosition();

			int left = pos.getX() - unit.getType().dimensionLeft();
			int right = pos.getX() + unit.getType().dimensionRight();
			int top = pos.getY() - unit.getType().dimensionUp();
			int bottom = pos.getY() + unit.getType().dimensionDown();

			//MyBotModule.game.drawBoxMap(BWAPI.Position(left, top), BWAPI.Position(right, bottom), Color.Grey, false);

			// 유닛의 HitPoint 남아있는 비율 표시
			if (!unit.getType().isResourceContainer() && unit.getType().maxHitPoints() > 0) {
				double hpRatio = (double)unit.getHitPoints() / (double)unit.getType().maxHitPoints();

				Color hpColor = Color.Green;
				if (hpRatio < 0.66) hpColor = Color.Orange;
				if (hpRatio < 0.33) hpColor = Color.Red;

				int ratioRight = left + (int)((right - left) * hpRatio);
				int hpTop = top + verticalOffset;
				int hpBottom = top + 4 + verticalOffset;

				MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), Color.Grey, true);
				MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(ratioRight, hpBottom), hpColor, true);
				MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), hpColor.Black, false);

				int ticWidth = 3;

				for (int i = left; i < right - 1; i += ticWidth) {
					MyBotModule.Broodwar.drawLineMap(new Position(i, hpTop), new Position(i, hpBottom), Color.Black);
				}
			}

			// 유닛의 Shield 남아있는 비율 표시
			if (!unit.getType().isResourceContainer() && unit.getType().maxShields() > 0) {
				double shieldRatio = (double)unit.getShields() / (double)unit.getType().maxShields();

				int ratioRight = left + (int)((right - left) * shieldRatio);
				int hpTop = top - 3 + verticalOffset;
				int hpBottom = top + 1 + verticalOffset;

				MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), Color.Grey, true);
				MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(ratioRight, hpBottom), Color.Blue, true);
				MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), Color.Black, false);

				int ticWidth = 3;

				for (int i = left; i < right - 1; i += ticWidth) {
					MyBotModule.Broodwar.drawLineMap(new Position(i, hpTop), new Position(i, hpBottom), Color.Black);
				}
			}

			// Mineral / Gas 가 얼마나 남아있는가
			if (unit.getType().isResourceContainer() && unit.getInitialResources() > 0) {
				double mineralRatio = (double)unit.getResources() / (double)unit.getInitialResources();

				int ratioRight = left + (int)((right - left) * mineralRatio);
				int hpTop = top + verticalOffset;
				int hpBottom = top + 4 + verticalOffset;

				MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), Color.Grey, true);
				MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(ratioRight, hpBottom), Color.Cyan, true);
				MyBotModule.Broodwar.drawBoxMap(new Position(left, hpTop), new Position(right, hpBottom), Color.Black, false);

				int ticWidth = 3;

				for (int i = left; i < right - 1; i += ticWidth) {
					MyBotModule.Broodwar.drawLineMap(new Position(i, hpTop), new Position(i, hpBottom), Color.Black);
				}
			}
		}
	}

	/// UnitType 별 통계 정보를 Screen 에 표시합니다
	public void drawUnitStatisticsOnScreen(int x, int y) {
		int currentY = y;

		// 아군이 입은 피해 누적값
		MyBotModule.Broodwar.drawTextScreen(x, currentY, white + " Self Loss:" + white + " Minerals: " + brown + InformationManager.Instance().getUnitData(MyBotModule.Broodwar.self()).getMineralsLost() + white + " Gas: " + red + InformationManager.Instance().getUnitData(MyBotModule.Broodwar.self()).getGasLost());
		currentY += 10;

		// 아군 모든 유닛 숫자 합계
		//MyBotModule.Broodwar.drawTextScreen(x, currentY,  white + " allUnitCount: " + MyBotModule.Broodwar.self().allUnitCount(UnitType.AllUnits));
		//currentY += 10;

		// 아군 건설/훈련 완료한 유닛 숫자 합계
		//MyBotModule.Broodwar.drawTextScreen(x, currentY,  white + " completedUnitCount: " + MyBotModule.Broodwar.self().completedUnitCount(UnitType.AllUnits));
		//currentY += 10;

		// 아군 건설/훈련중인 유닛 숫자 합계
		//MyBotModule.Broodwar.drawTextScreen(x, currentY,  white + " incompleteUnitCount: " + MyBotModule.Broodwar.self().incompleteUnitCount(UnitType.AllUnits));
		//currentY += 10;

		// 아군 유닛 파괴/사망 숫자 누적값
		//MyBotModule.Broodwar.drawTextScreen(x, currentY,  white + " deadUnitCount: " + MyBotModule.Broodwar.self().deadUnitCount(UnitType.AllUnits));
		//currentY += 10;

		// 상대방 유닛을 파괴/사망 시킨 숫자 누적값
		//MyBotModule.Broodwar.drawTextScreen(x, currentY,  white + " killedUnitCount: " + MyBotModule.Broodwar.self().killedUnitCount(UnitType.AllUnits));
		//currentY += 10;

		//MyBotModule.Broodwar.drawTextScreen(x, currentY,  white + " UnitScore: " + MyBotModule.Broodwar.self().getUnitScore());
		//currentY += 10;
		//MyBotModule.Broodwar.drawTextScreen(x, currentY,  white + " RazingScore: " + MyBotModule.Broodwar.self().getRazingScore());
		//currentY += 10;
		//MyBotModule.Broodwar.drawTextScreen(x, currentY,  white + " BuildingScore: " + MyBotModule.Broodwar.self().getBuildingScore());
		//currentY += 10;
		//MyBotModule.Broodwar.drawTextScreen(x, currentY,  white + " KillScore: " + MyBotModule.Broodwar.self().getKillScore());
		//currentY += 10;

		// 적군이 입은 피해 누적값
		if(InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer) != null)
		{
			MyBotModule.Broodwar.drawTextScreen(x, currentY, brown + " Enemy Loss:" + white +" Minerals: " + red + InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getMineralsLost() + white + " Gas: " + teal + InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getGasLost());
		}
			
		// 적군의 UnitType 별 파악된 Unit 숫자를 표시
		MyBotModule.Broodwar.drawTextScreen(x,		 currentY + 20, white + " UNIT NAME");
		MyBotModule.Broodwar.drawTextScreen(x + 110, currentY + 20, white + " Created");
		MyBotModule.Broodwar.drawTextScreen(x + 150, currentY + 20, white + " Dead");
		MyBotModule.Broodwar.drawTextScreen(x + 190, currentY + 20, white + " Alive");

		int yspace = 0;
		
		Set<String> allUnit = new HashSet<String>();
		Iterator<String> it = null;
		if(InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer) != null)
		{
			it = InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getNumCreatedUnits().keySet().iterator();
			while(it.hasNext())
			{
				String unit = it.next();
				allUnit.add(unit);
			}
			it = InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getNumDeadUnits().keySet().iterator();
			while(it.hasNext())
			{
				String unit = it.next();
				allUnit.add(unit);
			}
			it = InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getNumUnits().keySet().iterator();
			while(it.hasNext())
			{
				String unit = it.next();
				allUnit.add(unit);
			}
			
			it = allUnit.iterator();
			// for (UnitType t : UnitType.allUnitTypes())
			while(it.hasNext())
			{
				tempUnitName = it.next();
				int numCreatedUnits = InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getNumCreatedUnits(tempUnitName);
				int numDeadUnits = InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getNumDeadUnits(tempUnitName);
				int numUnits = InformationManager.Instance().getUnitData(InformationManager.Instance().enemyPlayer).getNumUnits(tempUnitName);
	
				if (numUnits > 0)
				{
					MyBotModule.Broodwar.drawTextScreen(x,		 currentY + 30 + ((yspace)* 10), tempUnitName);
					MyBotModule.Broodwar.drawTextScreen(x + 120, currentY + 30 + ((yspace)* 10), "" + numCreatedUnits);
					MyBotModule.Broodwar.drawTextScreen(x + 160, currentY + 30 + ((yspace)* 10), "" + numDeadUnits);
					MyBotModule.Broodwar.drawTextScreen(x + 200, currentY + 30 + ((yspace)* 10), "" + numUnits);
					yspace++;
				}
			}
		}
		
		yspace++;

		// 아군의 UnitType 별 파악된 Unit 숫자를 표시
		allUnit = new HashSet<String>();
		it = null;
		if(InformationManager.Instance().getUnitData(InformationManager.Instance().selfPlayer) != null)
		{
			it = InformationManager.Instance().getUnitData(InformationManager.Instance().selfPlayer).getNumCreatedUnits().keySet().iterator();
			while(it.hasNext())
			{
				String unit = it.next();
				allUnit.add(unit);
			}
			it = InformationManager.Instance().getUnitData(InformationManager.Instance().selfPlayer).getNumDeadUnits().keySet().iterator();
			while(it.hasNext())
			{
				String unit = it.next();
				allUnit.add(unit);
			}
			it = InformationManager.Instance().getUnitData(InformationManager.Instance().selfPlayer).getNumUnits().keySet().iterator();
			while(it.hasNext())
			{
				String unit = it.next();
				allUnit.add(unit);
			}
			
			it = allUnit.iterator();
			// for (UnitType t : UnitType.allUnitTypes())
			while(it.hasNext())
			{
				tempUnitName = it.next();
				int numCreatedUnits = InformationManager.Instance().getUnitData(InformationManager.Instance().selfPlayer).getNumCreatedUnits(tempUnitName);
				int numDeadUnits = InformationManager.Instance().getUnitData(InformationManager.Instance().selfPlayer).getNumDeadUnits(tempUnitName);
				int numUnits = InformationManager.Instance().getUnitData(InformationManager.Instance().selfPlayer).getNumUnits(tempUnitName);
	
				if (numUnits > 0)
				{
					MyBotModule.Broodwar.drawTextScreen(x,		 currentY + 30 + ((yspace)* 10), tempUnitName);
					MyBotModule.Broodwar.drawTextScreen(x + 120, currentY + 30 + ((yspace)* 10), "" + numCreatedUnits);
					MyBotModule.Broodwar.drawTextScreen(x + 160, currentY + 30 + ((yspace)* 10), "" + numDeadUnits);
					MyBotModule.Broodwar.drawTextScreen(x + 200, currentY + 30 + ((yspace)* 10), "" + numUnits);
					yspace++;
				}
			}
		}
	}

	/// BWTA 라이브러리에 의한 Map 분석 결과 정보를 Map 에 표시합니다
	public void drawBWTAResultOnMap() {
		/*//we will iterate through all the base locations, and draw their outlines.
		// C+ . for (std.set<BWTA.BaseLocation*>.const_iterator i = BWTA.getBaseLocations().begin(); i != BWTA.getBaseLocations().end(); i++)
		for(BaseLocation baseLocation : BWTA.getBaseLocations())
		{
			TilePosition p = baseLocation.getTilePosition();
			Position c = baseLocation.getPosition();

			//draw outline of Base location 
			MyBotModule.Broodwar.drawBoxMap(p.getX() * 32, p.getY() * 32, p.getX() * 32 + 4 * 32, p.getY() * 32 + 3 * 32, Color.Blue);

			//draw a circle at each mineral patch
			// C++ : for (BWAPI.Unitset.iterator j = (*i).getStaticMinerals().begin(); j != (*i).getStaticMinerals().end(); j++)
			for(Unit unit : baseLocation.getStaticMinerals())
			{
				Position q = unit.getInitialPosition();
				MyBotModule.Broodwar.drawCircleMap(q.getX(), q.getY(), 30, Color.Cyan);
			}

			//draw the outlines of vespene geysers
			// C++ : for (BWAPI.Unitset.iterator j = (*i).getGeysers().begin(); j != (*i).getGeysers().end(); j++)
			for(Unit unit :baseLocation.getGeysers() )
			{
				TilePosition q = unit.getInitialTilePosition();
				MyBotModule.Broodwar.drawBoxMap(q.getX() * 32, q.getY() * 32, q.getX() * 32 + 4 * 32, q.getY() * 32 + 2 * 32, Color.Orange);
			}

			//if this is an island expansion, draw a yellow circle around the base location
			if (baseLocation.isIsland())
			{
				MyBotModule.Broodwar.drawCircleMap(c, 80, Color.Yellow);
			}
		}

		//we will iterate through all the regions and draw the polygon outline of it in green.
		// C++ : for (std.set<BWTA.Region*>.const_iterator r = BWTA.getRegions().begin(); r != BWTA.getRegions().end(); r++)
		for(Region region : BWTA.getRegions())
		{
			Polygon p = region.getPolygon();
			for (int j = 0; j<p.getPoints().size(); j++)
			{
				Position point1 = p.getPoints().get(j);
				Position point2 = p.getPoints().get((j + 1) % p.getPoints().size());
				MyBotModule.Broodwar.drawLineMap(point1, point2, Color.Green);
			}
		}

		//we will visualize the chokepoints with red lines
		// C++ : for (std.set<BWTA.Region*>.const_iterator r = BWTA.getRegions().begin(); r != BWTA.getRegions().end(); r++)
		for(Region region : BWTA.getRegions())
		{
			// C++ : for (std.set<BWTA.Chokepoint*>.const_iterator c = (*r).getChokepoints().begin(); c != (*r).getChokepoints().end(); c++)
			for(Chokepoint Chokepoint : region.getChokepoints())
			{
				Position point1 = Chokepoint.getSides().first;
				Position point2 = Chokepoint.getSides().second;
				MyBotModule.Broodwar.drawLineMap(point1, point2, Color.Red);
			}
		}*/
		int blueCount = 0;
		int cyanCount = 0;
		int orangeCount = 0;
		
		if(hasSavedBWTAInfo == false)
		{
			for(BaseLocation baseLocation : BWTA.getBaseLocations())
			{
				blueCount++;
				for(Unit unit : baseLocation.getStaticMinerals())
				{
					cyanCount++;
				}
				for(Unit unit :baseLocation.getGeysers() )
				{
					orangeCount++;
				}
			}
			
			blue = new int[blueCount][4];
			int blueIndex = 0;
			cyan = new int[cyanCount][2];
			int cyanIndex = 0;
			orange = new int[orangeCount][4];
			int orangeIndex = 0;
			
			for(BaseLocation baseLocation : BWTA.getBaseLocations())
			{
				TilePosition p = baseLocation.getTilePosition();
				Position c = baseLocation.getPosition();
				
				blue[blueIndex][0] = p.getX() * 32;
				blue[blueIndex][1] = p.getY() * 32;
				blue[blueIndex][2] = p.getX() * 32 + 4 * 32;
				blue[blueIndex][3] = p.getY() * 32 + 3 * 32;
				blueIndex++;
				
				//draw a circle at each mineral patch
				// C++ : for (BWAPI.Unitset.iterator j = (*i).getStaticMinerals().begin(); j != (*i).getStaticMinerals().end(); j++)
				for(Unit unit : baseLocation.getStaticMinerals())
				{
					Position q = unit.getInitialPosition();
					cyan[cyanIndex][0] = q.getX();
					cyan[cyanIndex][1] = q.getY();
					cyanIndex++;
				}

				//draw the outlines of vespene geysers
				// C++ : for (BWAPI.Unitset.iterator j = (*i).getGeysers().begin(); j != (*i).getGeysers().end(); j++)
				for(Unit unit :baseLocation.getGeysers() )
				{
					TilePosition q = unit.getInitialTilePosition();
					orange[orangeIndex][0] = q.getX() * 32;
					orange[orangeIndex][1] = q.getY() * 32;
					orange[orangeIndex][2] = q.getX() * 32 + 4 * 32;
					orange[orangeIndex][3] = q.getY() * 32 + 2 * 32;
					orangeIndex++;
				}

				//if this is an island expansion, draw a yellow circle around the base location
				if (baseLocation.isIsland())
				{
					yellow.add(c);
				}
			}

			//we will iterate through all the regions and draw the polygon outline of it in green.
			// C++ : for (std.set<BWTA.Region*>.const_iterator r = BWTA.getRegions().begin(); r != BWTA.getRegions().end(); r++)
			for(Region region : BWTA.getRegions())
			{
				Polygon p = region.getPolygon();
				for (int j = 0; j<p.getPoints().size(); j++)
				{
					green1.add(p.getPoints().get(j));
					green2.add(p.getPoints().get((j + 1) % p.getPoints().size()));
				}
			}

			//we will visualize the chokepoints with red lines
			// C++ : for (std.set<BWTA.Region*>.const_iterator r = BWTA.getRegions().begin(); r != BWTA.getRegions().end(); r++)
			for(Region region : BWTA.getRegions())
			{
				// C++ : for (std.set<BWTA.Chokepoint*>.const_iterator c = (*r).getChokepoints().begin(); c != (*r).getChokepoints().end(); c++)
				for(Chokepoint Chokepoint : region.getChokepoints())
				{
					red1.add(Chokepoint.getSides().first);
					red2.add(Chokepoint.getSides().second);
				}
			}
			hasSavedBWTAInfo = true;
			
//			System.out.println(blueCount + " " + cyanCount + " " + orangeCount + " " + yellowCount + " " + greenCount + " " + redCount);
		}

		if(hasSavedBWTAInfo)
		{
			for(int i1=0 ; i1<blue.length ; i1++)
			{
				MyBotModule.Broodwar.drawBoxMap(blue[i1][0], blue[i1][1], blue[i1][2], blue[i1][3], Color.Blue);
			}
			for(int i2=0 ; i2<cyan.length ; i2++)
			{
				MyBotModule.Broodwar.drawCircleMap(cyan[i2][0], cyan[i2][1], 30, Color.Cyan);	
			}
			for(int i3=0 ; i3<orange.length ; i3++)
			{
				MyBotModule.Broodwar.drawBoxMap(orange[i3][0], orange[i3][1], orange[i3][2], orange[i3][3], Color.Orange);
			}
			for(int i4=0 ; i4<yellow.size() ; i4++)
			{
				MyBotModule.Broodwar.drawCircleMap(yellow.get(i4), 80, Color.Yellow);	
			}
			for(int i5=0 ; i5<green1.size() ; i5++)
			{
				MyBotModule.Broodwar.drawLineMap(green1.get(i5), green2.get(i5), Color.Green);	
			}
			for(int i6=0 ; i6<red1.size() ; i6++)
			{
				MyBotModule.Broodwar.drawLineMap(red1.get(i6), red2.get(i6), Color.Red);	
			}			

			// OccupiedBaseLocation 을 원으로 표시
			for (BaseLocation baseLocation : InformationManager.Instance().getOccupiedBaseLocations(InformationManager.Instance().selfPlayer)) {
				MyBotModule.Broodwar.drawCircleMap(baseLocation.getPosition(), 10 * Config.TILE_SIZE, Color.Blue);	
			}
			for (BaseLocation baseLocation : InformationManager.Instance().getOccupiedBaseLocations(InformationManager.Instance().enemyPlayer)) {
				MyBotModule.Broodwar.drawCircleMap(baseLocation.getPosition(), 10 * Config.TILE_SIZE, Color.Red);	
			}

			// ChokePoint, BaseLocation 을 텍스트로 표시
			if (InformationManager.Instance().getFirstChokePoint(MyBotModule.Broodwar.self()) != null) {
				MyBotModule.Broodwar.drawTextMap(InformationManager.Instance().getMainBaseLocation(MyBotModule.Broodwar.self()).getPosition(), "My MainBaseLocation");
			}
			if (InformationManager.Instance().getFirstChokePoint(MyBotModule.Broodwar.self()) != null) {
				MyBotModule.Broodwar.drawTextMap(InformationManager.Instance().getFirstChokePoint(MyBotModule.Broodwar.self()).getCenter(), "My First ChokePoint");
			}
			if (InformationManager.Instance().getSecondChokePoint(MyBotModule.Broodwar.self()) != null) {
				MyBotModule.Broodwar.drawTextMap(InformationManager.Instance().getSecondChokePoint(MyBotModule.Broodwar.self()).getCenter(), "My Second ChokePoint");
			}
			if (InformationManager.Instance().getFirstExpansionLocation(MyBotModule.Broodwar.self()) != null) {
				MyBotModule.Broodwar.drawTextMap(InformationManager.Instance().getFirstExpansionLocation(MyBotModule.Broodwar.self()).getPosition(), "My First ExpansionLocation");
			}

			if (InformationManager.Instance().getFirstChokePoint(InformationManager.Instance().enemyPlayer) != null) {
				MyBotModule.Broodwar.drawTextMap(InformationManager.Instance().getMainBaseLocation(InformationManager.Instance().enemyPlayer).getPosition(), "Enemy MainBaseLocation");
			}
			if (InformationManager.Instance().getFirstChokePoint(InformationManager.Instance().enemyPlayer) != null) {
				MyBotModule.Broodwar.drawTextMap(InformationManager.Instance().getFirstChokePoint(InformationManager.Instance().enemyPlayer).getCenter(), "Enemy First ChokePoint");
			}
			if (InformationManager.Instance().getSecondChokePoint(InformationManager.Instance().enemyPlayer) != null) {
				MyBotModule.Broodwar.drawTextMap(InformationManager.Instance().getSecondChokePoint(InformationManager.Instance().enemyPlayer).getCenter(), "Enemy Second ChokePoint");
			}
			if (InformationManager.Instance().getFirstExpansionLocation(InformationManager.Instance().enemyPlayer) != null) {
				MyBotModule.Broodwar.drawTextMap(InformationManager.Instance().getFirstExpansionLocation(InformationManager.Instance().enemyPlayer).getPosition(), "Enemy First ExpansionLocation");
			}
			
		}
	}

	/// Tile Position 그리드를 Map 에 표시합니다
	public void drawMapGrid() {
		int	cellSize = MapGrid.Instance().getCellSize();
		int	mapWidth = MapGrid.Instance().getMapWidth();
		int mapHeight = MapGrid.Instance().getMapHeight();
		int	rows = MapGrid.Instance().getRows();
		int	cols = MapGrid.Instance().getCols();
		
		for (int i = 0; i<cols; i++) {
			MyBotModule.Broodwar.drawLineMap(i*cellSize, 0, i*cellSize, mapHeight, Color.Blue);
		}

		for (int j = 0; j<rows; j++) {
			MyBotModule.Broodwar.drawLineMap(0, j*cellSize, mapWidth, j*cellSize, Color.Blue);
		}
		
		for (int r = 0; r < rows; r+=2)
		{
			for (int c = 0; c < cols; c+=2)
			{
				MyBotModule.Broodwar.drawTextMap(c * 32, r * 32, c + "," + r);
			}
		}		
	}

	/// BuildOrderQueue 를 Screen 에 표시합니다
	public void drawBuildOrderQueueOnScreen(int x, int y) {
		MyBotModule.Broodwar.drawTextScreen(x, y, white + " <Build Order>");

		/*
		std.deque< BuildOrderItem >* queue = BuildManager.Instance().buildQueue.getQueue();
		size_t reps = queue.size() < 24 ? queue.size() : 24;
		for (size_t i(0); i<reps; i++) {
			const MetaType & type = (*queue)[queue.size() - 1 - i].metaType;
			MyBotModule.game.drawTextScreen(x, y + 10 + (i * 10), " %s", type.getName().c_str());
		}
		*/

		Deque<BuildOrderItem> buildQueue = BuildManager.Instance().buildQueue.getQueue();
		int itemCount = 0;

		// C++ : for (std.deque<BuildOrderItem>.reverse_iterator itr = buildQueue.rbegin(); itr != buildQueue.rend(); itr++) {
		// C++ : 			BuildOrderItem & currentItem = *itr;
		// C++ : 			MyBotModule.game.drawTextScreen(x, y + 10 + (itemCount * 10), " %s", currentItem.metaType.getName().c_str());
		// C++ : 			itemCount++;
		// C++ : 			if (itemCount >= 24) break;
		// C++ : 		}
		
		Object[] tempQueue = buildQueue.toArray();
		
		for(int i=0 ; i<tempQueue.length ; i++){
			BuildOrderItem currentItem = (BuildOrderItem)tempQueue[i];
			MyBotModule.Broodwar.drawTextScreen(x, y + 10 + (itemCount * 10), white + currentItem.metaType.getName() + " " + currentItem.blocking);
			itemCount++;
			if (itemCount >= 24) break;
		}
	}

	/// Build 진행 상태를 Screen 에 표시합니다
	public void drawBuildStatusOnScreen(int x, int y) {
		// 건설 / 훈련 중인 유닛 진행상황 표시
		Vector<Unit> unitsUnderConstruction = new Vector<Unit>();
		for (Unit unit : MyBotModule.Broodwar.self().getUnits())
		{
			if (unit != null && unit.isBeingConstructed())
			{
				unitsUnderConstruction.add(unit);
			}
		}

		// sort it based on the time it was started
		Object[] tempArr = unitsUnderConstruction.toArray();
		//Arrays.sort(tempArr);
		unitsUnderConstruction = new Vector<Unit>();
		for(int i=0 ; i<tempArr.length ; i++){
			unitsUnderConstruction.add((Unit)tempArr[i]);
		}
		// C++ : std.sort(unitsUnderConstruction.begin(), unitsUnderConstruction.end(), CompareWhenStarted());

		MyBotModule.Broodwar.drawTextScreen(x, y, white + " <Build Status>");

		int reps = unitsUnderConstruction.size() < 10 ? unitsUnderConstruction.size() : 10;

		for (Unit unit : unitsUnderConstruction)
		{
			y += 10;
			UnitType t = unit.getType();
			if (t == UnitType.Zerg_Egg)
			{
				t = unit.getBuildType();
			}

			MyBotModule.Broodwar.drawTextScreen(x, y, "" + white + t + " (" + unit.getRemainingBuildTime() + ")");
		}

		// Tech Research 표시

		// Upgrade 표시
	}

	/// Construction 을 하기 위해 예약해둔 Tile 들을 Map 에 표시합니다
	public void drawReservedBuildingTilesOnMap() {
		boolean[][] reserveMap = ConstructionPlaceFinder.Instance().getReserveMap();
		if(reserveMap.length > 0 && reserveMap[0] != null && reserveMap[0].length > 0)
		{
			int rwidth = reserveMap.length;
			int rheight = reserveMap[0].length;

			for (int x = 0; x < rwidth; ++x)
			{
				for (int y = 0; y < rheight; ++y)
				{
					if (reserveMap[x][y])
					{
						int x1 = x * 32 + 8;
						int y1 = y * 32 + 8;
						int x2 = (x + 1) * 32 - 8;
						int y2 = (y + 1) * 32 - 8;

						MyBotModule.Broodwar.drawBoxMap(x1, y1, x2, y2, Color.Yellow, false);
					}
				}
			}
		}
	}
	
	/// Construction 을 하지 못하는 Tile 들을 Map 에 표시합니다
	public void drawTilesToAvoidOnMap() {
		Set<TilePosition> tilesToAvoid = ConstructionPlaceFinder.Instance().getTilesToAvoid();
		for (TilePosition t : tilesToAvoid)
		{
			int x1 = t.getX() * 32 + 8;
			int y1 = t.getY() * 32 + 8;
			int x2 = (t.getX() + 1) * 32 - 8;
			int y2 = (t.getY() + 1) * 32 - 8;

			MyBotModule.Broodwar.drawBoxMap(x1, y1, x2, y2, Color.Orange, false);
		}
	}

	/// ConstructionQueue 를 Screen 에 표시합니다
	public void drawConstructionQueueOnScreenAndMap(int x, int y) {
		MyBotModule.Broodwar.drawTextScreen(x, y, white + " <Construction Status>");

		int yspace = 0;

		Vector<ConstructionTask> constructionQueue = ConstructionManager.Instance().getConstructionQueue();

		for (final ConstructionTask b : constructionQueue)
		{
			String constructionState = "";

			if (b.getStatus() == ConstructionTask.ConstructionStatus.Unassigned.ordinal())
			{
				MyBotModule.Broodwar.drawTextScreen(x, y + 10 + ((yspace)* 10), "" + white + b.getType() + " - No Worker");
			}
			else if (b.getStatus() == ConstructionTask.ConstructionStatus.Assigned.ordinal())
			{
				if (b.getConstructionWorker() == null) {
					MyBotModule.Broodwar.drawTextScreen(x, y + 10 + ((yspace)* 10), b.getType() + " - Assigned Worker Null");
				}			
				else {
					MyBotModule.Broodwar.drawTextScreen(x, y + 10 + ((yspace)* 10), b.getType() + " - Assigned Worker " + b.getConstructionWorker().getID() + ", Position (" + b.getFinalPosition().getX() + "," + b.getFinalPosition().getY() + ")");
				}

				int x1 = b.getFinalPosition().getX() * 32;
				int y1 = b.getFinalPosition().getY() * 32;
				int x2 = (b.getFinalPosition().getX()+ b.getType().tileWidth()) * 32;
				int y2 = (b.getFinalPosition().getY() + b.getType().tileHeight()) * 32;

				MyBotModule.Broodwar.drawLineMap(b.getConstructionWorker().getPosition().getX(), b.getConstructionWorker().getPosition().getY(), (x1 + x2) / 2, (y1 + y2) / 2, Color.Orange);
				MyBotModule.Broodwar.drawBoxMap(x1, y1, x2, y2, Color.Red, false);
			}
			else if (b.getStatus() == ConstructionTask.ConstructionStatus.UnderConstruction.ordinal())
			{
				MyBotModule.Broodwar.drawTextScreen(x, y + 10 + ((yspace)* 10), "" + white + b.getType() + " - Under Construction");
			}
			yspace++;
		}
	}

	/// Unit 의 Id 를 Map 에 표시합니다
	public void drawUnitIdOnMap() {
		for (Unit unit : MyBotModule.Broodwar.self().getUnits())
		{
			MyBotModule.Broodwar.drawTextMap(unit.getPosition().getX(), unit.getPosition().getY() + 5, "" + white + unit.getID());
		}
		for (Unit unit : MyBotModule.Broodwar.enemy().getUnits())
		{
			MyBotModule.Broodwar.drawTextMap(unit.getPosition().getX(), unit.getPosition().getY() + 5, "" + white + unit.getID());
		}
	}

	/// Worker Unit 들의 상태를 Screen 에 표시합니다
	public void drawWorkerStateOnScreen(int x, int y) {
		WorkerData  workerData = WorkerManager.Instance().getWorkerData();

		MyBotModule.Broodwar.drawTextScreen(x, y, white + "<Workers : " + workerData.getNumMineralWorkers() + ">");

		int yspace = 0;

		for (Unit unit : workerData.getWorkers())
		{
			if (unit == null) continue;

			// Mineral / Gas / Idle Worker 는 표시 안한다
			if (workerData.getJobCode(unit) == 'M' || workerData.getJobCode(unit) == 'I' || workerData.getJobCode(unit) == 'G') {
				continue;
			}

			MyBotModule.Broodwar.drawTextScreen(x, y + 10 + ((yspace)* 10), white + " " + unit.getID());

			if (workerData.getJobCode(unit) == 'B') {
				MyBotModule.Broodwar.drawTextScreen(x + 30, y + 10 + ((yspace++) * 10), white + " " + workerData.getJobCode(unit) + " " + unit.getBuildType() + " " + (unit.isConstructing() ? 'Y' : 'N') + " (" + unit.getTilePosition().getX() + ", " + unit.getTilePosition().getY() + ")");
			}
			else {
				MyBotModule.Broodwar.drawTextScreen(x + 30, y + 10 + ((yspace++) * 10), white + " " + workerData.getJobCode(unit));
			}
		}
	}

	/// ResourceDepot 별 Worker 숫자를 Map 에 표시합니다
	public void drawWorkerCountOnMap() {
		for (Unit depot : WorkerManager.Instance().getWorkerData().getDepots())
		{
			if (depot == null) continue;

			int x = depot.getPosition().getX() - 64;
			int y = depot.getPosition().getY() - 32;

			MyBotModule.Broodwar.drawBoxMap(x - 2, y - 1, x + 75, y + 14, Color.Black, true);
			MyBotModule.Broodwar.drawTextMap(x, y, white + " Workers: " + WorkerManager.Instance().getWorkerData().getNumAssignedWorkers(depot));
		}
	}

	/// Worker Unit 의 자원채취 현황을 Map 에 표시합니다
	public void drawWorkerMiningStatusOnMap() {
		WorkerData  workerData = WorkerManager.Instance().getWorkerData();

		for (Unit worker : workerData.getWorkers())
		{
			if (worker == null) continue;

			Position pos = worker.getTargetPosition();

			MyBotModule.Broodwar.drawTextMap(worker.getPosition().getX(), worker.getPosition().getY() - 5, "" + white + workerData.getJobCode(worker));
			
			MyBotModule.Broodwar.drawLineMap(worker.getPosition().getX(), worker.getPosition().getY(), pos.getX(), pos.getY(), Color.Cyan);

			/*
			// ResourceDepot ~ Worker 사이에 직선 표시
			BWAPI.Unit depot = workerData.getWorkerDepot(worker);
			if (depot) {
				MyBotModule.game.drawLineMap(worker.getPosition().x, worker.getPosition().y, depot.getPosition().x, depot.getPosition().y, Color.Orange);
			}
			*/
		}
	}

	/// 정찰 상태를 Screen 에 표시합니다
	public void drawScoutInformation(int x, int y)
	{
		int currentScoutStatus = ScoutManager.Instance().getScoutStatus();
		String scoutStatusString = null;

		if(currentScoutStatus == ScoutManager.ScoutStatus.MovingToAnotherBaseLocation.ordinal()){
			scoutStatusString = "Moving To Another Base Location";
		}else if(currentScoutStatus == ScoutManager.ScoutStatus.MoveAroundEnemyBaseLocation.ordinal()){
			scoutStatusString = "Move Around Enemy BaseLocation";
		}else if(currentScoutStatus == ScoutManager.ScoutStatus.NoScout.ordinal()){
			scoutStatusString = "No Scout";
		}else{
			scoutStatusString = "No Scout";
		}

		// get the enemy base location, if we have one
		BaseLocation enemyBaseLocation = InformationManager.Instance().getMainBaseLocation(InformationManager.Instance().enemyPlayer);

		if (enemyBaseLocation != null) {
			MyBotModule.Broodwar.drawTextScreen(x, y, "Enemy MainBaseLocation : (" + enemyBaseLocation.getTilePosition().getX() + ", " + enemyBaseLocation.getTilePosition().getY() + ")");
		}
		else {
			MyBotModule.Broodwar.drawTextScreen(x, y, "Enemy MainBaseLocation : Unknown");
		}

		if (currentScoutStatus == ScoutManager.ScoutStatus.NoScout.ordinal()) {
			MyBotModule.Broodwar.drawTextScreen(x, y + 10, "No Scout Unit");
		}
		else {
			
			Unit scoutUnit = ScoutManager.Instance().getScoutUnit();
			if (scoutUnit != null) {
				MyBotModule.Broodwar.drawTextScreen(x, y + 10, "Scout Unit : " + scoutUnit.getType() + " " + scoutUnit.getID() + " (" + scoutUnit.getTilePosition().getX() + ", " + scoutUnit.getTilePosition().getY() + ")");
	
				Position scoutMoveTo = scoutUnit.getTargetPosition();
	
				if (scoutMoveTo != null && scoutMoveTo != Position.None && scoutMoveTo.isValid()) {
	
					double currentScoutTargetDistance;
	
					if (currentScoutStatus == ScoutManager.ScoutStatus.MovingToAnotherBaseLocation.ordinal()) {
						if (scoutUnit.getType().isFlyer()) {
							currentScoutTargetDistance = (int)(scoutUnit.getPosition().getDistance(scoutMoveTo));
						}
						else {
							currentScoutTargetDistance = BWTA.getGroundDistance(scoutUnit.getTilePosition(), scoutMoveTo.toTilePosition());
						}
	
						MyBotModule.Broodwar.drawTextScreen(x, y + 20, "Target = (" + scoutMoveTo.getX() / Config.TILE_SIZE + ", " + scoutMoveTo.getY() / Config.TILE_SIZE + ") Distance = " + currentScoutTargetDistance);
					}
					/*
					else if (currentScoutStatus == ScoutManager.ScoutStatus.MoveAroundEnemyBaseLocation.ordinal()) {
	
						Vector<Position> vertices = ScoutManager.Instance().getEnemyRegionVertices();
						for (int i = 0 ; i < vertices.size() ; ++i)
						{
							MyBotModule.Broodwar.drawCircleMap(vertices.get(i), 4, Color.Green, false);
							MyBotModule.Broodwar.drawTextMap(vertices.get(i), "" + i);
						}
						MyBotModule.Broodwar.drawCircleMap(scoutMoveTo, 5, Color.Red, true);
					}
					*/
				}
			}
		}
	}

	/// Unit 의 Target 으로 잇는 선을 Map 에 표시합니다
	public void drawUnitTargetOnMap() 
	{
		for (Unit unit : MyBotModule.Broodwar.self().getUnits())
		{
			if (unit != null && unit.isCompleted() && !unit.getType().isBuilding() && !unit.getType().isWorker())
			{
				Unit targetUnit = unit.getTarget();
				if (targetUnit != null && targetUnit.getPlayer() != MyBotModule.Broodwar.self()) {
					MyBotModule.Broodwar.drawCircleMap(unit.getPosition(), dotRadius, Color.Red, true);
					MyBotModule.Broodwar.drawCircleMap(targetUnit.getTargetPosition(), dotRadius, Color.Red, true);
					MyBotModule.Broodwar.drawLineMap(unit.getPosition(), targetUnit.getTargetPosition(), Color.Red);
				}
				else if (unit.isMoving()) {
					MyBotModule.Broodwar.drawCircleMap(unit.getPosition(), dotRadius, Color.Orange, true);
					MyBotModule.Broodwar.drawCircleMap(unit.getTargetPosition(), dotRadius, Color.Orange, true);
					MyBotModule.Broodwar.drawLineMap(unit.getPosition(), unit.getTargetPosition(), Color.Orange);
				}

			}
		}
	}

	/// Bullet 을 Map 에 표시합니다 <br>
	/// Cloaking Unit 의 Bullet 표시에 쓰입니다
	public void drawBulletsOnMap()
	{
		for (Bullet b : MyBotModule.Broodwar.getBullets())
		{
			Position p = b.getPosition();
			double velocityX = b.getVelocityX();
			double velocityY = b.getVelocityY();

			if(b.getType() == BulletType.Acid_Spore) bulletTypeName = "Acid_Spore";
			else if(b.getType() == BulletType.Anti_Matter_Missile) bulletTypeName = "Anti_Matter_Missile";
			else if(b.getType() == BulletType.Arclite_Shock_Cannon_Hit) bulletTypeName = "Arclite_Shock_Cannon_Hit";
			else if(b.getType() == BulletType.ATS_ATA_Laser_Battery) bulletTypeName = "ATS_ATA_Laser_Battery";
			else if(b.getType() == BulletType.Burst_Lasers) bulletTypeName = "Burst_Lasers";
			else if(b.getType() == BulletType.C_10_Canister_Rifle_Hit) bulletTypeName = "C_10_Canister_Rifle_Hit";
			else if(b.getType() == BulletType.Consume) bulletTypeName = "Consume";
			else if(b.getType() == BulletType.Corrosive_Acid_Shot) bulletTypeName = "Corrosive_Acid_Shot";
			else if(b.getType() == BulletType.Dual_Photon_Blasters_Hit) bulletTypeName = "Dual_Photon_Blasters_Hit";
			else if(b.getType() == BulletType.EMP_Missile) bulletTypeName = "EMP_Missile";
			else if(b.getType() == BulletType.Ensnare) bulletTypeName = "Ensnare";
			else if(b.getType() == BulletType.Fragmentation_Grenade) bulletTypeName = "Fragmentation_Grenade";
			else if(b.getType() == BulletType.Fusion_Cutter_Hit) bulletTypeName = "Fusion_Cutter_Hit";
			else if(b.getType() == BulletType.Gauss_Rifle_Hit) bulletTypeName = "Gauss_Rifle_Hit";
			else if(b.getType() == BulletType.Gemini_Missiles) bulletTypeName = "Gemini_Missiles";
			else if(b.getType() == BulletType.Glave_Wurm) bulletTypeName = "Glave_Wurm";
			else if(b.getType() == BulletType.Halo_Rockets) bulletTypeName = "Halo_Rockets";
			else if(b.getType() == BulletType.Invisible) bulletTypeName = "Invisible";
			else if(b.getType() == BulletType.Longbolt_Missile) bulletTypeName = "Longbolt_Missile";
			else if(b.getType() == BulletType.Melee) bulletTypeName = "Melee";
			else if(b.getType() == BulletType.Needle_Spine_Hit) bulletTypeName = "Needle_Spine_Hit";
			else if(b.getType() == BulletType.Neutron_Flare) bulletTypeName = "Neutron_Flare";
			else if(b.getType() == BulletType.None) bulletTypeName = "None";
			else if(b.getType() == BulletType.Optical_Flare_Grenade) bulletTypeName = "Optical_Flare_Grenade";
			else if(b.getType() == BulletType.Particle_Beam_Hit) bulletTypeName = "Particle_Beam_Hit";
			else if(b.getType() == BulletType.Phase_Disruptor) bulletTypeName = "Phase_Disruptor";
			else if(b.getType() == BulletType.Plague_Cloud) bulletTypeName = "Plague_Cloud";
			else if(b.getType() == BulletType.Psionic_Shockwave_Hit) bulletTypeName = "Psionic_Shockwave_Hit";
			else if(b.getType() == BulletType.Psionic_Storm) bulletTypeName = "Psionic_Storm";
			else if(b.getType() == BulletType.Pulse_Cannon) bulletTypeName = "Pulse_Cannon";
			else if(b.getType() == BulletType.Queen_Spell_Carrier) bulletTypeName = "Queen_Spell_Carrier";
			else if(b.getType() == BulletType.Seeker_Spores) bulletTypeName = "Seeker_Spores";
			else if(b.getType() == BulletType.STA_STS_Cannon_Overlay) bulletTypeName = "STA_STS_Cannon_Overlay";
			else if(b.getType() == BulletType.Subterranean_Spines) bulletTypeName = "Subterranean_Spines";
			else if(b.getType() == BulletType.Sunken_Colony_Tentacle) bulletTypeName = "Sunken_Colony_Tentacle";
			else if(b.getType() == BulletType.Unknown) bulletTypeName = "Unknown";
			else if(b.getType() == BulletType.Yamato_Gun) bulletTypeName = "Yamato_Gun";
			
			// 아군 것이면 녹색, 적군 것이면 빨간색
			MyBotModule.Broodwar.drawLineMap(p, new Position(p.getX() + (int)velocityX, p.getY() + (int)velocityY), b.getPlayer() == MyBotModule.Broodwar.self() ? Color.Green : Color.Red);
			if(b.getType() != null)
			{
				MyBotModule.Broodwar.drawTextMap(p, (b.getPlayer() == MyBotModule.Broodwar.self() ? "" + teal : "" + red) + bulletTypeName);
			}
		}
	}
}