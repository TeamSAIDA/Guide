#pragma once

#include "Common.h"
#include "BuildOrderQueue.h"
#include "ConstructionManager.h"

namespace MyBot
{
	/// 빌드(건물 건설 / 유닛 훈련 / 테크 리서치 / 업그레이드) 명령을 순차적으로 실행하기 위해 빌드 큐를 관리하고, 빌드 큐에 있는 명령을 하나씩 실행하는 class<br>
	/// 빌드 명령 중 건물 건설 명령은 ConstructionManager로 전달합니다<br>
	/// @see ConstructionManager
	class BuildManager
	{
		BuildManager();

		/// 해당 MetaType 을 build 할 수 있는 producer 를 찾아 반환합니다
		/// @param t 빌드하려는 대상의 타입
		/// @param closestTo 파라메타 입력 시 producer 후보들 중 해당 position 에서 가장 가까운 producer 를 리턴합니다
		/// @param producerID 파라메타 입력 시 해당 ID의 unit 만 producer 후보가 될 수 있습니다
		BWAPI::Unit			getProducer(MetaType t, BWAPI::Position closestTo = BWAPI::Positions::None, int producerID = -1);

		/// 해당 MetaType 을 build 할 수 있는, getProducer 리턴값과 다른 producer 를 찾아 반환합니다<br>
		/// 프로토스 종족 유닛 중 Protoss_Archon / Protoss_Dark_Archon 을 빌드할 때 사용합니다
		BWAPI::Unit			getAnotherProducer(BWAPI::Unit producer, BWAPI::Position closestTo = BWAPI::Positions::None);

		BWAPI::Unit         getClosestUnitToPosition(const BWAPI::Unitset & units, BWAPI::Position closestTo);
		BWAPI::Unit         selectUnitOfType(BWAPI::UnitType type, BWAPI::Position closestTo = BWAPI::Position(0, 0));

		int                 getAvailableMinerals();
		int                 getAvailableGas();
		bool                hasEnoughResources(MetaType type);
		bool                hasNumCompletedUnitType(BWAPI::UnitType type, int num);

		bool                canMakeNow(BWAPI::Unit producer, MetaType t);

		BWAPI::TilePosition getDesiredPosition(BWAPI::UnitType unitType, BWAPI::TilePosition seedPosition, BuildOrderItem::SeedPositionStrategy seedPositionStrategy);

		/// seedPositionStrategy 을 현재 게임상황에 맞게 seedPosition 으로 바꾸어 리턴합니다
		BWAPI::Position		getSeedPositionFromSeedLocationStrategy(BuildOrderItem::SeedPositionStrategy seedPositionStrategy);

		void				checkBuildOrderQueueDeadlockAndAndFixIt();

	public:
		/// static singleton 객체를 리턴합니다
		static BuildManager &	Instance();

		/// buildQueue 에 대해 Dead lock 이 있으면 제거하고, 가장 우선순위가 높은 BuildOrderItem 를 실행되도록 시도합니다
		void				update();

		/// BuildOrderItem 들의 목록을 저장하는 buildQueue 
		BuildOrderQueue     buildQueue;

		/// BuildOrderItem 들의 목록을 저장하는 buildQueue 를 리턴합니다
		BuildOrderQueue *	getBuildQueue();

		/// buildQueue 의 Dead lock 여부를 판단하기 위해, 가장 우선순위가 높은 BuildOrderItem 의 producer 가 존재하게될 것인지 여부를 리턴합니다
		bool				isProducerWillExist(BWAPI::UnitType producerType);

	};


}