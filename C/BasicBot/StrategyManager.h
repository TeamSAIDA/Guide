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
	/// 상황을 판단하여, 정찰, 빌드, 공격, 방어 등을 수행하도록 총괄 지휘를 하는 class<br>
	/// InformationManager 에 있는 정보들로부터 상황을 판단하고, <br>
	/// BuildManager 의 buildQueue에 빌드 (건물 건설 / 유닛 훈련 / 테크 리서치 / 업그레이드) 명령을 입력합니다.<br>
	/// 정찰, 빌드, 공격, 방어 등을 수행하는 코드가 들어가는 class
	class StrategyManager
	{
		StrategyManager();

		bool isInitialBuildOrderFinished;
		void setInitialBuildOrder();

		void executeWorkerTraining();
		void executeSupplyManagement();
		void executeBasicCombatUnitTraining();

		bool isFullScaleAttackStarted;
		void executeCombat();

	public:
		/// static singleton 객체를 리턴합니다
		static StrategyManager &	Instance();

		/// 경기가 시작될 때 일회적으로 전략 초기 세팅 관련 로직을 실행합니다
		void onStart();

		///  경기가 종료될 때 일회적으로 전략 결과 정리 관련 로직을 실행합니다
		void onEnd(bool isWinner);

		/// 경기 진행 중 매 프레임마다 경기 전략 관련 로직을 실행합니다
		void update();

	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 경기 결과 파일 Save / Load 및 로그파일 Save 예제 추가를 위한 변수 및 메소드 선언

	private:
		/// 한 게임에 대한 기록을 저장하는 자료구조
		class GameRecord {
		public:
			std::string mapName;
			std::string enemyName;
			std::string enemyRace;
			std::string enemyRealRace;
			std::string myName;
			std::string myRace;
			int gameFrameCount = 0;
			int myWinCount = 0;
			int myLoseCount = 0;
		};
		/// 과거 전체 게임들의 기록을 저장하는 자료구조
		std::vector<GameRecord> gameRecordList;

		/// 과거 전체 게임 기록을 로딩합니다
		void loadGameRecordList();
		/// 과거 전체 게임 기록 + 이번 게임 기록을 저장합니다
		void saveGameRecordList(bool isWinner);
		/// 이번 게임 중간에 상시적으로 로그를 저장합니다
		void saveGameLog();

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////

	};
}
