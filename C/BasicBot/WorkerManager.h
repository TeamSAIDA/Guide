#pragma once

#include "Common.h"
#include "WorkerData.h"
#include "ConstructionTask.h"
#include "ConstructionManager.h"
#include "InformationManager.h"

namespace MyBot
{
	/// 일꾼 유닛들의 상태를 관리하고 컨트롤하는 class
	class WorkerManager
	{
		/// 각 Worker 에 대한 WorkerJob 상황을 저장하는 자료구조 객체
		WorkerData  workerData;

		/// 일꾼 중 한명을 Repair Worker 로 정해서, 전체 수리 대상을 하나씩 순서대로 수리합니다
		BWAPI::Unit currentRepairWorker;
				
		void        updateWorkerStatus();

		/// Idle 일꾼을 Mineral 일꾼으로 만듭니다
		/// Mineral worker 숫자가 많이 지정되어있지 않은, 주위에 Mineral 이 있는, 거리가 가까운 Resource Depot 을 찾아서
		/// 그 Resource Depot 근처의 Mineral 을 채취하게 합니다
		void        handleIdleWorkers();

		void        handleGasWorkers();
		void        handleMoveWorkers();
		void        handleCombatWorkers();
		void        handleRepairWorkers();

		/// Resource Depot (센터 건물)에 너무 많은 수의 Mineral worker 들이 지정되어 있다면, 해당 일꾼을 Idle 상태로 만듭니다
		void        rebalanceWorkers();

		WorkerManager();

	public:
		/// static singleton 객체를 리턴합니다
		static WorkerManager &  Instance();

		/// 일꾼 유닛들의 상태를 저장하는 workerData 객체를 업데이트하고, 일꾼 유닛들이 자원 채취 등 임무 수행을 하도록 합니다
		void        update();

		/// 일꾼 유닛들의 상태를 저장하는 workerData 객체를 업데이트합니다
		void        onUnitDestroy(BWAPI::Unit unit);

		/// 일꾼 유닛들의 상태를 저장하는 workerData 객체를 업데이트합니다
		/// 저그 드론 유닛이 건물로 Morph 하다가 취소해서 다시 드론으로 Morph 하게 된 경우에도 호출됩니다
		void        onUnitMorph(BWAPI::Unit unit);

		// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
		// 일꾼 탄생/파괴 등에 대한 업데이트 로직 버그 수정 : onUnitShow 가 아니라 onUnitComplete 에서 처리하도록 수정

		// onUnitShow 메소드 제거
		/// 일꾼 유닛들의 상태를 저장하는 workerData 객체를 업데이트합니다
		//void        onUnitShow(BWAPI::Unit unit);

		// onUnitComplete 메소드 추가
		/// 일꾼 유닛들의 상태를 저장하는 workerData 객체를 업데이트합니다
		/// Terran_SCV, Protoss_Probe 유닛 훈련이 끝나서 탄생할 경우, 
		/// Zerg_Drone 유닛이 탄생하는 경우,
		/// Zerg_Drone 유닛이 건물로 Morph 가 끝나서 건물이 완성되는 경우,
		/// Zerg_Drone 유닛의 Zerg_Extractor 건물로의 Morph 를 취소시켜서 Zerg_Drone 유닛이 새롭게 탄생하는 경우
		/// 호출됩니다
		void        onUnitComplete(BWAPI::Unit unit);

		// BasicBot 1.1 Patch End //////////////////////////////////////////////////

		
		/// 일꾼 유닛들의 상태를 저장하는 workerData 객체를 리턴합니다
		WorkerData  getWorkerData();



		/// 해당 일꾼 유닛 unit 의 WorkerJob 값를 Idle 로 변경합니다
		void        setIdleWorker(BWAPI::Unit unit);
		
		/// idle 상태인 일꾼 유닛 unit 의 숫자를 리턴합니다
		int         getNumIdleWorkers();



		/// 해당 일꾼 유닛 unit 의 WorkerJob 값를 Mineral 로 변경합니다
		void        setMineralWorker(BWAPI::Unit unit);
		int         getNumMineralWorkers();
		bool        isMineralWorker(BWAPI::Unit worker);

		/// target 으로부터 가장 가까운 Mineral 일꾼 유닛을 리턴합니다
		BWAPI::Unit getClosestMineralWorkerTo(BWAPI::Position target);

		/// 해당 일꾼 유닛 unit 으로부터 가장 가까운 ResourceDepot 건물을 리턴합니다
		BWAPI::Unit getClosestResourceDepotFromWorker(BWAPI::Unit worker);


		/// Mineral 일꾼 유닛들 중에서 Gas 임무를 수행할 일꾼 유닛을 정해서 리턴합니다<br>
		/// Idle 일꾼은 Build, Repair, Scout 등 다른 임무에 먼저 투입되어야 하기 때문에 Mineral 일꾼 중에서만 정합니다
		BWAPI::Unit chooseGasWorkerFromMineralWorkers(BWAPI::Unit refinery);
		int         getNumGasWorkers();

		/// Mineral 혹은 Idle 일꾼 유닛들 중에서 Scout 임무를 수행할 일꾼 유닛을 정해서 리턴합니다
		BWAPI::Unit getScoutWorker();
		void        setScoutWorker(BWAPI::Unit worker);
		bool        isScoutWorker(BWAPI::Unit worker);

		/// buildingPosition 에서 가장 가까운 Move 혹은 Idle 혹은 Mineral 일꾼 유닛들 중에서 Construction 임무를 수행할 일꾼 유닛을 정해서 리턴합니다<br>
		/// Move / Idle Worker 중에서 먼저 선정하고, 없으면 Mineral Worker 중에서 선정합니다<br>
		/// 일꾼 유닛이 2개 이상이면, avoidWorkerID 에 해당하는 worker 는 선정하지 않도록 합니다<br>
		/// if setJobAsConstructionWorker is true (default), it will be flagged as a builder unit<br>
		/// if setJobAsConstructionWorker is false, we just want to see which worker will build a building
		BWAPI::Unit chooseConstuctionWorkerClosestTo(BWAPI::UnitType buildingType, BWAPI::TilePosition buildingPosition, bool setJobAsConstructionWorker = true, int avoidWorkerID = 0);
		void        setConstructionWorker(BWAPI::Unit worker, BWAPI::UnitType buildingType);
		bool        isConstructionWorker(BWAPI::Unit worker);

		/// position 에서 가장 가까운 Mineral 혹은 Idle 혹은 Move 일꾼 유닛들 중에서 Repair 임무를 수행할 일꾼 유닛을 정해서 리턴합니다
		BWAPI::Unit chooseRepairWorkerClosestTo(BWAPI::Position p, int maxRange = 100000000);
		void        setRepairWorker(BWAPI::Unit worker, BWAPI::Unit unitToRepair);
		void        stopRepairing(BWAPI::Unit worker);
	
		/// position 에서 가장 가까운 Mineral 혹은 Idle 일꾼 유닛들 중에서 Move 임무를 수행할 일꾼 유닛을 정해서 리턴합니다
		void        setMoveWorker(BWAPI::Unit worker, int m, int g, BWAPI::Position p);
		BWAPI::Unit chooseMoveWorkerClosestTo(BWAPI::Position p);

		/// 해당 일꾼 유닛으로부터 가장 가까운 적군 유닛을 리턴합니다
		BWAPI::Unit getClosestEnemyUnitFromWorker(BWAPI::Unit worker);

		/// 해당 일꾼 유닛에게 Combat 임무를 부여합니다
		void        setCombatWorker(BWAPI::Unit worker);
		/// 모든 Combat 일꾼 유닛에 대해 임무를 해제합니다
		void        stopCombat();

	};
}