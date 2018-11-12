#pragma once

#include "Common.h"
#include "Config.h"

namespace MyBot
{
	struct Rect
	{
		int x, y;
		int height, width;
	};

	/// 이동 (move), 공격 (attack), 수리 (repair), 우클릭 (rightClick)  등 유닛 컨트롤 명령을 내릴 때 각종 체크해야할 사항들을 체크한 후 명령 내리도록 하는 헬퍼 함수들
	namespace CommandUtil
	{
		/// attacker 가 target 을 공격하도록 명령 합니다
		void attackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
		
		/// attacker 가 targetPosition 을 향해 공격 가도록 명령 합니다
		void attackMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition);

		/// attacker 가 targetPosition 을 향해 이동 가도록 명령 합니다
		void move(BWAPI::Unit attacker, const BWAPI::Position & targetPosition);

		/// unit 이 target 에 대해 어떤 행위를 하도록 명령 합니다<br>
		/// 일꾼 유닛이 Mineral Field 에게 : Mineral 자원 채취<br>
		/// 일꾼 유닛이 Refinery 건물에게 : Gas 자원 채취<br>
		/// 전투 유닛이 다른 아군 유닛에게 : Move 명령<br>
		/// 전투 유닛이 다른 적군 유닛에게 : Attack 명령<br>
		void rightClick(BWAPI::Unit unit, BWAPI::Unit target);

		/// unit 이 target 에 대해 수리 하도록 명령 합니다 
		void repair(BWAPI::Unit unit, BWAPI::Unit target);
	};

	namespace UnitUtil
	{
		bool IsCombatUnit(BWAPI::Unit unit);
		bool IsValidUnit(BWAPI::Unit unit);
		bool CanAttackAir(BWAPI::Unit unit);
		bool CanAttackGround(BWAPI::Unit unit);
		bool IsGroundTarget(BWAPI::Unit unit);
		bool IsAirTarget(BWAPI::Unit unit);
		bool CanAttack(BWAPI::Unit attacker, BWAPI::Unit target);
		bool CanAttack(BWAPI::UnitType attacker, BWAPI::UnitType target);
		double CalculateLTD(BWAPI::Unit attacker, BWAPI::Unit target);
		int GetAttackRange(BWAPI::Unit attacker, BWAPI::Unit target);
		int GetAttackRange(BWAPI::UnitType attacker, BWAPI::UnitType target);
		int GetTransportSize(BWAPI::UnitType type);
		BWAPI::WeaponType GetWeapon(BWAPI::Unit attacker, BWAPI::Unit target);
		BWAPI::WeaponType GetWeapon(BWAPI::UnitType attacker, BWAPI::UnitType target);

		size_t GetAllUnitCount(BWAPI::UnitType type);

		BWAPI::Unit GetClosestUnitTypeToTarget(BWAPI::UnitType type, BWAPI::Position target);
		double GetDistanceBetweenTwoRectangles(Rect & rect1, Rect & rect2);
	};
}