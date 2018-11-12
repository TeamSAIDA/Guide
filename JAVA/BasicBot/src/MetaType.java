import java.lang.reflect.Field;
import java.util.Set;

import bwapi.Color;
import bwapi.Race;
import bwapi.TechType;
import bwapi.TilePosition;
import bwapi.UnitType;
import bwapi.UpgradeType;

/// BuildManager 등에서 UnitType, TechType, UpgradeType 구분없이 동일한 Method 로 실행할 수 있게 하기 위해 만든 Wrapper class
public class MetaType {

	private int type;
	private Race race;
	
	private UnitType unitType;
	private TechType techType;
	private UpgradeType upgradeType;
	
	private enum MetaTypes {Unit, Tech, Upgrade, Default};
	
	public MetaType()
	{
		type = MetaTypes.Default.ordinal(); 
		race = Race.None;
	}

	public MetaType (UnitType t)
	{
		unitType = t;
	    type = MetaTypes.Unit.ordinal(); 
	    race = t.getRace();
	}

	public MetaType (TechType t)
	{
		techType = t;
	    type = MetaTypes.Tech.ordinal(); 
	    race = t.getRace();
	}

	public MetaType (UpgradeType t)
	{
		upgradeType = t;
	    type = MetaTypes.Upgrade.ordinal(); 
	    race = t.getRace();
	}

	public final int type()
	{
	    return type;
	}

	public final boolean isUnit() 
	{
	    return type == MetaTypes.Unit.ordinal(); 
	}

	public final boolean isTech() 
	{ 
	    return type == MetaTypes.Tech.ordinal(); 
	}

	public final boolean isUpgrade() 
	{ 
	    return type == MetaTypes.Upgrade.ordinal(); 
	}

	public final Race getRace()
	{
	    return race;
	}

	public final boolean isBuilding() 
	{ 
	    return type == MetaTypes.Unit.ordinal() && unitType.isBuilding(); 
	}

	public final boolean isRefinery() 
	{ 
	    return isBuilding() && unitType.isRefinery(); 
	}

	public final UnitType getUnitType()
	{
	    return unitType;
	}

	public final TechType getTechType()
	{
	    return techType;
	}

	public final UpgradeType getUpgradeType()
	{
	    return upgradeType;
	}

	public int supplyRequired()
	{
		if (isUnit())
		{
			return unitType.supplyRequired();
		}
		else
		{
			return 0;
		}
	}

	public final int mineralPrice()
	{
		return isUnit() ? unitType.mineralPrice() : (isTech() ? techType.mineralPrice() : upgradeType.mineralPrice());
	}

	public final int gasPrice()
	{
		return isUnit() ? unitType.gasPrice() : (isTech() ? techType.gasPrice() : upgradeType.gasPrice());
	}

	public final UnitType whatBuilds()
	{
		return isUnit() ? unitType.whatBuilds().first : (isTech() ? techType.whatResearches() : upgradeType.whatUpgrades());
	}

	public final String getName()
	{
		if (isUnit())
		{
			return unitType.toString();
			//return unitType.getName();
		}
		else if (isTech())
		{
			return techType.toString();
			//return techType.getName();
		}
		else if (isUpgrade())
		{
			return upgradeType.toString();
			//return upgradeType.getName();
		}
		else
		{
			return "LOL";	
		}
	}
	
	@Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof MetaType)) return false;

        MetaType that = (MetaType) o;

        if (this.type != that.type) return false;

        return true;
    }

}