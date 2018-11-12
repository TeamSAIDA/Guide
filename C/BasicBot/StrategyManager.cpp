#include "StrategyManager.h"

using namespace MyBot;

StrategyManager & StrategyManager::Instance()
{
	static StrategyManager instance;
	return instance;
}

StrategyManager::StrategyManager()
{
	isFullScaleAttackStarted = false;
	isInitialBuildOrderFinished = false;
}

void StrategyManager::onStart()
{
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 경기 결과 파일 Save / Load 및 로그파일 Save 예제 추가

	// 과거 게임 기록을 로딩합니다
	loadGameRecordList();

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
	
	setInitialBuildOrder();
}

void StrategyManager::onEnd(bool isWinner)
{	
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 경기 결과 파일 Save / Load 및 로그파일 Save 예제 추가

	// 과거 게임 기록 + 이번 게임 기록을 저장합니다
	saveGameRecordList(isWinner);

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

void StrategyManager::update()
{
	if (BuildManager::Instance().buildQueue.isEmpty()) {
		isInitialBuildOrderFinished = true;
	}
		
	executeWorkerTraining();

	executeSupplyManagement();

	executeBasicCombatUnitTraining();

	executeCombat();

	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////

	// 이번 게임의 로그를 남깁니다
	saveGameLog();

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

void StrategyManager::setInitialBuildOrder()
{
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss) {

		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getBasicSupplyProviderUnitType(), BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Gateway, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Zealot);

		/*
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Assimilator, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Forge, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Photon_Cannon, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Gateway, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Zealot);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Cybernetics_Core, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Dragoon);
		// 드라군 사정거리 업그레이드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Singularity_Charge);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Citadel_of_Adun);
		// 질럿 속도 업그레이드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Leg_Enhancements);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Shield_Battery);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Templar_Archives);
		// 하이템플러
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_High_Templar);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_High_Templar);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Psionic_Storm);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Hallucination);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Khaydarin_Amulet);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Archon);

		// 다크아칸
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Dark_Templar);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Dark_Templar);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Maelstrom);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Mind_Control);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Argus_Talisman);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Dark_Archon);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Robotics_Facility);

		// 셔틀
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Shuttle);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Robotics_Support_Bay);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Gravitic_Drive);

		// 리버
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Reaver);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Scarab_Damage);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Reaver_Capacity);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Scarab);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Observatory);
		// 옵저버
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Observer);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Gravitic_Boosters);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Sensor_Array);

		// 공중유닛
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Stargate);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Fleet_Beacon);

		// 스카우트
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Scout);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Apial_Sensors);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Gravitic_Thrusters);

		// 커세어
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Corsair);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Disruption_Web);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Argus_Jewel);

		// 캐리어
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Carrier);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Carrier_Capacity);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Interceptor);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Interceptor);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Interceptor);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Interceptor);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Interceptor);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Interceptor);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Interceptor);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Interceptor);

		// 아비터
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Arbiter_Tribunal);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Protoss_Arbiter);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Recall);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Stasis_Field);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Khaydarin_Core);

		// 포지 - 지상 유닛 업그레이드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Protoss_Ground_Weapons);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Protoss_Plasma_Shields);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Protoss_Ground_Armor);

		// 사이버네틱스코어 - 공중 유닛 업그레이드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Protoss_Air_Weapons);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Protoss_Air_Armor);

		*/
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran) {
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getBasicSupplyProviderUnitType(), BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Barracks, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Marine);

		/*
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Barracks, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Bunker, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Academy);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Comsat_Station);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Marine);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Firebat);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Medic);

		// 가스 리파이너리
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getRefineryBuildingType());
		
		// 지상유닛 업그레이드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Engineering_Bay);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Terran_Infantry_Weapons);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Terran_Infantry_Armor);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Missile_Turret);

		// 마린 스팀팩
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Stim_Packs);
		// 마린 사정거리 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::U_238_Shells);

		// 메딕
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Optical_Flare);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Restoration);
		// 메딕 에너지 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Caduceus_Reactor);

		// 팩토리
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Factory);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Machine_Shop);
		// 벌쳐 스파이더 마인
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Spider_Mines);
		// 벌쳐 이동속도 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Ion_Thrusters);
		// 시즈탱크 시즈모드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Tank_Siege_Mode);

		// 벌쳐
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Vulture);

		// 시즈탱크
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode);

		// 아머니
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Armory);
		// 지상 메카닉 유닛 업그레이드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Terran_Vehicle_Plating);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Terran_Vehicle_Weapons);
		// 공중 유닛 업그레이드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Terran_Ship_Plating);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Terran_Ship_Weapons);
		// 골리앗 사정거리 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Charon_Boosters);

		// 골리앗
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Goliath);

		// 스타포트
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Starport);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Control_Tower);
		// 레이쓰 클러킹
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Cloaking_Field);
		// 레이쓰 에너지 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Apollo_Reactor);

		// 레이쓰
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Wraith);

		// 발키리
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Valkyrie);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Command_Center);

		// 사이언스 퍼실리티
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Science_Facility);
		// 사이언스 베슬 - 기술
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Irradiate);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::EMP_Shockwave);
		// 사이언스 베슬 에너지 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Titan_Reactor);

		// 사이언스 베슬
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Science_Vessel);
		// 사이언스 퍼실리티 - 배틀크루저 생산 가능
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Physics_Lab);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Yamato_Gun);
		// 배틀크루저 에너지 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Colossus_Reactor);
		// 배틀크루저
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Battlecruiser);

		// 사이언스 퍼실리티 - 고스트 생산 가능
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Science_Facility);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Covert_Ops);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Lockdown);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Personnel_Cloaking);
		// 고스트 가시거리 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Ocular_Implants);
		// 고스트 에너지 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Moebius_Reactor);

		// 고스트
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Ghost);

		// 핵폭탄
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Command_Center);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Nuclear_Silo);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Terran_Nuclear_Missile);
		*/
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) {
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Spawning_Pool);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Zergling);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Zergling);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Zergling);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getBasicSupplyProviderUnitType(), BuildOrderItem::SeedPositionStrategy::MainBaseLocation);

		/*		
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Hatchery, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Hatchery, BuildOrderItem::SeedPositionStrategy::FirstExpansionLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Hatchery, BuildOrderItem::SeedPositionStrategy::FirstExpansionLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());

		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getBasicSupplyProviderUnitType(), BuildOrderItem::SeedPositionStrategy::MainBaseLocation);

		// 가스 익스트랙터
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Extractor, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);

		// 성큰 콜로니
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Creep_Colony, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Sunken_Colony, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getRefineryBuildingType());

		// 저글링 이동속도 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Metabolic_Boost);

		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());
		BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getWorkerType());

		// 에볼루션 챔버
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Evolution_Chamber);
		// 에볼루션 챔버 -> 지상유닛 업그레이드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Zerg_Melee_Attacks, false);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Zerg_Missile_Attacks, false);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Zerg_Carapace, false);

		// 스포어 코로니
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Creep_Colony, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Spore_Colony, BuildOrderItem::SeedPositionStrategy::MainBaseLocation);

		// 히드라
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Hydralisk_Den);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Hydralisk);

		// 레어
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Lair);

		// 오버로드 운반가능
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Ventral_Sacs);
		// 오버로드 시야 증가
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Antennae);
		// 오버로드 속도 증가
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Pneumatized_Carapace);

		// 히드라 이동속도 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Muscular_Augments, false);
		// 히드라 공격 사정거리 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Grooved_Spines, false);

		// 럴커
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Lurker_Aspect);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Hydralisk);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Lurker);

		// 스파이어
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Spire);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Mutalisk);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Scourge);

		// 스파이어 -> 공중유닛 업그레이드
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks, false);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace, false);

		// 퀸
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Queens_Nest);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Queen);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Spawn_Broodlings, false);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Ensnare, false);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Gamete_Meiosis, false);

		// 하이브
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Hive);
		// 저글링 공격 속도 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Adrenal_Glands, false);

		// 스파이어 -> 그레이트 스파이어
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Greater_Spire);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Guardian);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Mutalisk);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Devourer);

		// 울트라리스크
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Ultralisk);
		// 울트라리스크 이동속도 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Anabolic_Synthesis, false);
		// 울트라리스크 방어력 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Chitinous_Plating, false);

		// 디파일러
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Defiler_Mound);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Defiler);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Consume, false);
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::TechTypes::Plague, false);
		// 디파일러 에너지 업
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UpgradeTypes::Metasynaptic_Node, false);


		// 나이더스 캐널
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Nydus_Canal);
		
		// 참고로, Zerg_Nydus_Canal 건물로부터 Nydus Canal Exit를 만드는 방법은 다음과 같습니다
		//
		//if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Nydus_Canal) > 0) {
		//	for (auto & unit : BWAPI::Broodwar->self()->getUnits()) {
		//		if (unit->getType() == BWAPI::UnitTypes::Zerg_Nydus_Canal) {
		//			BWAPI::TilePosition targetTilePosition(unit->getTilePosition().x + 6, unit->getTilePosition().y); // Creep 이 있는 곳이어야 한다
		//			unit->build(BWAPI::UnitTypes::Zerg_Nydus_Canal, targetTilePosition);
		//		}
		//	}
		//}

		// 퀸 - 인페스티드 테란 : 테란 Terran_Command_Center 건물의 HitPoint가 낮을 때, 퀸을 들여보내서 Zerg_Infested_Command_Center 로 바꾸면, 그 건물에서 실행 됨
		BuildManager::Instance().buildQueue.queueAsLowestPriority(BWAPI::UnitTypes::Zerg_Infested_Terran);
		*/
	}
}

// 일꾼 계속 추가 생산
void StrategyManager::executeWorkerTraining()
{
	// InitialBuildOrder 진행중에는 아무것도 하지 않습니다
	if (isInitialBuildOrderFinished == false) {
		return;
	}

	if (BWAPI::Broodwar->self()->minerals() >= 50) {
		// workerCount = 현재 일꾼 수 + 생산중인 일꾼 수
		int workerCount = BWAPI::Broodwar->self()->allUnitCount(InformationManager::Instance().getWorkerType());

		if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) {

			for (auto & unit : BWAPI::Broodwar->self()->getUnits())
			{
				if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg) {
					// Zerg_Egg 에게 morph 명령을 내리면 isMorphing = true, isBeingConstructed = true, isConstructing = true 가 된다
					// Zerg_Egg 가 다른 유닛으로 바뀌면서 새로 만들어진 유닛은 잠시 isBeingConstructed = true, isConstructing = true 가 되었다가, 
					if (unit->isMorphing() && unit->getBuildType() == BWAPI::UnitTypes::Zerg_Drone) {
						workerCount++;
					}
				}
			}
		}
		else {
			for (auto & unit : BWAPI::Broodwar->self()->getUnits())
			{
				if (unit->getType().isResourceDepot())
				{
					if (unit->isTraining()) {
						workerCount += unit->getTrainingQueue().size();
					}
				}
			}
		}

		if (workerCount < 30) {
			for (auto & unit : BWAPI::Broodwar->self()->getUnits())
			{
				if (unit->getType().isResourceDepot())
				{
					if (unit->isTraining() == false || unit->getLarva().size() > 0) {

						// 빌드큐에 일꾼 생산이 1개는 있도록 한다
						if (BuildManager::Instance().buildQueue.getItemCount(InformationManager::Instance().getWorkerType()) == 0) {
							//std::cout << "worker enqueue" << std::endl;
							BuildManager::Instance().buildQueue.queueAsLowestPriority(MetaType(InformationManager::Instance().getWorkerType()), false);
						}
					}
				}
			}
		}
	}
}

// Supply DeadLock 예방 및 SupplyProvider 가 부족해질 상황 에 대한 선제적 대응으로서 SupplyProvider를 추가 건설/생산한다
void StrategyManager::executeSupplyManagement()
{
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 가이드 추가 및 콘솔 출력 명령 주석 처리

	// InitialBuildOrder 진행중 혹은 그후라도 서플라이 건물이 파괴되어 데드락이 발생할 수 있는데, 이 상황에 대한 해결은 참가자께서 해주셔야 합니다.
	// 오버로드가 학살당하거나, 서플라이 건물이 집중 파괴되는 상황에 대해  무조건적으로 서플라이 빌드 추가를 실행하기 보다 먼저 전략적 대책 판단이 필요할 것입니다

	// BWAPI::Broodwar->self()->supplyUsed() > BWAPI::Broodwar->self()->supplyTotal()  인 상황이거나
	// BWAPI::Broodwar->self()->supplyUsed() + 빌드매니저 최상단 훈련 대상 유닛의 unit->getType().supplyRequired() > BWAPI::Broodwar->self()->supplyTotal() 인 경우
	// 서플라이 추가를 하지 않으면 더이상 유닛 훈련이 안되기 때문에 deadlock 상황이라고 볼 수도 있습니다.
	// 저그 종족의 경우 일꾼을 건물로 Morph 시킬 수 있기 때문에 고의적으로 이런 상황을 만들기도 하고, 
	// 전투에 의해 유닛이 많이 죽을 것으로 예상되는 상황에서는 고의적으로 서플라이 추가를 하지 않을수도 있기 때문에
	// 참가자께서 잘 판단하셔서 개발하시기 바랍니다.

	// InitialBuildOrder 진행중에는 아무것도 하지 않습니다
	if (isInitialBuildOrderFinished == false) {
		return;
	}

	// 1초에 한번만 실행
	if (BWAPI::Broodwar->getFrameCount() % 24 != 0) {
		return;
	}

	// 게임에서는 서플라이 값이 200까지 있지만, BWAPI 에서는 서플라이 값이 400까지 있다
	// 저글링 1마리가 게임에서는 서플라이를 0.5 차지하지만, BWAPI 에서는 서플라이를 1 차지한다
	if (BWAPI::Broodwar->self()->supplyTotal() <= 400)
	{
		// 서플라이가 다 꽉찼을때 새 서플라이를 지으면 지연이 많이 일어나므로, supplyMargin (게임에서의 서플라이 마진 값의 2배)만큼 부족해지면 새 서플라이를 짓도록 한다
		// 이렇게 값을 정해놓으면, 게임 초반부에는 서플라이를 너무 일찍 짓고, 게임 후반부에는 서플라이를 너무 늦게 짓게 된다
		int supplyMargin = 12;

		// currentSupplyShortage 를 계산한다
		int currentSupplyShortage = BWAPI::Broodwar->self()->supplyUsed() + supplyMargin - BWAPI::Broodwar->self()->supplyTotal();

		if (currentSupplyShortage > 0) {

			// 생산/건설 중인 Supply를 센다
			int onBuildingSupplyCount = 0;

			// 저그 종족인 경우, 생산중인 Zerg_Overlord (Zerg_Egg) 를 센다. Hatchery 등 건물은 세지 않는다
			if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) {
				for (auto & unit : BWAPI::Broodwar->self()->getUnits())
				{					
					if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg && unit->getBuildType() == BWAPI::UnitTypes::Zerg_Overlord) {
						onBuildingSupplyCount += BWAPI::UnitTypes::Zerg_Overlord.supplyProvided();
					}
					// 갓태어난 Overlord 는 아직 SupplyTotal 에 반영안되어서, 추가 카운트를 해줘야함 
					if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord && unit->isConstructing()) {
						onBuildingSupplyCount += BWAPI::UnitTypes::Zerg_Overlord.supplyProvided();
					}
				}
			}
			// 저그 종족이 아닌 경우, 건설중인 Protoss_Pylon, Terran_Supply_Depot 를 센다. Nexus, Command Center 등 건물은 세지 않는다
			else {
				onBuildingSupplyCount += ConstructionManager::Instance().getConstructionQueueItemCount(InformationManager::Instance().getBasicSupplyProviderUnitType()) * InformationManager::Instance().getBasicSupplyProviderUnitType().supplyProvided();
			}

			// 주석처리
			//std::cout << "currentSupplyShortage : " << currentSupplyShortage << " onBuildingSupplyCount : " << onBuildingSupplyCount << std::endl;

			if (currentSupplyShortage > onBuildingSupplyCount) {

				// BuildQueue 최상단에 SupplyProvider 가 있지 않으면 enqueue 한다
				bool isToEnqueue = true;
				if (!BuildManager::Instance().buildQueue.isEmpty()) {
					BuildOrderItem currentItem = BuildManager::Instance().buildQueue.getHighestPriorityItem();
					if (currentItem.metaType.isUnit() && currentItem.metaType.getUnitType() == InformationManager::Instance().getBasicSupplyProviderUnitType()) {
						isToEnqueue = false;
					}
				}
				if (isToEnqueue) {
					// 주석처리
					//std::cout << "enqueue supply provider " << InformationManager::Instance().getBasicSupplyProviderUnitType().getName().c_str() << std::endl;
					BuildManager::Instance().buildQueue.queueAsHighestPriority(MetaType(InformationManager::Instance().getBasicSupplyProviderUnitType()), true);
				}
			}

		}
	}
	// BasicBot 1.1 Patch End ////////////////////////////////////////////////
}

void StrategyManager::executeBasicCombatUnitTraining()
{
	// InitialBuildOrder 진행중에는 아무것도 하지 않습니다
	if (isInitialBuildOrderFinished == false) {
		return;
	}

	// 기본 병력 추가 훈련
	if (BWAPI::Broodwar->self()->minerals() >= 200 && BWAPI::Broodwar->self()->supplyUsed() < 390) {
		{
			for (auto & unit : BWAPI::Broodwar->self()->getUnits())
			{
				if (unit->getType() == InformationManager::Instance().getBasicCombatBuildingType()) {
					if (unit->isTraining() == false || unit->getLarva().size() > 0) {
						if (BuildManager::Instance().buildQueue.getItemCount(InformationManager::Instance().getBasicCombatUnitType()) == 0) {
							BuildManager::Instance().buildQueue.queueAsLowestPriority(InformationManager::Instance().getBasicCombatUnitType());
						}
					}
				}
			}
		}
	}
}


void StrategyManager::executeCombat()
{
	// 공격 모드가 아닐 때에는 전투유닛들을 아군 진영 길목에 집결시켜서 방어
	if (isFullScaleAttackStarted == false)		
	{
		BWTA::Chokepoint* firstChokePoint = BWTA::getNearestChokepoint(InformationManager::Instance().getMainBaseLocation(InformationManager::Instance().selfPlayer)->getTilePosition());

		for (auto & unit : BWAPI::Broodwar->self()->getUnits())
		{
			if (unit->getType() == InformationManager::Instance().getBasicCombatUnitType() && unit->isIdle()) {
				CommandUtil::attackMove(unit, firstChokePoint->getCenter());
			}
		}

		// 전투 유닛이 2개 이상 생산되었고, 적군 위치가 파악되었으면 총공격 모드로 전환
		if (BWAPI::Broodwar->self()->completedUnitCount(InformationManager::Instance().getBasicCombatUnitType()) > 2) {

			if (InformationManager::Instance().enemyPlayer != nullptr
				&& InformationManager::Instance().enemyRace != BWAPI::Races::Unknown
				&& InformationManager::Instance().getOccupiedBaseLocations(InformationManager::Instance().enemyPlayer).size() > 0)
			{				
				isFullScaleAttackStarted = true;
			}
		}

	}
	// 공격 모드가 되면, 모든 전투유닛들을 적군 Main BaseLocation 로 공격 가도록 합니다
	else {
		//std::cout << "enemy OccupiedBaseLocations : " << InformationManager::Instance().getOccupiedBaseLocations(InformationManager::Instance().enemyPlayer).size() << std::endl;
		
		if (InformationManager::Instance().enemyPlayer != nullptr
			&& InformationManager::Instance().enemyRace != BWAPI::Races::Unknown
			&& InformationManager::Instance().getOccupiedBaseLocations(InformationManager::Instance().enemyPlayer).size() > 0)
		{
			// 공격 대상 지역 결정
			BWTA::BaseLocation * targetBaseLocation = nullptr;
			double closestDistance = 100000000;

			for (BWTA::BaseLocation * baseLocation : InformationManager::Instance().getOccupiedBaseLocations(InformationManager::Instance().enemyPlayer)) {

				double distance = BWTA::getGroundDistance(
					InformationManager::Instance().getMainBaseLocation(InformationManager::Instance().selfPlayer)->getTilePosition(), 
					baseLocation->getTilePosition());

				if (distance < closestDistance) {
					closestDistance = distance;
					targetBaseLocation = baseLocation;
				}
			}

			if (targetBaseLocation != nullptr) {

				for (auto & unit : BWAPI::Broodwar->self()->getUnits())
				{
					// 건물은 제외
					if (unit->getType().isBuilding()) continue;
					// 모든 일꾼은 제외
					if (unit->getType().isWorker()) continue;

					// canAttack 유닛은 attackMove Command 로 공격을 보냅니다
					if (unit->canAttack()) {

						if (unit->isIdle()) {
							CommandUtil::attackMove(unit, targetBaseLocation->getPosition());
						}
					}
				}
			}
		}
	}
}

// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
// 경기 결과 파일 Save / Load 및 로그파일 Save 예제 추가
void StrategyManager::loadGameRecordList()
{
	// 과거의 게임에서 bwapi-data\write 폴더에 기록했던 파일은 대회 서버가 bwapi-data\read 폴더로 옮겨놓습니다
	// 따라서, 파일 로딩은 bwapi-data\read 폴더로부터 하시면 됩니다

	// TODO : 파일명은 각자 봇 명에 맞게 수정하시기 바랍니다
	std::string gameRecordFileName = "bwapi-data\\read\\NoNameBot_GameRecord.dat";

	FILE *file;
	errno_t err;
	if ((err = fopen_s(&file, gameRecordFileName.c_str(), "r")) != 0)
	{
		std::cout << "loadGameRecord failed. Could not open file :" << gameRecordFileName.c_str() << std::endl;
	}
	else
	{
		std::cout << "loadGameRecord from file: " << gameRecordFileName.c_str() << std::endl;
		char line[4096];
		while (fgets(line, sizeof line, file) != nullptr)
		{
			std::stringstream ss(line);

			GameRecord tempGameRecord;
			ss >> tempGameRecord.mapName;
			ss >> tempGameRecord.myName;
			ss >> tempGameRecord.myRace;
			ss >> tempGameRecord.myWinCount;
			ss >> tempGameRecord.myLoseCount;
			ss >> tempGameRecord.enemyName;
			ss >> tempGameRecord.enemyRace;
			ss >> tempGameRecord.enemyRealRace;
			ss >> tempGameRecord.gameFrameCount;

			gameRecordList.push_back(tempGameRecord);
		}
		fclose(file);
	}
}

void StrategyManager::saveGameRecordList(bool isWinner)
{
	// 이번 게임의 파일 저장은 bwapi-data\write 폴더에 하시면 됩니다.
	// bwapi-data\write 폴더에 저장된 파일은 대회 서버가 다음 경기 때 bwapi-data\read 폴더로 옮겨놓습니다

	// TODO : 파일명은 각자 봇 명에 맞게 수정하시기 바랍니다
	std::string gameRecordFileName = "bwapi-data\\write\\NoNameBot_GameRecord.dat";

	std::cout << "saveGameRecord to file: " << gameRecordFileName.c_str() << std::endl;

	std::string mapName = BWAPI::Broodwar->mapFileName();
	std::replace(mapName.begin(), mapName.end(), ' ', '_');
	std::string enemyName = BWAPI::Broodwar->enemy()->getName();
	std::replace(enemyName.begin(), enemyName.end(), ' ', '_');
	std::string myName = BWAPI::Broodwar->self()->getName();
	std::replace(myName.begin(), myName.end(), ' ', '_');

	/// 이번 게임에 대한 기록
	GameRecord thisGameRecord;
	thisGameRecord.mapName = mapName;
	thisGameRecord.myName = myName;
	thisGameRecord.myRace = BWAPI::Broodwar->self()->getRace().c_str();
	thisGameRecord.enemyName = enemyName;
	thisGameRecord.enemyRace = BWAPI::Broodwar->enemy()->getRace().c_str();
	thisGameRecord.enemyRealRace = InformationManager::Instance().enemyRace.c_str();
	thisGameRecord.gameFrameCount = BWAPI::Broodwar->getFrameCount();
	if (isWinner) {
		thisGameRecord.myWinCount = 1;
		thisGameRecord.myLoseCount = 0;
	}
	else {
		thisGameRecord.myWinCount = 0;
		thisGameRecord.myLoseCount = 1;
	}
	// 이번 게임 기록을 전체 게임 기록에 추가
	gameRecordList.push_back(thisGameRecord);

	// 전체 게임 기록 write
	std::stringstream ss;
	for (GameRecord gameRecord : gameRecordList) {
		ss << gameRecord.mapName << " "
			<< gameRecord.myName << " "
			<< gameRecord.myRace << " "
			<< gameRecord.myWinCount << " "
			<< gameRecord.myLoseCount << " "
			<< gameRecord.enemyName << " "
			<< gameRecord.enemyRace << " "
			<< gameRecord.enemyRealRace << " "
			<< gameRecord.gameFrameCount << "\n";

	}
	Logger::overwriteToFile(gameRecordFileName, ss.str());
}

void StrategyManager::saveGameLog()
{
	// 100 프레임 (5초) 마다 1번씩 로그를 기록합니다
	// 참가팀 당 용량 제한이 있고, 타임아웃도 있기 때문에 자주 하지 않는 것이 좋습니다
	// 로그는 봇 개발 시 디버깅 용도로 사용하시는 것이 좋습니다
	if (BWAPI::Broodwar->getFrameCount() % 100 != 0) {
		return;
	}

	// TODO : 파일명은 각자 봇 명에 맞게 수정하시기 바랍니다
	std::string gameLogFileName = "bwapi-data\\write\\NoNameBot_LastGameLog.dat";

	std::string mapName = BWAPI::Broodwar->mapFileName();
	std::replace(mapName.begin(), mapName.end(), ' ', '_');
	std::string enemyName = BWAPI::Broodwar->enemy()->getName();
	std::replace(enemyName.begin(), enemyName.end(), ' ', '_');
	std::string myName = BWAPI::Broodwar->self()->getName();
	std::replace(myName.begin(), myName.end(), ' ', '_');

	std::stringstream ss;
	ss << mapName << " "
		<< myName << " "
		<< BWAPI::Broodwar->self()->getRace().c_str() << " "
		<< enemyName << " "
		<< InformationManager::Instance().enemyRace.c_str() << " "
		<< BWAPI::Broodwar->getFrameCount() << " "
		<< BWAPI::Broodwar->self()->supplyUsed() << " "
		<< BWAPI::Broodwar->self()->supplyTotal() << " "
		<< "\n";

	Logger::appendTextToFile(gameLogFileName, ss.str());
}

// BasicBot 1.1 Patch End //////////////////////////////////////////////////
