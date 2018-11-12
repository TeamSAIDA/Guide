#include "Config.h"

namespace Config
{
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 봇 이름 및 파일 경로 기본값 변경

	namespace BotInfo
	{
		std::string BotName = "NoNameBot";
		std::string BotAuthors = "NoName";
	}

    namespace Files
    {
		std::string LogFilename = Config::BotInfo::BotName + "_LastGameLog.dat";
		std::string ReadDirectory = "bwapi-data\\read\\";
		std::string WriteDirectory = "bwapi-data\\write\\";		
    }

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////

	namespace BWAPIOptions
	{
		int SetLocalSpeed = 20;
		int SetFrameSkip = 0;
		bool EnableUserInput = true;
		bool EnableCompleteMapInformation = false;
	}
	
	namespace Tools
	{
		extern int MAP_GRID_SIZE = 32;      
	}

	namespace Macro
	{
		int WorkersPerRefinery = 3;
		int BuildingSpacing = 2;
		int BuildingResourceDepotSpacing = 0;
		int BuildingPylonEarlyStageSpacing = 4;
		int BuildingPylonSpacing = 2;
		int BuildingSupplyDepotSpacing = 0;
		int BuildingDefenseTowerSpacing = 0;
	}

	namespace Debug
	{
		bool DrawGameInfo = true;
		bool DrawUnitHealthBars = true;
		bool DrawProductionInfo = true;
		bool DrawScoutInfo = true;
		bool DrawResourceInfo = false;
		bool DrawWorkerInfo = true;
		bool DrawReservedBuildingTiles = false;
		bool DrawBuildingInfo = true;
		bool DrawMouseCursorInfo = true;
		bool DrawEnemyUnitInfo = true;
		bool DrawBWTAInfo = true;
		bool DrawMapGrid = false;
		bool DrawUnitTargetInfo = true;
	}

}