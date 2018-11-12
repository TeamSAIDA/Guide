#pragma once

#include "Common.h"
#include "GameCommander.h"

namespace MyBot
{
	class MyBotModule
	{
		GameCommander   gameCommander;

	public:
		MyBotModule();
		~MyBotModule();

		void onStart();
		void onEnd(bool isWinner);
		void onFrame();
		void onSendText(std::string text);
		void onReceiveText(BWAPI::Player player, std::string text);
		void onPlayerLeft(BWAPI::Player player);
		void onNukeDetect(BWAPI::Position target);
		void onUnitDiscover(BWAPI::Unit unit);
		void onUnitEvade(BWAPI::Unit unit);
		void onUnitShow(BWAPI::Unit unit);
		void onUnitHide(BWAPI::Unit unit);
		void onUnitCreate(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);
		void onUnitMorph(BWAPI::Unit unit);
		void onUnitRenegade(BWAPI::Unit unit);
		void onSaveGame(std::string gameName);
		void onUnitComplete(BWAPI::Unit unit);
	};

}