#pragma once

#include "BWAPI.h"
#include <cassert>

/// 봇 프로그램 설정
namespace Config
{
	/// 봇 기본 정보
	namespace BotInfo
	{
		/// 봇 이름
		extern std::string BotName;
		/// 봇 개발자 이름
		extern std::string BotAuthors;
	}

	/// 파일 관련 설정
	namespace Files
    {
		/// 로그 파일 이름
		extern std::string LogFilename;
		/// 읽기 파일 경로
		extern std::string ReadDirectory;
		/// 쓰기 파일 경로
		extern std::string WriteDirectory;
    }
	    
	/// CommonUtil 관련 설정
	namespace Tools
	{
		/// MapGrid 에서 한 개 GridCell 의 size
		extern int MAP_GRID_SIZE;
	}
	
	/// BWAPI 옵션 관련 설정
	namespace BWAPIOptions
    {
		/// 로컬에서 게임을 실행할 때 게임스피드 (코드 제출 후 서버에서 게임을 실행할 때는 서버 설정을 사용함)<br>
		/// Speedups for automated play, sets the number of milliseconds bwapi spends in each frame.<br>
		/// Fastest: 42 ms/frame.  1초에 24 frame. 일반적으로 1초에 24frame을 기준 게임속도로 합니다.<br>
		/// Normal: 67 ms/frame. 1초에 15 frame.<br>
		/// As fast as possible : 0 ms/frame. CPU가 할수있는 가장 빠른 속도. 
        extern int SetLocalSpeed;
		/// 로컬에서 게임을 실행할 때 FrameSkip (코드 제출 후 서버에서 게임을 실행할 때는 서버 설정을 사용함)<br>
		/// frameskip을 늘리면 화면 표시도 업데이트 안하므로 훨씬 빠릅니다
        extern int SetFrameSkip;
		/// 로컬에서 게임을 실행할 때 사용자 키보드/마우스 입력 허용 여부 (코드 제출 후 서버에서 게임을 실행할 때는 서버 설정을 사용함)
		extern bool EnableUserInput;
		/// 로컬에서 게임을 실행할 때 전체 지도를 다 보이게 할 것인지 여부 (코드 제출 후 서버에서 게임을 실행할 때는 서버 설정을 사용함)
        extern bool EnableCompleteMapInformation;
    }
	
	/// 디버그 관련 설정
	namespace Debug
	{
		/// 화면 표시 여부 - 게임 정보
		extern bool DrawGameInfo;

		/// 화면 표시 여부 - 미네랄, 가스
		extern bool DrawResourceInfo;
		/// 화면 표시 여부 - 지도
		extern bool DrawBWTAInfo;
		/// 화면 표시 여부 - 바둑판
		extern bool DrawMapGrid;

		/// 화면 표시 여부 - 유닛 HitPoint
		extern bool DrawUnitHealthBars;
		/// 화면 표시 여부 - 유닛 통계
		extern bool DrawEnemyUnitInfo;
		/// 화면 표시 여부 - 유닛 ~ Target 간 직선
		extern bool DrawUnitTargetInfo;

		/// 화면 표시 여부 - 빌드 큐
		extern bool DrawProductionInfo;

		/// 화면 표시 여부 - 건물 Construction 상황
		extern bool DrawBuildingInfo;

		/// 화면 표시 여부 - 건물 ConstructionPlace 예약 상황
		extern bool DrawReservedBuildingTiles;

		/// 화면 표시 여부 - 정찰 상태
		extern bool DrawScoutInfo;
		/// 화면 표시 여부 - 일꾼 목록
		extern bool DrawWorkerInfo;

		/// 화면 표시 여부 - 마우스 커서
		extern bool DrawMouseCursorInfo;
	}

	/// 게임로직 관련 파라메터
	namespace Macro
	{
		/// 각각의 Refinery 마다 투입할 일꾼 최대 숫자
		extern int WorkersPerRefinery;

		/// 건물과 건물간 띄울 최소한의 간격 - 일반적인 건물의 경우
		extern int BuildingSpacing;
		/// 건물과 건물간 띄울 최소한의 간격 - ResourceDepot 건물의 경우 (Nexus, Hatchery, Command Center)
		extern int BuildingResourceDepotSpacing; 
		/// 건물과 건물간 띄울 최소한의 간격 - Protoss_Pylon 건물의 경우 - 게임 초기에
		extern int BuildingPylonEarlyStageSpacing;
		/// 건물과 건물간 띄울 최소한의 간격 - Protoss_Pylon 건물의 경우 - 게임 초기 이후에
		extern int BuildingPylonSpacing;
		/// 건물과 건물간 띄울 최소한의 간격 - Terran_Supply_Depot 건물의 경우
		extern int BuildingSupplyDepotSpacing;
		/// 건물과 건물간 띄울 최소한의 간격 - 방어 건물의 경우 (포톤캐논. 성큰콜로니. 스포어콜로니. 터렛. 벙커)
		extern int BuildingDefenseTowerSpacing; 
	}
}