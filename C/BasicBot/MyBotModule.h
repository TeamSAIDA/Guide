#pragma once

#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <BWTA.h>

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>

#include "Common.h"
#include "CommandUtil.h"
#include "GameCommander.h"
#include "UXManager.h"

namespace MyBot
{
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
	/// 알고리즘 경진대회 빌드서버가 사용하는 Main, MyBotModule, UXManager 파일을 예시적으로 BasicBot 에 반영하였습니다 <br>
	/// 실제 알고리즘 경진대회 빌드서버에서는 코드를 일부 수정해서 빌드하게 할 수 있습니다 <br>
	///
	/// 알고리즘 경진대회 빌드서버가 사용하는 Main 은 MyBotModule 을 실행시키는 기능을 수행합니다. <br>
	/// 알고리즘 경진대회 빌드서버가 사용하는 MyBotModule 은 GameCommander 에게 이벤트를 전달하는 기능을 수행하며, 게임 속도 지연 여부 파악, 게임 무승부 상황 파악 등을 통해 게임을 강제 패배시키거나 강제 종료시키는 행동을 수행합니다. <br>
	/// 알고리즘 경진대회 빌드서버가 사용하는 UX Manager 는 알고리즘 경진대회 운영, 사후 판정 등에 필요한 최소한의 내용만 화면에 표시합니다. <br>
	/// 이 파일들은 InformationManager 등 다른 파일들과 Dependency가 없도록 개발되었기 때문에, <br>
	/// 참가자들은 InformationManager 등 다른 파일들을 자유롭게 수정하실 수 있습니다. 
	/// 
	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
	class MyBotModule
	{
		/// 실제 봇프로그램<br>
		/// @see GameCommander
		GameCommander   gameCommander;

		/// 사용자가 입력한 text 를 parse 해서 처리합니다
		void ParseTextCommand(const std::string & commandLine);

	public:
		MyBotModule();
		~MyBotModule();

		/// 경기가 시작될 때 일회적으로 발생하는 이벤트를 처리합니다
		void onStart();
		///  경기가 종료될 때 일회적으로 발생하는 이벤트를 처리합니다
		void onEnd(bool isWinner);
		/// 경기 진행 중 매 프레임마다 발생하는 이벤트를 처리합니다
		void onFrame();

		/// 유닛(건물/지상유닛/공중유닛)이 Create 될 때 발생하는 이벤트를 처리합니다
		void onUnitCreate(BWAPI::Unit unit);
		///  유닛(건물/지상유닛/공중유닛)이 Destroy 될 때 발생하는 이벤트를 처리합니다
		void onUnitDestroy(BWAPI::Unit unit);

		/// 유닛(건물/지상유닛/공중유닛)이 Morph 될 때 발생하는 이벤트를 처리합니다
		/// Zerg 종족의 유닛은 건물 건설이나 지상유닛/공중유닛 생산에서 거의 대부분 Morph 형태로 진행됩니다
		void onUnitMorph(BWAPI::Unit unit);

		/// 유닛(건물/지상유닛/공중유닛)의 소속 플레이어가 바뀔 때 발생하는 이벤트를 처리합니다.<br>
		/// Gas Geyser에 어떤 플레이어가 Refinery 건물을 건설했을 때, Refinery 건물이 파괴되었을 때, Protoss 종족 Dark Archon 의 Mind Control 에 의해 소속 플레이어가 바뀔 때 발생합니다
		void onUnitRenegade(BWAPI::Unit unit);
		/// 유닛(건물/지상유닛/공중유닛)의 하던 일 (건물 건설, 업그레이드, 지상유닛 훈련 등)이 끝났을 때 발생하는 이벤트를 처리합니다
		void onUnitComplete(BWAPI::Unit unit);

		/// 유닛(건물/지상유닛/공중유닛)이 Discover 될 때 발생하는 이벤트를 처리합니다.<br>
		/// 아군 유닛이 Create 되었을 때 라든가, 적군 유닛이 Discover 되었을 때 발생합니다
		void onUnitDiscover(BWAPI::Unit unit);
		/// 유닛(건물/지상유닛/공중유닛)이 Evade 될 때 발생하는 이벤트를 처리합니다.<br>
		/// 유닛이 Destroy 될 때 발생합니다
		void onUnitEvade(BWAPI::Unit unit);

		/// 유닛(건물/지상유닛/공중유닛)이 Show 될 때 발생하는 이벤트를 처리합니다.<br>
		/// 아군 유닛이 Create 되었을 때 라든가, 적군 유닛이 Discover 되었을 때 발생합니다
		void onUnitShow(BWAPI::Unit unit);
		/// 유닛(건물/지상유닛/공중유닛)이 Hide 될 때 발생하는 이벤트를 처리합니다.<br>
		/// 보이던 유닛이 Hide 될 때 발생합니다
		void onUnitHide(BWAPI::Unit unit);

		/// 핵미사일 발사가 감지되었을 때 발생하는 이벤트를 처리합니다
		void onNukeDetect(BWAPI::Position target);

		/// 다른 플레이어가 대결을 나갔을 때 발생하는 이벤트를 처리합니다
		void onPlayerLeft(BWAPI::Player player);
		/// 게임을 저장할 때 발생하는 이벤트를 처리합니다
		void onSaveGame(std::string gameName);

		/// 텍스트를 입력 후 엔터를 하여 다른 플레이어들에게 텍스트를 전달하려 할 때 발생하는 이벤트를 처리합니다
		void onSendText(std::string text);
		/// 다른 플레이어로부터 텍스트를 전달받았을 때 발생하는 이벤트를 처리합니다
		void onReceiveText(BWAPI::Player player, std::string text);

	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 타임아웃 패배, 자동 패배 체크 관련 변수 및 메소드 선언

	private:
		void initializeLostConditionVariables();
		void checkLostConditions();

		bool isToCheckGameLostCondition;		///< 자동 패배 체크 실행 여부		
		bool isGameLostConditionSatisfied;		///< 자동 패배 체크 결과		
		int gameLostConditionSatisfiedFrame;	///< 자동 패배 조건이 시작된 프레임 시점
		int maxDurationForGameLostCondition;	///< 자동 패배 조건이 만족된채 게임을 유지시키는 최대 프레임 수
		void checkGameLostConditionAndLeaveGame();	///< 자동 패배 조건을 체크하여, 조건 만족 시 GG 선언하고 게임을 나갑니다

		bool isToCheckTimeOut;					///< 타임 아웃 체크 실행 여부		
		int timeOutConditionSatisfiedFrame;		///< 타임 아웃 조건이 시작된 프레임 시점		
		bool isTimeOutConditionSatisfied;		///< 타임 아웃 체크 결과		
		int maxDurationForTimeOutLostCondition;	///< 타임 아웃 조건이 만족된채 게임을 유지시키는 최대 프레임 수		
		std::vector<int> timerLimits;			///< 타임 아웃 한계시간 (ms/frame)
		std::vector<int> timerLimitsBound;		///< 타임 아웃 초과한계횟수
		std::vector<int> timerLimitsExceeded;	///< 타임 아웃 초과횟수
		std::vector<long long> timeStartedAtFrame;	///< 해당 프레임을 시작한 시각
		std::vector<long long> timeElapsedAtFrame;	///< 해당 프레임에서 사용한 시간 (ms)		
		void checkTimeOutConditionAndLeaveGame();	///< 타임 아웃 조건을 체크하여, 조건 만족 시 GG 선언하고 게임을 나갑니다


		bool isToTestTimeOut;					///< 타임 아웃 체크 테스트 실행 여부
		int timeOverTestDuration;
		int timeOverTestFrameCountLimit;
		int timeOverTestFrameCount;
		void doTimeOutDelay();					///< 타임 아웃 체크 테스트 실행 


	// BasicBot 1.1 Patch End //////////////////////////////////////////////////

	};

}