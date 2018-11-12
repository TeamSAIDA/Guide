#include "MyBotModule.h"

using namespace BWAPI;
using namespace BWTA;
using namespace MyBot;

MyBotModule::MyBotModule(){
}

MyBotModule::~MyBotModule(){
}

void MyBotModule::onStart(){
	// 리플레이 재생일 경우 아무것도 하지 않음
	if (BWAPI::Broodwar->isReplay()) {
		return;
	}

	// 랜덤 시드값 설정
	time_t t;
	srand((unsigned int)(time(&t)));

	// 전체 지도 및 상대편 이벤트들 다 파악하는 모드
	//BWAPI::Broodwar->enableFlag(BWAPI::Flag::CompleteMapInformation);
	// 키보드/마우스로 게임 플레이를 진행할 수 있는 모드
	BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);
	
	// 동일한 게임 명령은 하나로 처리해서 CPU 부담을 줄여줌
	Broodwar->setCommandOptimizationLevel(1);

	// Sets the number of milliseconds bwapi spends in each frame
	// Fastest: 42 ms/frame.  1초에 약 24 frame (정확히는 23.80952380952381 frame). 가장 일반적인 게임 속도
	// Normal: 67 ms/frame. 1초에 약 15 frame
	// As fast as possible : 0 ms/frame. CPU가 할수있는 가장 빠른 속도. 
	// 개발 시에는 1초에 15 frame 을 게임속도로 한다
	BWAPI::Broodwar->setLocalSpeed(20);
	// frameskip을 늘리면 화면 표시도 업데이트 안하므로 훨씬 빠르다
	BWAPI::Broodwar->setFrameSkip(0);
	
	std::cout << "Map analyzing started" << std::endl;
	BWTA::readMap();
	BWTA::analyze();
	BWTA::buildChokeNodes();
	std::cout << "Map analyzing finished" << std::endl;
}

void MyBotModule::onEnd(bool isWinner){
	if (isWinner)
		std::cout << "I won the game" << std::endl;
	else
		std::cout << "I lost the game" << std::endl;
}

void MyBotModule::onFrame(){

	// 대결 시작한지 500 frame 이 되었을 때 1번만 표시
	if (BWAPI::Broodwar->getFrameCount() == 500) {

		// 명령 프롬프트에 표시
		std::cout << "Hello Starcraft command prompt" << std::endl;

		// 게임 화면에 표시
		BWAPI::Broodwar << "Hello Starcraft game screen" << std::endl;
	}
}

void MyBotModule::onSendText(std::string text){
	BWAPI::Broodwar->sendText("%s", text.c_str());

	// Display the text to the game 
	BWAPI::Broodwar << text << std::endl;
}

void MyBotModule::onReceiveText(BWAPI::Player player, std::string text){
	BWAPI::Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void MyBotModule::onPlayerLeft(BWAPI::Player player){
	BWAPI::Broodwar << player->getName() << " left the game." << std::endl;
}

void MyBotModule::onNukeDetect(BWAPI::Position target){
	if (target != Positions::Unknown)
	{
		BWAPI::Broodwar->drawCircleMap(target, 40, Colors::Red, true);
		BWAPI::Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else {
		BWAPI::Broodwar << "Nuclear Launch Detected" << std::endl;
	}
}

void MyBotModule::onUnitCreate(BWAPI::Unit unit){
	if (unit->getPlayer()->isNeutral() == false)
	{
		BWAPI::Broodwar << unit->getType().c_str() << " " << unit->getID() << " created at " << unit->getTilePosition().x << ", " << unit->getTilePosition().y << std::endl;
	}
}

void MyBotModule::onUnitMorph(BWAPI::Unit unit){
	if (unit->getPlayer()->isNeutral() == false)
	{
		BWAPI::Broodwar << unit->getType().c_str() << " " << unit->getID() << " morphed at " << unit->getTilePosition().x << "," << unit->getTilePosition().y << std::endl;
	}
}

void MyBotModule::onUnitDestroy(BWAPI::Unit unit){
	if (unit->getPlayer()->isNeutral() == false)
	{
		BWAPI::Broodwar << unit->getType().c_str() << " " << unit->getID() << " destroyed at " << unit->getTilePosition().x << "," << unit->getTilePosition().y << std::endl;
	}
}

void MyBotModule::onUnitShow(BWAPI::Unit unit){
	if (unit->getPlayer()->isNeutral() == false)
	{
		BWAPI::Broodwar << unit->getType().c_str() << " " << unit->getID() << " showed at " << unit->getTilePosition().x << "," << unit->getTilePosition().y << std::endl;
	}
}

void MyBotModule::onUnitHide(BWAPI::Unit unit){
	if (unit->getPlayer()->isNeutral() == false)
	{
		BWAPI::Broodwar << unit->getType().c_str() << " " << unit->getID() << " hid at " << unit->getTilePosition().x << "," << unit->getTilePosition().y << std::endl;
	}
}

void MyBotModule::onUnitRenegade(BWAPI::Unit unit){
	BWAPI::Broodwar << unit->getType().c_str() << " " << unit->getID() << " renegaded at " << unit->getTilePosition().x << "," << unit->getTilePosition().y << std::endl;
}

void MyBotModule::onUnitDiscover(BWAPI::Unit unit){
	if (unit->getPlayer()->isNeutral() == false)
	{
		BWAPI::Broodwar << unit->getType().c_str() << " " << unit->getID() << " discovered at " << unit->getTilePosition().x << "," << unit->getTilePosition().y << std::endl;
	}
}

void MyBotModule::onUnitEvade(BWAPI::Unit unit){
	if (unit->getPlayer()->isNeutral() == false)
	{
		BWAPI::Broodwar << unit->getType().c_str() << " " << unit->getID() << " evaded at " << unit->getTilePosition().x << "," << unit->getTilePosition().y << std::endl;
	}
}

void MyBotModule::onUnitComplete(BWAPI::Unit unit){
	if (unit->getPlayer()->isNeutral() == false)
	{
		BWAPI::Broodwar << unit->getType().c_str() << " " << unit->getID() << " completed at " << unit->getTilePosition().x << "," << unit->getTilePosition().y << std::endl;
	}
}

void MyBotModule::onSaveGame(std::string gameName){
	BWAPI::Broodwar->sendText("The game was saved to \"%s\".", gameName.c_str());
}


