#pragma once

#include "Common.h"

namespace MyBot
{
	/// 해당 Unit의 ID, UnitType, 소속 Player, HitPoint, lastPosition, completed(건물이 완성된 것인지) 등을 저장해두는 자료구조.<br>
	/// 적군 유닛의 경우 아군 시야 내에 있지 않아 invisible 상태가 되었을 때 정보를 조회할수도 없어지고 파악했던 정보도 유실되기 때문에 별도 자료구조가 필요합니다
	struct UnitInfo
	{
		int             unitID;
		int             lastHealth;
		int             lastShields;
		BWAPI::Player   player;
		BWAPI::Unit     unit;
		BWAPI::Position lastPosition;
		BWAPI::UnitType type;
		bool            completed;

		UnitInfo()
			: unitID(0)
			, lastHealth(0)
			, player(nullptr)
			, unit(nullptr)
			, lastPosition(BWAPI::Positions::None)
			, type(BWAPI::UnitTypes::None)
			, completed(false)
		{
		}

		const bool operator == (BWAPI::Unit unit) const
		{
			return unitID == unit->getID();
		}

		const bool operator == (const UnitInfo & rhs) const
		{
			return (unitID == rhs.unitID);
		}

		const bool operator < (const UnitInfo & rhs) const
		{
			return (unitID < rhs.unitID);
		}
	};

	typedef std::map<BWAPI::Unit,UnitInfo> UnitAndUnitInfoMap;

	class UnitData
	{
		/// Unit 과 UnitInfo 를 Map 형태로 저장하는 자료구조.<br>
		/// C++ 에서는 Unit 포인터를 Key 로 사용하지만,<br> 
		/// JAVA 에서는 Unit 자료구조의 equals 메쏘드 때문에 오작동하므로 Unit.getID() 값을 Key 로 사용함
		UnitAndUnitInfoMap						unitAndUnitInfoMap;

		const bool isBadUnitInfo(const UnitInfo & ui) const;

		/// UnitType별 파괴/사망한 유닛 숫자 누적값<br>
		/// C++ 에서는 UnitType 의 열거형 값을 Key 로 사용하지만, <br>
		/// JAVA 에서는 UnitType 의 열거형 값이 부재하므로 Unit.getType() 값을 Key 로 사용함
		std::vector<int>						numDeadUnits;
		/// UnitType별 건설/훈련했던 유닛 숫자 누적값<br>
		/// C++ 에서는 UnitType 의 열거형 값을 Key 로 사용하지만, <br>
		/// JAVA 에서는 UnitType 의 열거형 값이 부재하므로 Unit.getType() 값을 Key 로 사용함
		std::vector<int>						numCreatedUnits;
		/// UnitType별 존재하는 유닛 숫자 카운트. 적군 유닛의 경우 식별된 유닛 숫자 카운트<br>
		/// C++ 에서는 UnitType 의 열거형 값을 Key 로 사용하지만, <br>
		/// JAVA 에서는 UnitType 의 열거형 값이 부재하므로 Unit.getType() 값을 Key 로 사용함
		std::vector<int>						numUnits;

		/// 사망한 유닛을 생산하는데 소요되었던 Mineral 의 누적값 (얼마나 손해를 보았는가 계산하기 위함임)
		int										mineralsLost;
		/// 사망한 유닛을 생산하는데 소요되었던 Gas 의 누적값 (얼마나 손해를 보았는가 계산하기 위함임)
		int										gasLost;

	public:

		UnitData();

		/// 유닛의 상태정보를 업데이트합니다
		void	updateUnitInfo(BWAPI::Unit unit);

		/// 파괴/사망한 유닛을 자료구조에서 제거합니다
		void	removeUnit(BWAPI::Unit unit);

		/// 포인터가 null 이 되었거나, 파괴되어 Resource_Vespene_Geyser로 돌아간 Refinery, 예전에는 건물이 있었던 걸로 저장해두었는데 지금은 파괴되어 없어진 건물 (특히, 테란의 경우 불타서 소멸한 건물) 데이터를 제거합니다
		void	removeBadUnits();

		/// 사망한 유닛을 생산하는데 소요되었던 Mineral 의 누적값 (얼마나 손해를 보았는가 계산하기 위함임) 을 리턴합니다
		int		getMineralsLost()                           const;

		/// 사망한 유닛을 생산하는데 소요되었던 Gas 의 누적값 (얼마나 손해를 보았는가 계산하기 위함임)
		int		getGasLost()                                const;

		/// 해당 UnitType 의 식별된 Unit 숫자를 리턴합니다
		int		getNumUnits(BWAPI::UnitType t)              const;

		/// 해당 UnitType 의 식별된 Unit 파괴/사망 누적값을 리턴합니다
		int		getNumDeadUnits(BWAPI::UnitType t)          const;

		/// 해당 UnitType 의 식별된 Unit 건설/훈련 누적값을 리턴합니다
		int		getNumCreatedUnits(BWAPI::UnitType t)		const;

		// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
		// getUnitAndUnitInfoMap 메소드에 대해 const 제거

		std::map<BWAPI::Unit, UnitInfo> & getUnitAndUnitInfoMap();

		// BasicBot 1.1 Patch End //////////////////////////////////////////////////
	};
}