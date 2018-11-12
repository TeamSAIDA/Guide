#pragma once

#include "Common.h"
#include "InformationManager.h"

namespace MyBot
{
	namespace ScoutStatus
	{
		enum { 
			NoScout=0,						///< 정찰 유닛을 미지정한 상태
			MovingToAnotherBaseLocation=1,	///< 적군의 BaseLocation 이 미발견된 상태에서 정찰 유닛을 이동시키고 있는 상태
			MoveAroundEnemyBaseLocation=2   ///< 적군의 BaseLocation 이 발견된 상태에서 정찰 유닛을 이동시키고 있는 상태
		};
	}

	/// 게임 초반에 일꾼 유닛 중에서 정찰 유닛을 하나 지정하고, 정찰 유닛을 이동시켜 정찰을 수행하는 class<br>
	/// 적군의 BaseLocation 위치를 알아내는 것까지만 개발되어있습니다
	class ScoutManager
	{
		ScoutManager();

		int								currentScoutStatus;

		BWAPI::Unit						currentScoutUnit;

		BWTA::BaseLocation *			currentScoutTargetBaseLocation;
		int								currentScoutTargetDistance;
		
		int                             currentScoutFreeToVertexIndex;		
		std::vector<BWAPI::Position>    enemyBaseRegionVertices;
		BWAPI::Position					currentScoutTargetPosition;
		
		/// 정찰 유닛을 필요하면 새로 지정합니다
		void							assignScoutIfNeeded();

		/// 정찰 유닛을 이동시킵니다
		void                            moveScoutUnit();

		void                            calculateEnemyRegionVertices();
		BWAPI::Position                 getScoutFleePositionFromEnemyRegionVertices();
		int                             getClosestVertexIndex(BWAPI::Unit unit);

	public:
		/// static singleton 객체를 리턴합니다
		static ScoutManager & Instance();

		/// 정찰 유닛을 지정하고, 정찰 상태를 업데이트하고, 정찰 유닛을 이동시킵니다
		void update();

		/// 정찰 유닛을 리턴합니다
		BWAPI::Unit getScoutUnit();

		// 정찰 상태를 리턴합니다
		int getScoutStatus();
		
		/// 정찰 유닛의 이동 목표 BaseLocation 을 리턴합니다
		BWTA::BaseLocation * getScoutTargetBaseLocation();
				
		/// 적군의 Main Base Location 이 있는 Region 의 경계선에 해당하는 Vertex 들의 목록을 리턴합니다
		std::vector<BWAPI::Position> & getEnemyRegionVertices();
	};
}