#pragma once

#include "Common.h"

namespace MyBot
{
	/// BuildManager 등에서 UnitType, TechType, UpgradeType 구분없이 동일한 Method 로 실행할 수 있게 하기 위해 만든 Wrapper class
	namespace MetaTypes
	{
		enum {
			Unit, 
			Tech, 
			Upgrade, 
			Default
		};
	}

	/// BuildManager 등에서 UnitType, TechType, UpgradeType 구분없이 동일한 Method 로 실행할 수 있게 하기 위해 만든 Wrapper class
	class MetaType
	{
		size_t                  _type;
		BWAPI::Race             _race;

		BWAPI::UnitType         _unitType;
		BWAPI::TechType         _techType;
		BWAPI::UpgradeType      _upgradeType;

	public:

		MetaType ();
		MetaType (const std::string & name);
		MetaType (BWAPI::UnitType t);
		MetaType (BWAPI::TechType t);
		MetaType (BWAPI::UpgradeType t);

		bool    isUnit()		const;
		bool    isTech()		const;
		bool    isUpgrade()	    const;
		bool    isCommand()	    const;
		bool    isBuilding()	const;
		bool    isRefinery()	const;
    
		const size_t & type() const;
		const BWAPI::Race & getRace() const;

		const BWAPI::UnitType & getUnitType() const;
		const BWAPI::TechType & getTechType() const;
		const BWAPI::UpgradeType & getUpgradeType() const;

		int     supplyRequired();
		int     mineralPrice()  const;
		int     gasPrice()      const;

		BWAPI::UnitType whatBuilds() const;
		std::string getName() const;
	};

	typedef std::pair<MetaType, size_t> MetaPair;
	typedef std::vector<MetaPair> MetaPairVector;

}