#pragma once

#include "Common.h"
#include "MetaType.h"

namespace MyBot
{
	/// 빌드 명령
	struct BuildOrderItem
	{
		MetaType			metaType;		///< the thing we want to 'build'
		int					priority;		///< the priority at which to place it in the queue
		bool				blocking;		///< whether or not we block further items
		BWAPI::TilePosition seedLocation;	///< 건설 위치
		int					producerID;		///< producer unitID (건물ID, 유닛ID)

		/// 건설위치 초안 결정 정책.<br>
		/// 향후 적진 길목, 언덕 위 등 추가 가능
		enum SeedPositionStrategy { 
			MainBaseLocation,			///< 아군 베이스
			MainBaseBackYard,			///< 아군 베이스 뒷편
			FirstChokePoint,			///< 아군 첫번째 길목
			FirstExpansionLocation,		///< 아군 첫번째 앞마당
			SecondChokePoint,			///< 아군 두번째 길목
			SeedPositionSpecified		///< 별도 지정 위치
		};
		SeedPositionStrategy		seedLocationStrategy;	///< 건설위치 초안 결정 정책

		/// 건설 위치는 SeedPositionStrategy::MainBaseLocation 을 통해 찾는다
		/// @param _metaType : 빌드 대상 타입
		/// @param _priority : 0 = 가장 낮은 우선순위. 숫자가 클수록 더 높은 우선순위. 큐에 있는 아이템들 중에서 가장 높은 우선순위의 아이템 (우선순위가 높으면 먼저 큐에 넣은 것)이 먼저 생산 진행됨. 
		/// @param _blocking : true = 지금 이것을 생산할 수 없으면, 이것 생산 가능해질때까지 기다림.  false = 지금 이것을 생산을 할 수 없으면 다음것 먼저 생산 실행.
		/// @param _producerID : producerID 를 지정하면 해당 unit 이 빌드를 실행하게 함
		BuildOrderItem(MetaType _metaType, int _priority = 0, bool _blocking = true, int _producerID = -1)
			: metaType(_metaType)
			, priority(_priority)
			, blocking(_blocking)
			, producerID(_producerID)
			, seedLocation(BWAPI::TilePositions::None)
			, seedLocationStrategy(SeedPositionStrategy::MainBaseLocation)
		{
		}

		/// 건설 위치를 seedLocation 주위에서 찾는다
		BuildOrderItem(MetaType _metaType, BWAPI::TilePosition _seedLocation, int _priority = 0, bool _blocking = true, int _producerID = -1)
			: metaType(_metaType)
			, priority(_priority)
			, blocking(_blocking)
			, producerID(_producerID)
			, seedLocation(_seedLocation)
			, seedLocationStrategy(SeedPositionStrategy::SeedPositionSpecified)
		{
		}

		/// 건설 위치를 seedPositionStrategy 를 이용해서 찾는다. 못찾을 경우, 다른 SeedPositionStrategy 로 계속 찾는다
		BuildOrderItem(MetaType _metaType, SeedPositionStrategy _SeedPositionStrategy, int _priority = 0, bool _blocking = true, int _producerID = -1)
			: metaType(_metaType)
			, priority(_priority)
			, blocking(_blocking)
			, producerID(_producerID)
			, seedLocation(BWAPI::TilePositions::None)
			, seedLocationStrategy(_SeedPositionStrategy)
		{
		}



		bool operator<(const BuildOrderItem &x) const
		{
			return priority < x.priority;
		}
	};

	/// 빌드 오더 목록 자료구조 class
	class BuildOrderQueue
	{
		/// BuildOrderItem 들을 Double Ended Queue 자료구조로 관리합니다.<br>
		/// lowest priority 인 BuildOrderItem은 front 에, highest priority 인 BuildOrderItem 은 back 에 위치하게 합니다
		std::deque< BuildOrderItem >			queue;

		int lowestPriority;
		int highestPriority;
		int defaultPrioritySpacing;

		/// iteration 을 하기 위한 참고값.<br>
		/// highest priority 인 BuildOrderItem 으로부터 몇개나 skip 했는가. 
		int numSkippedItems;

	public:

		BuildOrderQueue();

		/// queues something with a given priority
		void queueItem(BuildOrderItem b);

		/// 빌드오더를 가장 높은 우선순위로 buildQueue 에 추가한다. blocking (다른 것을 생산하지 않고, 이것을 생산 가능하게 될 때까지 기다리는 모드) 기본값은 true 이다
		void queueAsHighestPriority(MetaType				_metaType, bool blocking = true, int _producerID = -1);		
		void queueAsHighestPriority(MetaType                _metaType, BuildOrderItem::SeedPositionStrategy _seedPositionStrategy = BuildOrderItem::SeedPositionStrategy::MainBaseLocation, bool blocking = true);
		void queueAsHighestPriority(MetaType                _metaType, BWAPI::TilePosition _seedPosition, bool blocking = true);
		void queueAsHighestPriority(BWAPI::UnitType         _unitType, BuildOrderItem::SeedPositionStrategy _seedPositionStrategy = BuildOrderItem::SeedPositionStrategy::MainBaseLocation, bool blocking = true);
		void queueAsHighestPriority(BWAPI::UnitType         _unitType, BWAPI::TilePosition _seedPosition, bool blocking = true);
		void queueAsHighestPriority(BWAPI::TechType         _techType, bool blocking = true, int _producerID = -1);
		void queueAsHighestPriority(BWAPI::UpgradeType      _upgradeType, bool blocking = true, int _producerID = -1);

		/// 빌드오더를 가장 낮은 우선순위로 buildQueue 에 추가한다. blocking (다른 것을 생산하지 않고, 이것을 생산 가능하게 될 때까지 기다리는 모드) 기본값은 true 이다
		void queueAsLowestPriority(MetaType				   _metaType, bool blocking = true, int _producerID = -1);
		void queueAsLowestPriority(MetaType                _metaType, BuildOrderItem::SeedPositionStrategy _seedPositionStrategy = BuildOrderItem::SeedPositionStrategy::MainBaseLocation, bool blocking = true);
		void queueAsLowestPriority(MetaType                _metaType, BWAPI::TilePosition _seedPosition, bool blocking = true);
		void queueAsLowestPriority(BWAPI::UnitType         _unitType, BuildOrderItem::SeedPositionStrategy _seedPositionStrategy = BuildOrderItem::SeedPositionStrategy::MainBaseLocation, bool blocking = true);
		void queueAsLowestPriority(BWAPI::UnitType         _unitType, BWAPI::TilePosition _seedPosition, bool blocking = true);
		void queueAsLowestPriority(BWAPI::TechType         _techType, bool blocking = true, int _producerID = -1);
		void queueAsLowestPriority(BWAPI::UpgradeType      _upgradeType, bool blocking = true, int _producerID = -1);

		void clearAll();											///< clears the entire build order queue
		
		size_t size();												///< returns the size of the queue
		bool isEmpty();

		BuildOrderItem getHighestPriorityItem();					///< returns the highest priority item
		void removeHighestPriorityItem();							///< removes the highest priority item

		bool canSkipCurrentItem();
		void skipCurrentItem();										///< increments skippedItems
		void removeCurrentItem();									///< skippedItems 다음의 item 을 제거합니다
		BuildOrderItem getNextItem();								///< returns the highest priority item

		/// BuildOrderQueue에 해당 type 의 Item 이 몇 개 존재하는지 리턴한다. queryTilePosition 을 입력한 경우, 건물에 대해서 추가 탐색한다
		int getItemCount(MetaType                _metaType, BWAPI::TilePosition queryTilePosition = BWAPI::TilePositions::None);
		/// BuildOrderQueue에 해당 type 의 Item 이 몇 개 존재하는지 리턴한다. queryTilePosition 을 입력한 경우, 건물에 대해서 추가 탐색한다
		int getItemCount(BWAPI::UnitType         _unitType, BWAPI::TilePosition queryTilePosition = BWAPI::TilePositions::None);
		int getItemCount(BWAPI::TechType         _techType);
		int getItemCount(BWAPI::UpgradeType      _upgradeType);

		int getHighestPriorityValue();								///< returns the highest priority value


		BuildOrderItem operator [] (int i);							///< overload the bracket operator for ease of use

		std::deque< BuildOrderItem > *			getQueue();
	};
}