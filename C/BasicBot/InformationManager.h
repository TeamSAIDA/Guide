#pragma once

#include "Common.h"
#include "UnitData.h"

namespace MyBot
{
	/// 게임 상황정보 중 일부를 자체 자료구조 및 변수들에 저장하고 업데이트하는 class<br>
	/// 현재 게임 상황정보는 BWAPI::Broodwar 를 조회하여 파악할 수 있지만, 과거 게임 상황정보는 BWAPI::Broodwar 를 통해 조회가 불가능하기 때문에 InformationManager에서 별도 관리하도록 합니다<br>
	/// 또한, BWAPI::Broodwar 나 BWTA 등을 통해 조회할 수 있는 정보이지만 전처리 / 별도 관리하는 것이 유용한 것도 InformationManager에서 별도 관리하도록 합니다
	class InformationManager 
	{
		InformationManager();

		/// Player - UnitData(각 Unit 과 그 Unit의 UnitInfo 를 Map 형태로 저장하는 자료구조) 를 저장하는 자료구조 객체<br>
		std::map<BWAPI::Player, UnitData>							_unitData;

		/// 해당 Player의 주요 건물들이 있는 BaseLocation. <br>
		/// 처음에는 StartLocation 으로 지정. mainBaseLocation 내 모든 건물이 파괴될 경우 재지정<br>
		/// 건물 여부를 기준으로 파악하기 때문에 부적절하게 판단할수도 있습니다 
		std::map<BWAPI::Player, BWTA::BaseLocation * >				_mainBaseLocations;

		/// 해당 Player의 mainBaseLocation 이 변경되었는가 (firstChokePoint, secondChokePoint, firstExpansionLocation 를 재지정 했는가)
		std::map<BWAPI::Player, bool>								_mainBaseLocationChanged;

		/// 해당 Player가 점령하고 있는 Region 이 있는 BaseLocation<br>
		/// 건물 여부를 기준으로 파악하기 때문에 부적절하게 판단할수도 있습니다 
		std::map<BWAPI::Player, std::list<BWTA::BaseLocation *> >	_occupiedBaseLocations;

		/// 해당 Player가 점령하고 있는 Region<br>
		/// 건물 여부를 기준으로 파악하기 때문에 부적절하게 판단할수도 있습니다 
		std::map<BWAPI::Player, std::set<BWTA::Region *> >			_occupiedRegions;

		/// 해당 Player의 mainBaseLocation 에서 가장 가까운 ChokePoint
		std::map<BWAPI::Player, BWTA::Chokepoint *>					_firstChokePoint;
		/// 해당 Player의 mainBaseLocation 에서 가장 가까운 BaseLocation
		std::map<BWAPI::Player, BWTA::BaseLocation *>				_firstExpansionLocation;
		/// 해당 Player의 mainBaseLocation 에서 두번째로 가까운 (firstChokePoint가 아닌) ChokePoint<br>
		/// 게임 맵에 따라서, secondChokePoint 는 일반 상식과 다른 지점이 될 수도 있습니다
		std::map<BWAPI::Player, BWTA::Chokepoint *>					_secondChokePoint;
	
		/// 전체 unit 의 정보를 업데이트 합니다 (UnitType, lastPosition, HitPoint 등)
		void                    updateUnitsInfo();

		/// 해당 unit 의 정보를 업데이트 합니다 (UnitType, lastPosition, HitPoint 등)
		void                    updateUnitInfo(BWAPI::Unit unit);
		void                    updateBaseLocationInfo();
		void					updateChokePointAndExpansionLocation();
		void                    updateOccupiedRegions(BWTA::Region * region, BWAPI::Player player);

	public:

		/// static singleton 객체를 리턴합니다
		static InformationManager & Instance();
			
		BWAPI::Player       selfPlayer;		///< 아군 Player		
		BWAPI::Race			selfRace;		///< 아군 Player의 종족		
		BWAPI::Player       enemyPlayer;	///< 적군 Player		
		BWAPI::Race			enemyRace;		///< 적군 Player의 종족  
		
		/// Unit 및 BaseLocation, ChokePoint 등에 대한 정보를 업데이트합니다
		void                    update();

		/// Unit 에 대한 정보를 업데이트합니다
		void					onUnitShow(BWAPI::Unit unit)        { updateUnitInfo(unit); }
		/// Unit 에 대한 정보를 업데이트합니다
		void					onUnitHide(BWAPI::Unit unit)        { updateUnitInfo(unit); }
		/// Unit 에 대한 정보를 업데이트합니다
		void					onUnitCreate(BWAPI::Unit unit)		{ updateUnitInfo(unit); }
		/// Unit 에 대한 정보를 업데이트합니다
		void					onUnitComplete(BWAPI::Unit unit)    { updateUnitInfo(unit); }
		/// Unit 에 대한 정보를 업데이트합니다
		void					onUnitMorph(BWAPI::Unit unit)       { updateUnitInfo(unit); }
		/// Unit 에 대한 정보를 업데이트합니다
		void					onUnitRenegade(BWAPI::Unit unit)    { updateUnitInfo(unit); }
		/// Unit 에 대한 정보를 업데이트합니다 
		/// 유닛이 파괴/사망한 경우, 해당 유닛 정보를 삭제합니다
		void					onUnitDestroy(BWAPI::Unit unit);
			
		
		/// 해당 BaseLocation 에 player의 건물이 존재하는지 리턴합니다
		/// @param baseLocation 대상 BaseLocation
		/// @param player 아군 / 적군
		/// @param radius TilePosition 단위
		bool					hasBuildingAroundBaseLocation(BWTA::BaseLocation * baseLocation, BWAPI::Player player, int radius = 10);
		
		/// 해당 Region 에 해당 Player의 건물이 존재하는지 리턴합니다
		bool					existsPlayerBuildingInRegion(BWTA::Region * region, BWAPI::Player player);		

		/// 해당 Player (아군 or 적군) 가 건물을 건설해서 점령한 Region 목록을 리턴합니다
		std::set<BWTA::Region *> &  getOccupiedRegions(BWAPI::Player player);

		/// 해당 Player (아군 or 적군) 의 건물을 건설해서 점령한 BaseLocation 목록을 리턴합니다		 
		std::list<BWTA::BaseLocation *> & getOccupiedBaseLocations(BWAPI::Player player);

		/// 해당 Player (아군 or 적군) 의 Main BaseLocation 을 리턴합니다		 
		BWTA::BaseLocation *	getMainBaseLocation(BWAPI::Player player);

		/// 해당 Player (아군 or 적군) 의 Main BaseLocation 에서 가장 가까운 ChokePoint 를 리턴합니다		 
		BWTA::Chokepoint *      getFirstChokePoint(BWAPI::Player player);

		/// 해당 Player (아군 or 적군) 의 Main BaseLocation 에서 가장 가까운 Expansion BaseLocation 를 리턴합니다		 
		BWTA::BaseLocation *    getFirstExpansionLocation(BWAPI::Player player);

		/// 해당 Player (아군 or 적군) 의 Main BaseLocation 에서 두번째로 가까운 ChokePoint 를 리턴합니다		 
		/// 게임 맵에 따라서, secondChokePoint 는 일반 상식과 다른 지점이 될 수도 있습니다
		BWTA::Chokepoint *      getSecondChokePoint(BWAPI::Player player);



		// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
		// getUnitAndUnitInfoMap 메소드에 대해 const 제거

		/// 해당 Player (아군 or 적군) 의 모든 유닛 목록 (가장 최근값) UnitAndUnitInfoMap 을 리턴합니다<br> 
		/// 파악된 정보만을 리턴하기 때문에 적군의 정보는 틀린 값일 수 있습니다
		UnitAndUnitInfoMap &    getUnitAndUnitInfoMap(BWAPI::Player player);

		// BasicBot 1.1 Patch End //////////////////////////////////////////////////

		/// 해당 Player (아군 or 적군) 의 모든 유닛 통계 UnitData 을 리턴합니다		 
		UnitData &				getUnitData(BWAPI::Player player);


		/// 해당 Player (아군 or 적군) 의 해당 UnitType 유닛 숫자를 리턴합니다 (훈련/건설 중인 유닛 숫자까지 포함)
		int						getNumUnits(BWAPI::UnitType type, BWAPI::Player player);

		/// 해당 Player (아군 or 적군) 의 position 주위의 유닛 목록을 unitInfo 에 저장합니다		 
		void                    getNearbyForce(std::vector<UnitInfo> & unitInfo, BWAPI::Position p, BWAPI::Player player, int radius);

		/// 해당 UnitType 이 전투 유닛인지 리턴합니다
		bool					isCombatUnitType(BWAPI::UnitType type) const;



		// 해당 종족의 UnitType 중 ResourceDepot 기능을 하는 UnitType을 리턴합니다
		BWAPI::UnitType			getBasicResourceDepotBuildingType(BWAPI::Race race = BWAPI::Races::None);

		// 해당 종족의 UnitType 중 Refinery 기능을 하는 UnitType을 리턴합니다
		BWAPI::UnitType			getRefineryBuildingType(BWAPI::Race race = BWAPI::Races::None);

		// 해당 종족의 UnitType 중 SupplyProvider 기능을 하는 UnitType을 리턴합니다
		BWAPI::UnitType			getBasicSupplyProviderUnitType(BWAPI::Race race = BWAPI::Races::None);

		// 해당 종족의 UnitType 중 Worker 에 해당하는 UnitType을 리턴합니다
		BWAPI::UnitType			getWorkerType(BWAPI::Race race = BWAPI::Races::None);

		// 해당 종족의 UnitType 중 Basic Combat Unit 에 해당하는 UnitType을 리턴합니다
		BWAPI::UnitType			getBasicCombatUnitType(BWAPI::Race race = BWAPI::Races::None);

		// 해당 종족의 UnitType 중 Basic Combat Unit 을 생산하기 위해 건설해야하는 UnitType을 리턴합니다
		BWAPI::UnitType			getBasicCombatBuildingType(BWAPI::Race race = BWAPI::Races::None);

		// 해당 종족의 UnitType 중 Advanced Combat Unit 에 해당하는 UnitType을 리턴합니다
		BWAPI::UnitType			getAdvancedCombatUnitType(BWAPI::Race race = BWAPI::Races::None);

		// 해당 종족의 UnitType 중 Observer 에 해당하는 UnitType을 리턴합니다
		BWAPI::UnitType			getObserverUnitType(BWAPI::Race race = BWAPI::Races::None);

		// 해당 종족의 UnitType 중 Basic Depense 기능을 하는 UnitType을 리턴합니다
		BWAPI::UnitType			getBasicDefenseBuildingType(BWAPI::Race race = BWAPI::Races::None);

		// 해당 종족의 UnitType 중 Advanced Depense 기능을 하는 UnitType을 리턴합니다
		BWAPI::UnitType			getAdvancedDefenseBuildingType(BWAPI::Race race = BWAPI::Races::None);
	};
}
