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

#include "MyBotModule.h"

using namespace BWAPI;
using namespace BWTA;
using namespace MyBot;

MyBotModule::MyBotModule(){
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 타임아웃 패배, 자동 패배 체크 관련 변수 초기화

	initializeLostConditionVariables();

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

MyBotModule::~MyBotModule(){
}

void MyBotModule::onStart(){
	
	if (BWAPI::Broodwar->isReplay()) {
		return;
	}

	time_t t;
	srand((unsigned int)(time(&t)));

	// Config 파일 관리가 번거롭고, 배포 및 사용시 Config 파일 위치를 지정해주는 것이 번거롭기 때문에, 
	// Config 를 파일로부터 읽어들이지 않고, Config 클래스의 값을 사용하도록 합니다.
	if (Config::BWAPIOptions::EnableCompleteMapInformation)
	{
		BWAPI::Broodwar->enableFlag(BWAPI::Flag::CompleteMapInformation);
	}

	if (Config::BWAPIOptions::EnableUserInput)
	{
		BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);
	}

	Broodwar->setCommandOptimizationLevel(1);


	// Speedups for automated play, sets the number of milliseconds bwapi spends in each frame
	// Fastest: 42 ms/frame.  1초에 24 frame. 일반적으로 1초에 24frame을 기준 게임속도로 합니다
	// Normal: 67 ms/frame. 1초에 15 frame
	// As fast as possible : 0 ms/frame. CPU가 할수있는 가장 빠른 속도. 
	BWAPI::Broodwar->setLocalSpeed(Config::BWAPIOptions::SetLocalSpeed);
	// frameskip을 늘리면 화면 표시도 업데이트 안하므로 훨씬 빠릅니다
	BWAPI::Broodwar->setFrameSkip(Config::BWAPIOptions::SetFrameSkip);
	
	std::cout << "Map analyzing started" << std::endl;
	BWTA::readMap();
	BWTA::analyze();
	BWTA::buildChokeNodes();
	std::cout << "Map analyzing finished" << std::endl;

	gameCommander.onStart();
}

void MyBotModule::onEnd(bool isWinner){
	if (isWinner)
		std::cout << "I won the game" << std::endl;
	else
		std::cout << "I lost the game" << std::endl;

	gameCommander.onEnd(isWinner);
}

void MyBotModule::onFrame(){

	if (BWAPI::Broodwar->isReplay()) {
		return;
	}

	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 타임아웃 패배, 자동 패배 체크 추가

	// timeStartedAtFrame 를 갱신한다
	if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
		timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	// 타임아웃 체크 메모리 부족시 증설
	if ((int)timeStartedAtFrame.size() < BWAPI::Broodwar->getFrameCount() + 10)
	{
		timeStartedAtFrame.resize(timeStartedAtFrame.size() + 10000);
		timeElapsedAtFrame.resize(timeElapsedAtFrame.size() + 10000);
	}

	// Pause 상태에서는 timeStartedAtFrame 를 계속 갱신해서, timeElapsedAtFrame 이 제대로 계산되도록 한다
	if (BWAPI::Broodwar->isPaused()) {
		timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	else {
		gameCommander.onFrame();
	}

	// 화면 출력 및 사용자 입력 처리
	// 빌드서버에서는 Dependency가 없는 빌드서버 전용 UXManager 를 실행시킵니다
	UXManager::Instance().update();

	checkLostConditions();

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
// 타임아웃 패배, 자동 패배 체크 추가

void MyBotModule::onUnitCreate(BWAPI::Unit unit){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		gameCommander.onUnitCreate(unit);
	}
}

void MyBotModule::onUnitDestroy(BWAPI::Unit unit){
	if (!BWAPI::Broodwar->isReplay()){

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		gameCommander.onUnitDestroy(unit);
	}
}

void MyBotModule::onUnitMorph(BWAPI::Unit unit){
	if (!BWAPI::Broodwar->isReplay()){

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		gameCommander.onUnitMorph(unit);
	}
}

void MyBotModule::onUnitRenegade(BWAPI::Unit unit){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		gameCommander.onUnitRenegade(unit);
	}
}

void MyBotModule::onUnitComplete(BWAPI::Unit unit){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		gameCommander.onUnitComplete(unit);
	}
}

void MyBotModule::onUnitDiscover(BWAPI::Unit unit){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		gameCommander.onUnitDiscover(unit);
	}
}

void MyBotModule::onUnitEvade(BWAPI::Unit unit){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		gameCommander.onUnitEvade(unit);
	}
}

void MyBotModule::onUnitShow(BWAPI::Unit unit){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		gameCommander.onUnitShow(unit);
	}
}

void MyBotModule::onUnitHide(BWAPI::Unit unit){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		gameCommander.onUnitHide(unit);
	}
}

void MyBotModule::onNukeDetect(BWAPI::Position target){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		// 빌드서버에서는 향후 적용
		gameCommander.onNukeDetect(target);
	}
}

void MyBotModule::onPlayerLeft(BWAPI::Player player){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		// 빌드서버에서는 향후 적용
		gameCommander.onPlayerLeft(player);
	}
}

void MyBotModule::onSaveGame(std::string gameName){
	if (!BWAPI::Broodwar->isReplay()) {

		if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
			timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		// 빌드서버에서는 향후 적용
		gameCommander.onSaveGame(gameName);
	}
}

void MyBotModule::ParseTextCommand(const std::string & commandString)
{
	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!

	BWAPI::Player self = BWAPI::Broodwar->self();

	if (commandString == "afap") {
		BWAPI::Broodwar->setLocalSpeed(0);
		BWAPI::Broodwar->setFrameSkip(0);
	}
	else if (commandString == "fast") {
		BWAPI::Broodwar->setLocalSpeed(24);
		BWAPI::Broodwar->setFrameSkip(0);
	}
	else if (commandString == "slow") {
		BWAPI::Broodwar->setLocalSpeed(42);
		BWAPI::Broodwar->setFrameSkip(0);
	}
	else if (commandString == "endthegame") {
		//bwapi->setFrameSkip(16);   // Not needed if using setGUI(false).
		BWAPI::Broodwar->setGUI(false);
	}


	// 1개 프레임 처리에 10초 넘게 걸리도록 하는 것을 1번 해본다
	else if (commandString == "delay12000_1") {
		isToTestTimeOut = true;
		timeOverTestFrameCount = 0;
		timeOverTestDuration = 12000;
		timeOverTestFrameCountLimit = 1;
	}
	// 1개 프레임 처리에 10초 넘게 걸리도록 하는 것을 2번 해본다
	else if (commandString == "delay12000_2") {
		isToTestTimeOut = true;
		timeOverTestFrameCount = 0;
		timeOverTestDuration = 12000;
		timeOverTestFrameCountLimit = 2;
	}
	// 1개 프레임 처리에 1초 넘게 걸리도록 하는 것을 9번 해본다
	else if (commandString == "delay1200_9") {
		isToTestTimeOut = true;
		timeOverTestFrameCount = 0;
		timeOverTestDuration = 1200;
		timeOverTestFrameCountLimit = 9;
	}
	// 1개 프레임 처리에 1초 넘게 걸리도록 하는 것을 12번 해본다
	else if (commandString == "delay1200_12") {
		isToTestTimeOut = true;
		timeOverTestFrameCount = 0;
		timeOverTestDuration = 1200;
		timeOverTestFrameCountLimit = 12;
	}
	// 1개 프레임 처리에 55 millisecond 넘게 걸리도록 하는 것을 310번 해본다
	else if (commandString == "delay70_310") {
		isToTestTimeOut = true;
		timeOverTestFrameCount = 0;
		timeOverTestDuration = 70;
		timeOverTestFrameCountLimit = 310;
	}
	// 1개 프레임 처리에 55 millisecond 넘게 걸리도록 하는 것을 330번 이상 해본다
	else if (commandString == "delay70_330") {
		isToTestTimeOut = true;
		timeOverTestFrameCount = 0;
		timeOverTestDuration = 70;
		timeOverTestFrameCountLimit = 330;
	}
}

void MyBotModule::onSendText(std::string text){

	if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
		timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	ParseTextCommand(text);

	gameCommander.onSendText(text);
		
	BWAPI::Broodwar->sendText("%s", text.c_str());
}

void MyBotModule::onReceiveText(BWAPI::Player player, std::string text){

	if (timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] == 0) {
		timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	gameCommander.onReceiveText(player, text);
}

void MyBotModule::initializeLostConditionVariables()
{
	// 자동 패배 조건 체크 관련 변수들 초기화
	isToCheckGameLostCondition = true;
	isGameLostConditionSatisfied = false;
	gameLostConditionSatisfiedFrame = 0;
	// 자동 패배 조건이 만족된채 게임을 유지시키는 최대 프레임 수 : 100 프레임
	maxDurationForGameLostCondition = 100;

	// 타임아웃 체크 관련 변수들 초기화
	isToCheckTimeOut = true;
	isTimeOutConditionSatisfied = false;
	timeOutConditionSatisfiedFrame = 0;
	// 타임 아웃 조건이 만족된채 게임을 유지시키는 최대 프레임 수 : 5 프레임
	maxDurationForTimeOutLostCondition = 5;

	timeStartedAtFrame.resize(100000, 0);
	timeElapsedAtFrame.resize(100000, 0);

	timerLimits.push_back(55);
	timerLimitsBound.push_back(320);
	timerLimitsExceeded.push_back(0);

	timerLimits.push_back(1000);
	timerLimitsBound.push_back(10);
	timerLimitsExceeded.push_back(0);

	timerLimits.push_back(10000);
	timerLimitsBound.push_back(2);
	timerLimitsExceeded.push_back(0);

	// 타임아웃 체크 테스트 관련 변수들 초기화
	isToTestTimeOut = false;
	timeOverTestDuration = 0;
	timeOverTestFrameCountLimit = 0;
	timeOverTestFrameCount = 0;
}

void MyBotModule::checkLostConditions()
{
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
void MyBotModule::checkGameLostConditionAndLeaveGame()
{
	bool isLostGame = false;

	int canProduceBuildingCount = 0;
	int canAttackBuildingCount = 0;
	int canDoSomeThingNonBuildingUnitCount = 0;

	for (auto & unit : BWAPI::Broodwar->self()->getUnits()) {
		if (unit->getType().isBuilding()) {

			// 생산 가능 건물이 하나라도 있으면 게임 지속 가능.
			if (unit->getType().canProduce()) {
				canProduceBuildingCount++;
				break;
			}

			// 공격 가능 건물이 하나라도 있으면 게임 지속 가능. 크립콜로니는 현재는 공격능력을 갖고있지 않지만, 향후 공격능력을 가질 수 있는 건물이므로 카운트에 포함
			if (unit->getType().canAttack() || unit->getType() == BWAPI::UnitTypes::Zerg_Creep_Colony) {
				canAttackBuildingCount++;
				break;
			}
		}
		else {
			// 생산 능력을 가진 유닛이나 공격 능력을 가진 유닛, 특수 능력을 가진 유닛이 하나라도 있으면 게임 지속 가능
			// 즉, 라바, 퀸, 디파일러, 싸이언스베쓸, 다크아칸 등은 게임 승리를 이끌 가능성이 조금이라도 있음
			// 치료, 수송, 옵저버 능력만 있는 유닛만 있으면 게임 중지.
			// 즉, 메딕, 드랍쉽, 오버로드, 옵저버, 셔틀만 존재하면, 게임 승리를 이끌 능력이 없음
			if (unit->getType().canAttack() || unit->getType().canProduce()
				|| (unit->getType().isSpellcaster() && unit->getType() != BWAPI::UnitTypes::Terran_Medic)
				|| unit->getType() == BWAPI::UnitTypes::Zerg_Larva
				|| unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg || unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon)
			{
				canDoSomeThingNonBuildingUnitCount++;
				break;
			}
		}
	}

	//BWAPI::Broodwar->drawTextScreen(250, 120, "canProduce Building Count        : %d", canProduceBuildingCount);
	//BWAPI::Broodwar->drawTextScreen(250, 130, "canAttack Building Count         : %d", canAttackBuildingCount);
	//BWAPI::Broodwar->drawTextScreen(250, 140, "canDoSomeThing NonBuilding Count : %d", canDoSomeThingNonBuildingUnitCount);

	// 자동 패배조건 만족하게 된 프레임 기록
	if (canDoSomeThingNonBuildingUnitCount == 0 && canProduceBuildingCount == 0 && canAttackBuildingCount == 0 && isGameLostConditionSatisfied == false) {
		BWAPI::Broodwar->sendText("I lost because I HAVE NO UNIT TO DEFEAT ENEMY PLAYER");
		BWAPI::Broodwar->sendText("GG");
		std::cout << "I lost because I HAVE NO UNIT TO DEFEAT ENEMY PLAYER" << std::endl;

		isGameLostConditionSatisfied = true;
		gameLostConditionSatisfiedFrame = BWAPI::Broodwar->getFrameCount();
	}
	// 자동 패배조건 벗어나게 되면 리셋
	else if (canDoSomeThingNonBuildingUnitCount != 0 || canProduceBuildingCount != 0 || canAttackBuildingCount != 0) {
		isGameLostConditionSatisfied = false;
	}

	// 자동 패배조건 만족 상황이 일정시간 동안 지속되었으면 게임 패배로 처리
	if (isGameLostConditionSatisfied) {

		BWAPI::Broodwar->drawTextScreen(250, 100, "I lost because I HAVE NO UNIT TO DEFEAT ENEMY PLAYER");
		BWAPI::Broodwar->drawTextScreen(250, 115, "I will leave game in %d frames", maxDurationForGameLostCondition - (BWAPI::Broodwar->getFrameCount() - gameLostConditionSatisfiedFrame));

		if (BWAPI::Broodwar->getFrameCount() - gameLostConditionSatisfiedFrame >= maxDurationForGameLostCondition) {
			BWAPI::Broodwar->leaveGame();
		}
	}
}


void MyBotModule::doTimeOutDelay()
{
	if (timeOverTestFrameCount < timeOverTestFrameCountLimit) {

		// 10 프레임마다 1번씩 타임 딜레이를 실행한다
		if (BWAPI::Broodwar->getFrameCount() % 10 == 0) {
			long long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - startTime < timeOverTestDuration) {
			}
			timeOverTestFrameCount++;
		}
	}
	else {
		isToTestTimeOut = false;
	}
}


void MyBotModule::checkTimeOutConditionAndLeaveGame()
{
	if (BWAPI::Broodwar->getFrameCount() >= 10) {

		// 현재 프레임에서의 소요시간 기록
		timeElapsedAtFrame[BWAPI::Broodwar->getFrameCount()] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - timeStartedAtFrame[BWAPI::Broodwar->getFrameCount()];

		long long timeElapsedAtLastFrame = timeElapsedAtFrame[BWAPI::Broodwar->getFrameCount() - 1];

		// 현재 시각 표시
		BWAPI::Broodwar->drawTextScreen(260, 5, "FrameCount :");
		BWAPI::Broodwar->drawTextScreen(340, 5, "%d", BWAPI::Broodwar->getFrameCount());
		BWAPI::Broodwar->drawTextScreen(370, 5, "(%3d:%2d)", (int)(BWAPI::Broodwar->getFrameCount() / (23.8 * 60)), (int)((int)(BWAPI::Broodwar->getFrameCount() / 23.8) % 60));

		// 타임아웃 체크 현황을 화면에 표시
		int y = 15;
		for (size_t t(0); t < timerLimits.size(); ++t)
		{
			Broodwar->drawTextScreen(260, y, "> %5d ms : %3d / %3d", timerLimits[t], timerLimitsExceeded[t], timerLimitsBound[t]);
			y += 10;
		}

		/*
		// 각 프레임별 소요시간 기록을 화면에 표시
		int y = 100;
		for (int i = BWAPI::Broodwar->getFrameCount() - 9; i < BWAPI::Broodwar->getFrameCount(); ++i)
		{
		Broodwar->drawTextScreen(260, y, "[%5d] : %20lld %5d ms", i, timeStartedAtFrame[i], timeElapsedAtFrame[i]);
		y += 10;
		}

		// 최대 많은 시간을 소요한 프레임을 화면에 표시
		y = 220;
		int maxI = 0;
		long long maxTimeElapsedAtFrame = 0;
		for (int i = 0; i < BWAPI::Broodwar->getFrameCount(); ++i)
		{
		if (maxTimeElapsedAtFrame < timeElapsedAtFrame[i]) {
		maxTimeElapsedAtFrame = timeElapsedAtFrame[i];
		maxI = i;
		}
		}
		Broodwar->drawTextScreen(260, y, "[%5d] : %20lld %5d ms", maxI, timeStartedAtFrame[maxI], timeElapsedAtFrame[maxI]);
		*/

		// 타임아웃 체크 및 게임 종료
		for (size_t t(0); t < timerLimits.size(); ++t)
		{
			if (timeElapsedAtLastFrame > timerLimits[t])
			{
				timerLimitsExceeded[t]++;

				if (timerLimitsExceeded[t] == timerLimitsBound[t])
				{
					isTimeOutConditionSatisfied = true;
					timeOutConditionSatisfiedFrame = BWAPI::Broodwar->getFrameCount();

					Broodwar->sendText("I lost because of TIMEOUT (%d frames exceed %d ms/frame)", timerLimitsBound[t], timerLimits[t]);
					Broodwar->sendText("GG");
					std::cout << "I lost because of TIMEOUT (" << timerLimitsBound[t] << " frames exceed " << timerLimits[t] << " ms/frame) " << std::endl;
				}
			}
		}

		// 자동 패배조건 만족 상황이 일정시간 동안 지속되었으면 게임 패배로 처리
		if (isTimeOutConditionSatisfied) {

			Broodwar->drawTextScreen(250, 100, "I lost because of TIMEOUT");

			if (Broodwar->getFrameCount() - timeOutConditionSatisfiedFrame >= maxDurationForTimeOutLostCondition) {

				// 빌드서버에서는 실제로 게임을 종료시켜버립니다
				//Broodwar->leaveGame();
			}
		}


	}
}

// BasicBot 1.1 Patch End //////////////////////////////////////////////////
