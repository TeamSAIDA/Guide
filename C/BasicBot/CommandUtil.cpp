#include "CommandUtil.h"

using namespace MyBot;

void CommandUtil::attackUnit(BWAPI::Unit attacker, BWAPI::Unit target)
{
	if (!attacker || !target)
	{
		return;
	}

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit &&	currentCommand.getTarget() == target)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(target);
}

void CommandUtil::attackMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition)
{
	if (!attacker || !targetPosition.isValid())
	{
		return;
	}

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Move &&	currentCommand.getTargetPosition() == targetPosition)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(targetPosition);
}

void CommandUtil::move(BWAPI::Unit attacker, const BWAPI::Position & targetPosition)
{
	if (!attacker || !targetPosition.isValid())
	{
		return;
	}

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to move to this position, ignore this command
	if ((currentCommand.getType() == BWAPI::UnitCommandTypes::Move) && (currentCommand.getTargetPosition() == targetPosition) && attacker->isMoving())
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->move(targetPosition);
}

void CommandUtil::rightClick(BWAPI::Unit unit, BWAPI::Unit target)
{
	if (!unit || !target)
	{
		return;
	}

	// if we have issued a command to this unit already this frame, ignore this one
	if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || unit->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(unit->getLastCommand());

	// if we've already told this unit to move to this position, ignore this command
	if ((currentCommand.getType() == BWAPI::UnitCommandTypes::Right_Click_Unit) && (currentCommand.getTargetPosition() == target->getPosition()))
	{
		return;
	}

	// if nothing prevents it, attack the target
	unit->rightClick(target);
}

void CommandUtil::repair(BWAPI::Unit unit, BWAPI::Unit target)
{
	if (!unit || !target)
	{
		return;
	}

	// if we have issued a command to this unit already this frame, ignore this one
	if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || unit->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(unit->getLastCommand());

	// if we've already told this unit to move to this position, ignore this command
	if ((currentCommand.getType() == BWAPI::UnitCommandTypes::Repair) && (currentCommand.getTarget() == target))
	{
		return;
	}

	// if nothing prevents it, attack the target
	unit->repair(target);
}

bool UnitUtil::IsCombatUnit(BWAPI::Unit unit)
{
	if (!unit)
	{
		return false;
	}

	// no workers or buildings allowed
	if (unit && unit->getType().isWorker() || unit->getType().isBuilding())
	{
		return false;
	}

	// check for various types of combat units
	if (unit->getType().canAttack() ||
		unit->getType() == BWAPI::UnitTypes::Terran_Medic ||
		unit->getType() == BWAPI::UnitTypes::Protoss_High_Templar ||
		unit->getType() == BWAPI::UnitTypes::Protoss_Observer ||
		unit->isFlying() && unit->getType().spaceProvided() > 0)
	{
		return true;
	}

	return false;
}

bool UnitUtil::IsValidUnit(BWAPI::Unit unit)
{
	if (!unit)
	{
		return false;
	}

	if (unit->isCompleted()
		&& unit->getHitPoints() > 0
		&& unit->exists()
		&& unit->getType() != BWAPI::UnitTypes::Unknown
		&& unit->getPosition().x != BWAPI::Positions::Unknown.x
		&& unit->getPosition().y != BWAPI::Positions::Unknown.y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

double UnitUtil::GetDistanceBetweenTwoRectangles(Rect & rect1, Rect & rect2)
{
	Rect & mostLeft = rect1.x < rect2.x ? rect1 : rect2;
	Rect & mostRight = rect2.x < rect1.x ? rect1 : rect2;
	Rect & upper = rect1.y < rect2.y ? rect1 : rect2;
	Rect & lower = rect2.y < rect1.y ? rect1 : rect2;

	int diffX = std::max(0, mostLeft.x == mostRight.x ? 0 : mostRight.x - (mostLeft.x + mostLeft.width));
	int diffY = std::max(0, upper.y == lower.y ? 0 : lower.y - (upper.y + upper.height));

	return std::sqrtf(static_cast<float>(diffX*diffX + diffY*diffY));
}

bool UnitUtil::CanAttack(BWAPI::Unit attacker, BWAPI::Unit target)
{
	return GetWeapon(attacker, target) != BWAPI::UnitTypes::None;
}

bool UnitUtil::CanAttackAir(BWAPI::Unit unit)
{
	return unit->getType().airWeapon() != BWAPI::WeaponTypes::None;
}

bool UnitUtil::CanAttackGround(BWAPI::Unit unit)
{
	return unit->getType().groundWeapon() != BWAPI::WeaponTypes::None;
}

double UnitUtil::CalculateLTD(BWAPI::Unit attacker, BWAPI::Unit target)
{
	BWAPI::WeaponType weapon = GetWeapon(attacker, target);

	if (weapon == BWAPI::WeaponTypes::None)
	{
		return 0;
	}

	return static_cast<double>(weapon.damageAmount()) / weapon.damageCooldown();
}

BWAPI::WeaponType UnitUtil::GetWeapon(BWAPI::Unit attacker, BWAPI::Unit target)
{
	return target->isFlying() ? attacker->getType().airWeapon() : attacker->getType().groundWeapon();
}

BWAPI::WeaponType UnitUtil::GetWeapon(BWAPI::UnitType attacker, BWAPI::UnitType target)
{
	return target.isFlyer() ? attacker.airWeapon() : attacker.groundWeapon();
}

int UnitUtil::GetAttackRange(BWAPI::Unit attacker, BWAPI::Unit target)
{
	BWAPI::WeaponType weapon = GetWeapon(attacker, target);

	if (weapon == BWAPI::WeaponTypes::None)
	{
		return 0;
	}

	int range = weapon.maxRange();

	if ((attacker->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
		&& (attacker->getPlayer() == BWAPI::Broodwar->self())
		&& BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Singularity_Charge))
	{
		range = 6 * 32;
	}

	return range;
}

int UnitUtil::GetAttackRange(BWAPI::UnitType attacker, BWAPI::UnitType target)
{
	BWAPI::WeaponType weapon = GetWeapon(attacker, target);

	if (weapon == BWAPI::WeaponTypes::None)
	{
		return 0;
	}

	return weapon.maxRange();
}

size_t UnitUtil::GetAllUnitCount(BWAPI::UnitType type)
{
	size_t count = 0;
	for (const auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		// trivial case: unit which exists matches the type
		if (unit->getType() == type)
		{
			count++;
		}

		// case where a zerg egg contains the unit type
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg && unit->getBuildType() == type)
		{
			count += type.isTwoUnitsInOneEgg() ? 2 : 1;
		}

		// case where a building has started constructing a unit but it doesn't yet have a unit associated with it
		if (unit->getRemainingTrainTime() > 0)
		{
			BWAPI::UnitType trainType = unit->getLastCommand().getUnitType();

			if (trainType == type && unit->getRemainingTrainTime() == trainType.buildTime())
			{
				count++;
			}
		}
	}

	return count;
}

// 전체 순차탐색을 하기 때문에 느리다
BWAPI::Unit UnitUtil::GetClosestUnitTypeToTarget(BWAPI::UnitType type, BWAPI::Position target)
{
	BWAPI::Unit closestUnit = nullptr;
	double closestDist = 100000000;

	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() == type)
		{
			double dist = unit->getDistance(target);
			if (!closestUnit || dist < closestDist)
			{
				closestUnit = unit;
				closestDist = dist;
			}
		}
	}

	return closestUnit;
}


