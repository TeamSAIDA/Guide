#include "Common.h"
#include "UnitData.h"

using namespace MyBot;

UnitData::UnitData() 
	: mineralsLost(0)
	, gasLost(0)
{
	int maxTypeID(0);
	for (const BWAPI::UnitType & t : BWAPI::UnitTypes::allUnitTypes())
	{
		maxTypeID = maxTypeID > t.getID() ? maxTypeID : t.getID();
	}

	numDeadUnits	    = std::vector<int>(maxTypeID + 1, 0);
	numCreatedUnits = std::vector<int>(maxTypeID + 1, 0);
	numUnits = std::vector<int>(maxTypeID + 1, 0);
}

void UnitData::updateUnitInfo(BWAPI::Unit unit)
{
	if (!unit) { return; }

    bool firstSeen = false;
    auto & it = unitAndUnitInfoMap.find(unit);
    if (it == unitAndUnitInfoMap.end())
    {
        firstSeen = true;
        unitAndUnitInfoMap[unit] = UnitInfo();
    }
    
	UnitInfo & ui   = unitAndUnitInfoMap[unit];
    ui.unit         = unit;
    ui.player       = unit->getPlayer();
	ui.lastPosition = unit->getPosition();
	ui.lastHealth   = unit->getHitPoints();
    ui.lastShields  = unit->getShields();
	ui.unitID       = unit->getID();
	ui.type         = unit->getType();
    ui.completed    = unit->isCompleted();

    if (firstSeen)
    {
		numCreatedUnits[unit->getType().getID()]++;
		numUnits[unit->getType().getID()]++;
	}
}

void UnitData::removeUnit(BWAPI::Unit unit)
{
	if (!unit) { return; }

	mineralsLost += unit->getType().mineralPrice();
	gasLost += unit->getType().gasPrice();
	numUnits[unit->getType().getID()]--;
	numDeadUnits[unit->getType().getID()]++;
		
	unitAndUnitInfoMap.erase(unit);
}

void UnitData::removeBadUnits()
{
	for (auto iter(unitAndUnitInfoMap.begin()); iter != unitAndUnitInfoMap.end();)
	{
		if (isBadUnitInfo(iter->second))
		{
			numUnits[iter->second.type.getID()]--;
			iter = unitAndUnitInfoMap.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

const bool UnitData::isBadUnitInfo(const UnitInfo & ui) const
{
    if (!ui.unit)
    {
        return false;
    }

	// Cull away any refineries/assimilators/extractors that were destroyed and reverted to vespene geysers
	if (ui.unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser)
	{ 
		return true;
	}

	// If the unit is a building and we can currently see its position and it is not there
	if (ui.type.isBuilding() && BWAPI::Broodwar->isVisible(ui.lastPosition.x/32, ui.lastPosition.y/32) && !ui.unit->isVisible())
	{
		return true;
	}

	return false;
}

int UnitData::getGasLost() const 
{ 
    return gasLost; 
}

int UnitData::getMineralsLost() const 
{ 
    return mineralsLost; 
}

int UnitData::getNumUnits(BWAPI::UnitType t) const 
{ 
    return numUnits[t.getID()]; 
}

int UnitData::getNumDeadUnits(BWAPI::UnitType t) const 
{ 
    return numDeadUnits[t.getID()]; 
}

int UnitData::getNumCreatedUnits(BWAPI::UnitType t) const
{
	return numCreatedUnits[t.getID()];
}

// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
// getUnitAndUnitInfoMap 메소드에 대해 const 제거

std::map<BWAPI::Unit, UnitInfo> & UnitData::getUnitAndUnitInfoMap()
{ 
    return unitAndUnitInfoMap; 
}

// BasicBot 1.1 Patch End //////////////////////////////////////////////////
