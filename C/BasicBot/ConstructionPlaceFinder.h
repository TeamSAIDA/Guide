#pragma once

#include "Common.h"
#include "MapTools.h"
#include "ConstructionTask.h"
#include "MetaType.h"
#include "BuildOrderQueue.h"
#include "InformationManager.h"

namespace MyBot
{
	/// 건설위치 탐색 방법
	namespace ConstructionPlaceSearchMethod
	{
		enum { 
			SpiralMethod = 0,	///< 나선형으로 돌아가며 탐색
			NewMethod = 1		///< 예비
		};
	}

	/// 건설위치 탐색을 위한 class
	class ConstructionPlaceFinder
	{
		ConstructionPlaceFinder();

		/// 건물 건설 예정 타일을 저장해놓기 위한 2차원 배열<br>
		/// TilePosition 단위이기 때문에 보통 128*128 사이즈가 된다<br>
		/// 참고로, 건물이 이미 지어진 타일은 저장하지 않는다
		std::vector< std::vector<bool> > _reserveMap;

		/// BaseLocation 과 Mineral / Geyser 사이의 타일들을 담는 자료구조. 여기에는 Addon 이외에는 건물을 짓지 않도록 합니다
		std::set< BWAPI::TilePosition > _tilesToAvoid;

		/// BaseLocation 과 Mineral / Geyser 사이의 타일들을 찾아 _tilesToAvoid 에 저장합니다<br>
		/// BaseLocation 과 Geyser 사이, ResourceDepot 건물과 Mineral 사이 공간으로 건물 건설 장소를 정하면 <br>
		/// 일꾼 유닛들이 장애물이 되어서 건설 시작되기까지 시간이 오래걸리고, 지어진 건물이 장애물이 되어서 자원 채취 속도도 느려지기 때문에, 이 공간은 건물을 짓지 않는 공간으로 두기 위함입니다
		void					setTilesToAvoid();
		
		/// 해당 buildingType 이 건설될 수 있는 위치를 desiredPosition 근처에서 탐색해서 탐색결과를 리턴합니다<br>
		/// buildingGapSpace를 반영해서 canBuildHereWithSpace 를 사용해서 체크.<br>
		/// 못찾는다면 BWAPI::TilePositions::None 을 리턴합니다<br>
		/// TODO 과제 : 건물을 계획없이 지을수 있는 곳에 짓는 것을 계속 하다보면, 유닛이 건물 사이에 갇히는 경우가 발생할 수 있는데, 이를 방지하는 방법은 생각해볼 과제입니다
		BWAPI::TilePosition		getBuildLocationNear(BWAPI::UnitType buildingType, BWAPI::TilePosition desiredPosition, int buildingGapSpace, size_t constructionPlaceSearchMethod) const;

	public:

		/// static singleton 객체를 리턴합니다
		static ConstructionPlaceFinder & Instance();


		/// seedPosition 및 seedPositionStrategy 파라메터를 활용해서 건물 건설 가능 위치를 탐색해서 리턴합니다<br>
		/// seedPosition 주위에서 가능한 곳을 선정하거나, seedPositionStrategy 에 따라 지형 분석결과 해당 지점 주위에서 가능한 곳을 선정합니다<br>
		/// seedPosition, seedPositionStrategy 을 입력하지 않으면, MainBaseLocation 주위에서 가능한 곳을 리턴합니다
		BWAPI::TilePosition		getBuildLocationWithSeedPositionAndStrategy(BWAPI::UnitType buildingType, BWAPI::TilePosition seedPosition = BWAPI::TilePositions::None, BuildOrderItem::SeedPositionStrategy seedPositionStrategy = BuildOrderItem::SeedPositionStrategy::MainBaseLocation) const;

		/// desiredPosition 근처에서 건물 건설 가능 위치를 탐색해서 리턴합니다<br>
		/// desiredPosition 주위에서 가능한 곳을 찾아 반환합니다<br>
		/// desiredPosition 이 valid 한 곳이 아니라면, desiredPosition 를 MainBaseLocation 로 해서 주위를 찾는다<br>
		/// Returns a suitable TilePosition to build a given building type near specified TilePosition aroundTile.<br>
		/// Returns BWAPI::TilePositions::None, if suitable TilePosition is not exists (다른 유닛들이 자리에 있어서, Pylon, Creep, 건물지을 타일 공간이 전혀 없는 경우 등)
		BWAPI::TilePosition		getBuildLocationNear(BWAPI::UnitType buildingType, BWAPI::TilePosition desiredPosition) const;

		/// seedPosition 근처에서 Refinery 건물 건설 가능 위치를 탐색해서 리턴합니다<br>
		/// 지도상의 여러 가스 광산 (Resource_Vespene_Geyser) 중 예약되어있지 않은 곳(isReservedTile), 다른 섬이 아닌 곳, 이미 Refinery 가 지어져있지않은 곳 중 <br>
		/// seedPosition 과 가장 가까운 곳을 리턴합니다
		BWAPI::TilePosition		getRefineryPositionNear(BWAPI::TilePosition seedPosition = BWAPI::TilePositions::None) const;
		
		bool					isBuildableTile(const ConstructionTask & b,int x,int y) const;		///< 건물 건설 가능 타일인지 여부를 리턴합니다
		void					reserveTiles(BWAPI::TilePosition position, int width, int height);	///< 건물 건설 예정 타일로 예약해서, 다른 건물을 중복해서 짓지 않도록 합니다
		void					freeTiles(BWAPI::TilePosition position, int width, int height);		///< 건물 건설 예정 타일로 예약했던 것을 해제합니다
		bool					isReservedTile(int x, int y) const;
		std::vector< std::vector<bool> > & getReserveMap();											///< reserveMap을 리턴합니다

		
		std::set< BWAPI::TilePosition > & getTilesToAvoid();										///< BaseLocation 과 Mineral / Geyser 사이의 타일들의 목록을 리턴합니다		
		bool					isTilesToAvoid(int x, int y) const;									///< (x, y) 가 BaseLocation 과 Mineral / Geyser 사이의 타일에 해당하는지 여부를 리턴합니다

		/// 해당 위치가 BaseLocation 과 겹치는지 여부를 리턴합니다<br>
		/// BaseLocation 에는 ResourceDepot 건물만 건설하고, 다른 건물은 건설하지 않기 위함입니다
		bool					isOverlapsWithBaseLocation(BWAPI::TilePosition tile,BWAPI::UnitType type) const;	
		
		/// 해당 위치에 건물 건설이 가능한지 여부를 리턴합니다<br>
		/// Broodwar 의 canBuildHere 및 _reserveMap 와 isOverlapsWithBaseLocation 을 체크합니다
		bool					canBuildHere(BWAPI::TilePosition position, const ConstructionTask & b) const;

		/// 해당 위치에 건물 건설이 가능한지 여부를 buildingGapSpace 조건을 포함해서 판단하여 리턴합니다<br>
		/// Broodwar 의 canBuildHere, isBuildableTile, isReservedTile 를 체크합니다
		bool					canBuildHereWithSpace(BWAPI::TilePosition position, const ConstructionTask & b, int buildingGapSpace = 2) const;

	};
}