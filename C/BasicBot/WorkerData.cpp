#include "WorkerData.h"
#include "CommandUtil.h"

using namespace MyBot;

WorkerData::WorkerData() 
{
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 멀티 기지간 일꾼 숫자 리밸런싱 조건값 수정 : 미네랄 갯수 * 2 배 초과일 경우 리밸런싱

	mineralAndMineralWorkerRatio = 2;

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////

	for (auto & unit : BWAPI::Broodwar->getAllUnits())
	{
		if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field))
		{
            workersOnMineralPatch[unit] = 0;
		}
	}
}

void WorkerData::workerDestroyed(BWAPI::Unit unit)
{
	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////

	// workers, depotWorkerCount, refineryWorkerCount 등 자료구조에서 사망한 일꾼 정보를 제거합니다
	for (auto it = workers.begin(); it != workers.end(); ) {
		BWAPI::Unit worker = *it;

		if (worker == nullptr ) {			
			it = workers.erase(it);
		}
		else {
			if (worker->getType().isWorker() == false || worker->getPlayer() != BWAPI::Broodwar->self() || worker->exists() == false || worker->getHitPoints() == 0) {
								
				clearPreviousJob(worker);

				// worker의 previousJob 이 잘못 설정되어있는 경우에 대해 정리합니다
				if (workerMineralMap.find(worker) != workerMineralMap.end()) {
					workerMineralMap.erase(worker);
				}
				if (workerDepotMap.find(worker) != workerDepotMap.end()) {
					workerDepotMap.erase(worker);
				}
				if (workerRefineryMap.find(worker) != workerRefineryMap.end()) {
					workerRefineryMap.erase(worker);
				}
				if (workerRepairMap.find(worker) != workerRepairMap.end()) {
					workerRepairMap.erase(worker);
				}
				if (workerMoveMap.find(worker) != workerMoveMap.end()) {
					workerMoveMap.erase(worker);
				}
				if (workerBuildingTypeMap.find(worker) != workerBuildingTypeMap.end()) {
					workerBuildingTypeMap.erase(worker);
				}
				if (workerMineralAssignment.find(worker) != workerMineralAssignment.end()) {
					workerMineralAssignment.erase(worker);
				}
				if (workerJobMap.find(worker) != workerJobMap.end()) {
					workerJobMap.erase(worker);
				}

				// depotWorkerCount 를 다시 셉니다
				for (auto & depot : depots) {
					int count = 0;					
					for (auto & it : workerDepotMap) {
						if (it.second == depot) {
							count ++;
						}
					}
					depotWorkerCount[depot] = count;
				}

				// refineryWorkerCount 를 다시 셉니다
				for (auto & it1 : refineryWorkerCount) {
					int count = 0;
					for (auto & it2 : workerRefineryMap) {
						if (it1.first == it2.second) {
							count ++;
						}
					}
					it1.second = count;
				}

				it = workers.erase(it);
			}
			else {
				++it;
			}
		}
	}

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////

	if (!unit) { return; }

	clearPreviousJob(unit);
	workers.erase(unit);
}

// WorkerJob::Idle 로 일단 추가한다
void WorkerData::addWorker(BWAPI::Unit unit)
{
	if (!unit) { return; }

	workers.insert(unit);
	workerJobMap[unit] = WorkerJob::Idle;
}

void WorkerData::addWorker(BWAPI::Unit unit, WorkerJob job, BWAPI::Unit jobUnit)
{
	if (!unit || !jobUnit) { return; }

	assert(workers.find(unit) == workers.end());

	workers.insert(unit);
	setWorkerJob(unit, job, jobUnit);
}

void WorkerData::addWorker(BWAPI::Unit unit, enum WorkerJob job, BWAPI::UnitType jobUnitType)
{
	if (!unit) { return; }

	assert(workers.find(unit) == workers.end());
	workers.insert(unit);
	setWorkerJob(unit, job, jobUnitType);
}

void WorkerData::addDepot(BWAPI::Unit unit)
{
	if (!unit) { return; }

	assert(depots.find(unit) == depots.end());
	depots.insert(unit);
	depotWorkerCount[unit] = 0;
}

void WorkerData::removeDepot(BWAPI::Unit unit)
{	
	if (!unit) { return; }

	depots.erase(unit);
	depotWorkerCount.erase(unit);

	// re-balance workers in here
	for (auto & worker : workers)
	{
		// if a worker was working at this depot
		if (workerDepotMap[worker] == unit)
		{
			setWorkerJob(worker, Idle, nullptr);
		}
	}
}

BWAPI::Unitset WorkerData::getDepots()
{
	return depots;
}


void WorkerData::addToMineralPatch(BWAPI::Unit unit, int num)
{
    if (workersOnMineralPatch.find(unit) == workersOnMineralPatch.end())
    {
        workersOnMineralPatch[unit] = num;
    }
    else
    {
        workersOnMineralPatch[unit] = workersOnMineralPatch[unit] + num;
    }
}

void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, BWAPI::Unit jobUnit)
{
	if (!unit) { return; }

	/*
	if (job == Idle)
	{
		std::cout << "set worker " << unit->getID() << " job " << workerJobMap[unit] << " -> 4 (idle) " << std::endl;
	}
	*/
	
	clearPreviousJob(unit);
	workerJobMap[unit] = job;

	if (job == Minerals)
	{
		// increase the number of workers assigned to this nexus
		depotWorkerCount[jobUnit] += 1;

		// set the mineral the worker is working on
		workerDepotMap[unit] = jobUnit;

        BWAPI::Unit mineralToMine = getMineralToMine(unit);
        workerMineralAssignment[unit] = mineralToMine;
        addToMineralPatch(mineralToMine, 1);

		// right click the mineral to start mining
		CommandUtil::rightClick(unit, mineralToMine);
	}
	else if (job == Gas)
	{
		// increase the count of workers assigned to this refinery
		refineryWorkerCount[jobUnit] += 1;

		// set the refinery the worker is working on
		workerRefineryMap[unit] = jobUnit;

		// right click the refinery to start harvesting
		CommandUtil::rightClick(unit, jobUnit);
	}
    else if (job == Repair)
    {
        // only SCV can repair
		if (unit->getType() == BWAPI::UnitTypes::Terran_SCV) {

			// set the unit the worker is to repair
			workerRepairMap[unit] = jobUnit;

			// start repairing if it is not repairing 
			// 기존이 이미 수리를 하고 있으면 계속 기존 것을 수리한다
			if (!unit->isRepairing())
			{
				CommandUtil::repair(unit, jobUnit);
			}
		}
    }
	else if (job == Scout)
	{

	}
    else if (job == Build)
    {
    }
}

void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, BWAPI::UnitType jobUnitType)
{
	if (!unit) { return; }

	clearPreviousJob(unit);
	workerJobMap[unit] = job;

	if (job == Build)
	{
		workerBuildingTypeMap[unit] = jobUnitType;
	}
}

void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, WorkerMoveData wmd)
{
	if (!unit) { return; }

	clearPreviousJob(unit);
	workerJobMap[unit] = job;

	if (job == Move)
	{
		workerMoveMap[unit] = wmd;
	}

	if (workerJobMap[unit] != Move)
	{
		//BWAPI::Broodwar->printf("Something went horribly wrong");
	}
}


void WorkerData::clearPreviousJob(BWAPI::Unit unit)
{
	if (!unit) { return; }

	WorkerJob previousJob = getWorkerJob(unit);

	if (previousJob == Minerals)
	{
		depotWorkerCount[workerDepotMap[unit]] -= 1;

		workerDepotMap.erase(unit);

        // remove a worker from this unit's assigned mineral patch
        addToMineralPatch(workerMineralAssignment[unit], -1);

        // erase the association from the map
        workerMineralAssignment.erase(unit);
	}
	else if (previousJob == Gas)
	{
		refineryWorkerCount[workerRefineryMap[unit]] -= 1;
		workerRefineryMap.erase(unit);
	}
	else if (previousJob == Build)
	{
		workerBuildingTypeMap.erase(unit);
	}
	else if (previousJob == Repair)
	{
		workerRepairMap.erase(unit);
	}
	else if (previousJob == Move)
	{
		workerMoveMap.erase(unit);
	}

	workerJobMap.erase(unit);
}

int WorkerData::getNumWorkers() const
{
	return workers.size();
}

int WorkerData::getNumMineralWorkers() const
{
	size_t num = 0;
	for (auto & unit : workers)
	{
		if (workerJobMap.at(unit) == WorkerData::Minerals)
		{
			num++;
		}
	}
	return num;
}

int WorkerData::getNumGasWorkers() const
{
	size_t num = 0;
	for (auto & unit : workers)
	{
		if (workerJobMap.at(unit) == WorkerData::Gas)
		{
			num++;
		}
	}
	return num;
}

int WorkerData::getNumIdleWorkers() const
{
	size_t num = 0;
	for (auto & unit : workers)
	{
		if (workerJobMap.at(unit) == WorkerData::Idle)
		{
			num++;
		}
	}
	return num;
}


enum WorkerData::WorkerJob WorkerData::getWorkerJob(BWAPI::Unit unit)
{
	if (!unit) { return Default; }

	std::map<BWAPI::Unit, enum WorkerJob>::iterator it = workerJobMap.find(unit);

	if (it != workerJobMap.end())
	{
		return it->second;
	}

	return Default;
}

bool WorkerData::depotHasEnoughMineralWorkers(BWAPI::Unit depot)
{
	if (!depot) { return false; }

	int assignedWorkers = getNumAssignedWorkers(depot);
	int mineralsNearDepot = getMineralsNearDepot(depot);

	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// 멀티 기지간 일꾼 숫자 리밸런싱 조건값 수정 : 미네랄 갯수 * 2 배 초과일 경우 리밸런싱

	// 충분한 수의 미네랄 일꾼 수를 얼마로 정할 것인가 : 
	// (근처 미네랄 수) * 1.5배 ~ 2배 정도가 적당
	// 근처 미네랄 수가 8개 라면, 일꾼 8마리여도 좋지만, 12마리면 조금 더 채취가 빠르다. 16마리면 충분하다. 24마리면 너무 많은 숫자이다.
	// 근처 미네랄 수가 0개 인 ResourceDepot은, 이미 충분한 수의 미네랄 일꾼이 꽉 차있는 것이다
	if (assignedWorkers >= (int)(mineralsNearDepot * mineralAndMineralWorkerRatio))
	{
		return true;
	}
	else
	{
		return false;
	}

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

BWAPI::Unitset WorkerData::getMineralPatchesNearDepot(BWAPI::Unit depot)
{
    // if there are minerals near the depot, add them to the set
    BWAPI::Unitset mineralsNearDepot;

	int radius = 320;

    for (auto & unit : BWAPI::Broodwar->getAllUnits())
	{
		if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field) && unit->getDistance(depot) < radius)
		{
            mineralsNearDepot.insert(unit);
		}
	}

    // if we didn't find any, use the whole map
    if (mineralsNearDepot.empty())
    {
        for (auto & unit : BWAPI::Broodwar->getAllUnits())
	    {
		    if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field))
		    {
                mineralsNearDepot.insert(unit);
		    }
	    }
    }

    return mineralsNearDepot;
}

int WorkerData::getMineralsNearDepot(BWAPI::Unit depot)
{
	if (!depot) { return 0; }

	int mineralsNearDepot = 0;

	for (auto & unit : BWAPI::Broodwar->getAllUnits())
	{
		if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field) && unit->getDistance(depot) < 200)
		{
			mineralsNearDepot++;
		}
	}

	return mineralsNearDepot;
}

BWAPI::Unit WorkerData::getWorkerResource(BWAPI::Unit unit)
{
	if (!unit) { return nullptr; }

	// create the iterator
	std::map<BWAPI::Unit, BWAPI::Unit>::iterator it;
	
	// if the worker is mining, set the iterator to the mineral map
	if (getWorkerJob(unit) == Minerals)
	{
		it = workerMineralMap.find(unit);
		if (it != workerMineralMap.end())
		{
			return it->second;
		}	
	}
	else if (getWorkerJob(unit) == Gas)
	{
		it = workerRefineryMap.find(unit);
		if (it != workerRefineryMap.end())
		{
			return it->second;
		}	
	}

	return nullptr;
}


BWAPI::Unit WorkerData::getMineralToMine(BWAPI::Unit worker)
{
	if (!worker) { return nullptr; }


	// get the depot associated with this unit
	BWAPI::Unit depot = getWorkerDepot(worker);
	BWAPI::Unit bestMineral = nullptr;
	double bestDist = 100000000;
    double bestNumAssigned = 10000000;

	if (depot)
	{
        BWAPI::Unitset mineralPatches = getMineralPatchesNearDepot(depot);

		for (auto & mineral : mineralPatches)
		{
				double dist = mineral->getDistance(depot);
                double numAssigned = workersOnMineralPatch[mineral];

                if (numAssigned < bestNumAssigned)
                {
                    bestMineral = mineral;
                    bestDist = dist;
                    bestNumAssigned = numAssigned;
                }
				else if (numAssigned == bestNumAssigned)
				{
					if (dist < bestDist)
                    {
                        bestMineral = mineral;
                        bestDist = dist;
                        bestNumAssigned = numAssigned;
                    }
				}
		
		}
	}

	return bestMineral;
}
/*
BWAPI::Unit WorkerData::getMineralToMine(BWAPI::Unit worker)
{
	if (!worker) { return nullptr; }

	// get the depot associated with this unit
	BWAPI::Unit depot = getWorkerDepot(worker);
	BWAPI::Unit mineral = nullptr;
	double closestDist = 10000;

	if (depot)
	{
		BOOST_FOREACH (BWAPI::Unit unit, BWAPI::Broodwar->getAllUnits())
		{
			if (unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field && unit->getResources() > 0)
			{
				double dist = unit->getDistance(depot);

				if (!mineral || dist < closestDist)
				{
					mineral = unit;
					closestDist = dist;
				}
			}
		}
	}

	return mineral;
}*/

BWAPI::Unit WorkerData::getWorkerRepairUnit(BWAPI::Unit unit)
{
	if (!unit) { return nullptr; }

	std::map<BWAPI::Unit, BWAPI::Unit>::iterator it = workerRepairMap.find(unit);

	if (it != workerRepairMap.end())
	{
		return it->second;
	}	

	return nullptr;
}

BWAPI::Unit WorkerData::getWorkerDepot(BWAPI::Unit unit)
{
	if (!unit) { return nullptr; }

	std::map<BWAPI::Unit, BWAPI::Unit>::iterator it = workerDepotMap.find(unit);

	if (it != workerDepotMap.end())
	{
		return it->second;
	}	

	return nullptr;
}

BWAPI::UnitType	WorkerData::getWorkerBuildingType(BWAPI::Unit unit)
{
	if (!unit) { return BWAPI::UnitTypes::None; }

	std::map<BWAPI::Unit, BWAPI::UnitType>::iterator it = workerBuildingTypeMap.find(unit);

	if (it != workerBuildingTypeMap.end())
	{
		return it->second;
	}	

	return BWAPI::UnitTypes::None;
}

WorkerMoveData WorkerData::getWorkerMoveData(BWAPI::Unit unit)
{
	std::map<BWAPI::Unit, WorkerMoveData>::iterator it = workerMoveMap.find(unit);

	assert(it != workerMoveMap.end());
	
	return (it->second);
}

int WorkerData::getNumAssignedWorkers(BWAPI::Unit unit)
{
	if (!unit) { return 0; }

	std::map<BWAPI::Unit, int>::iterator it;
	
	// if the worker is mining, set the iterator to the mineral map
	if (unit->getType().isResourceDepot())
	{
		it = depotWorkerCount.find(unit);

		// if there is an entry, return it
		if (it != depotWorkerCount.end())
		{
			return it->second;
		}
	}
	else if (unit->getType().isRefinery())
	{
		it = refineryWorkerCount.find(unit);

		// if there is an entry, return it
		if (it != refineryWorkerCount.end())
		{
			return it->second;
		}
		// otherwise, we are only calling this on completed refineries, so set it
		else
		{
			refineryWorkerCount[unit] = 0;
		}
	}

	// when all else fails, return 0
	return 0;
}

char WorkerData::getJobCode(BWAPI::Unit unit)
{
	if (!unit) { return 'X'; }

	WorkerData::WorkerJob j = getWorkerJob(unit);

	if (j == WorkerData::Build) return 'B';
	if (j == WorkerData::Combat) return 'C';
	if (j == WorkerData::Default) return 'D';
	if (j == WorkerData::Gas) return 'G';
	if (j == WorkerData::Idle) return 'I';
	if (j == WorkerData::Minerals) return 'M';
	if (j == WorkerData::Repair) return 'R';
	if (j == WorkerData::Move) return 'O';
	if (j == WorkerData::Scout) return 'S';
	return 'X';
}

void WorkerData::printWorkerJob()
{
	for (auto & it : workerJobMap) {
		std::cout << "worker " << it.first->getID() << " job " << it.second << std::endl;
	}
}