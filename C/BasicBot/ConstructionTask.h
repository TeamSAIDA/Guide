#pragma once

#include "Common.h"

namespace MyBot
{
	/// 건물 건설 Construction Task 의 진행 상태
	namespace ConstructionStatus
	{
		enum { 
			Unassigned = 0,				///< Construction 일꾼이 미지정 되어있는 상태
			Assigned = 1,				///< Construction 일꾼이 지정 되었지만, Construction 일꾼이 건설을 착수하지는 않은 상태
			UnderConstruction = 2		///< Construction 일꾼이 지정 되어 건설 작업을 하고있는 상태
		};
	}

	/// 건물 건설 Construction Task 자료구조
	class ConstructionTask 
	{     
	public:
    
		/// 건물의 타입
		BWAPI::UnitType         type;

		/// 건물을 지으려고 계획한 위치.<br>
		/// 일꾼이 건물을 지으러 가는 도중 해당 위치에 장애물이 있게 되는등 문제가 생기면 이 위치를 중심으로 다시 건물 지을 위치를 탐색해서 정합니다
		BWAPI::TilePosition     desiredPosition;

		/// 건물이 실제로 건설되는 위치. 
		BWAPI::TilePosition     finalPosition;
		
		/// 건물 건설 Construction Task 의 진행 상태
		size_t                  status;

		/// 해당 건물의 건설 Construction Task 를 받은 일꾼 유닛
		BWAPI::Unit             constructionWorker;
		
		/// 해당 건물의 건설 Construction 을 실행하는 유닛.<br>
		/// buildingUnit 값은 처음에 nullptr 로 세팅되고, construction 이 시작되어 isBeingConstructed, underConstrunction 상태가 되어야 비로소 값이 채워진다
		BWAPI::Unit             buildingUnit;

		/// 해당 건물의 건설 Construction Task 를 받은 일꾼 유닛에게 build 명령을 지시하였는지 여부.<br>
		/// 한번도 안가본 타일에는 build 명령을 내릴 수 없으므로, 일단 buildCommandGiven = false 인 상태로 일꾼을 해당 타일 위치로 이동시킨 후, <br>
		/// 일꾼이 해당 타일 위치 근처로 오면 buildCommand 지시를 합니다
		bool                    buildCommandGiven;

		/// 해당 건물의 건설 Construction Task 를 받은 일꾼 유닛에게 build 명령을 지시한 시각
		int                     lastBuildCommandGivenFrame;

		/// 해당 건물의 건설 Construction Task 를 최근에 받았던 일꾼 유닛의 ID.<br>
		/// 일꾼 유닛이 Construction Task 를 받았지만 실제 수행은 못하는 상태일 경우, 새롭게 일꾼 유닛을 선정해서 Construction Task 를 부여하는데, <br>
		/// 매번 똑같은 일꾼 유닛이 Construction Task 를 받지 않게 하기 위해서 관리
		int                     lastConstructionWorkerID;

		/// Construction Task 가 건설 작업 시작했는가 여부
		bool                    underConstruction;

		ConstructionTask() 
			: desiredPosition   (BWAPI::TilePositions::None)
			, finalPosition     (BWAPI::TilePositions::None)
			, type              (BWAPI::UnitTypes::Unknown)
			, buildingUnit      (nullptr)
			, constructionWorker       (nullptr)
			, lastBuildCommandGivenFrame(0)
			, lastConstructionWorkerID(0)
			, status            (ConstructionStatus::Unassigned)
			, buildCommandGiven (false)
			, underConstruction (false) 
		{} 

		ConstructionTask(BWAPI::UnitType t, BWAPI::TilePosition _desiredPosition)
			: desiredPosition   (_desiredPosition)
			, finalPosition     (BWAPI::TilePositions::None)
			, type              (t)
			, buildingUnit      (nullptr)
			, constructionWorker       (nullptr)
			, lastBuildCommandGivenFrame(0)
			, lastConstructionWorkerID(0)
			, status            (ConstructionStatus::Unassigned)
			, buildCommandGiven (false)
			, underConstruction (false) 
		{}

		// equals operator
		bool operator==(const ConstructionTask & b) 
		{
			if (b.type == this->type 
				&& b.desiredPosition.x == this->desiredPosition.x && b.desiredPosition.y == this->desiredPosition.y) {
				// buildings are equal if their worker unit or building unit are equal
				return (b.buildingUnit == buildingUnit) || (b.constructionWorker == constructionWorker);
			}
			else {
				return false;
			}
		}
	};
}