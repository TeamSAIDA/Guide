#pragma once

#include "Common.h"
#include "UnitData.h"
#include "BuildOrderQueue.h"
#include "InformationManager.h"
#include "WorkerManager.h"
#include "BuildManager.h"
#include "ConstructionManager.h"
#include "ScoutManager.h"
#include "StrategyManager.h"

namespace MyBot
{
	/// 봇 프로그램 개발의 편의성 향상을 위해 게임 화면에 추가 정보들을 표시하는 class<br>
	/// 여러 Manager 들로부터 정보를 조회하여 Screen 혹은 Map 에 정보를 표시합니다
	class UXManager
	{
		UXManager();

		const int dotRadius = 2;

		// 게임 개요 정보를 Screen 에 표시합니다
		void drawGameInformationOnScreen(int x, int y);
		
		/// APM (Action Per Minute) 숫자를 Screen 에 표시합니다
		void drawAPM(int x, int y);

		/// Players 정보를 Screen 에 표시합니다
		void drawPlayers();

		/// Player 들의 팀 (Force) 들의 정보를 Screen 에 표시합니다
		void drawForces();



		/// BWTA 라이브러리에 의한 Map 분석 결과 정보를 Map 에 표시합니다
		void drawBWTAResultOnMap();

		/// Tile Position 그리드를 Map 에 표시합니다
		void drawMapGrid();


		/// BuildOrderQueue 를 Screen 에 표시합니다
		void drawBuildOrderQueueOnScreen(int x, int y);

		/// ConstructionQueue 를 Screen 에 표시합니다
		void drawConstructionQueueOnScreenAndMap(int x, int y);

		/// Construction 을 하기 위해 예약해둔 Tile 들을 Map 에 표시합니다
		void drawReservedBuildingTilesOnMap();

		/// Construction 을 하지 못하는 Tile 들을 Map 에 표시합니다
		void drawTilesToAvoidOnMap();

		/// Build 진행 상태를 Screen 에 표시합니다
		void drawBuildStatusOnScreen(int x, int y);


		/// Unit 의 HitPoint 등 추가 정보를 Map 에 표시합니다
		void drawUnitExtendedInformationOnMap();

		/// UnitType 별 통계 정보를 Screen 에 표시합니다
		void drawUnitStatisticsOnScreen(int x, int y);

		/// Unit 의 Id 를 Map 에 표시합니다
		void drawUnitIdOnMap();

		/// Unit 의 Target 으로 잇는 선을 Map 에 표시합니다
		void drawUnitTargetOnMap();

		/// Bullet 을 Map 에 표시합니다 
		/// Cloaking Unit 의 Bullet 표시에 쓰입니다
		void drawBulletsOnMap();



		/// Worker Unit 의 자원채취 현황을 Map 에 표시합니다
		void drawWorkerMiningStatusOnMap();

		/// ResourceDepot 별 Worker 숫자를 Map 에 표시합니다
		void drawWorkerCountOnMap();

		/// Worker Unit 들의 상태를 Screen 에 표시합니다
		void drawWorkerStateOnScreen(int x, int y);


		/// 정찰 상태를 Screen 에 표시합니다
		void drawScoutInformation(int x, int y);

	public:
		/// static singleton 객체를 리턴합니다
		static UXManager &	Instance();

		/// 경기가 시작될 때 일회적으로 추가 정보를 출력합니다
		void onStart();

		/// 경기 진행 중 매 프레임마다 추가 정보를 출력하고 사용자 입력을 처리합니다
		void update();
	};

	/// 빌드 진행상황을 빌드 시작 순서대로 정렬하여 표시하기 위한 Comparator class
	class CompareWhenStarted
	{
	public:

		CompareWhenStarted() {}

		/// 빌드 진행상황을 빌드 시작 순서대로 정렬하여 표시하기 위한 sorting operator
		bool operator() (BWAPI::Unit u1, BWAPI::Unit u2)
		{
			int startedU1 = BWAPI::Broodwar->getFrameCount() - (u1->getType().buildTime() - u1->getRemainingBuildTime());
			int startedU2 = BWAPI::Broodwar->getFrameCount() - (u2->getType().buildTime() - u2->getRemainingBuildTime());
			return startedU1 > startedU2;
		}
	};

}
