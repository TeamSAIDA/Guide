import java.util.Set;

import bwapi.Color;
import bwapi.Position;
import bwapi.TilePosition;
import bwapi.Unit;
import bwapi.UnitType;

/// 건물 건설 Construction Task 자료구조
public class ConstructionTask {

	/// 건물 건설 Construction Task 의 진행 상태
	public enum ConstructionStatus { 
		Unassigned,				///< Construction 일꾼이 미지정 되어있는 상태
		Assigned,				///< Construction 일꾼이 지정 되었지만, Construction 일꾼이 건설을 착수하지는 않은 상태
		UnderConstruction		///< Construction 일꾼이 지정 되어 건설 작업을 하고있는 상태
	};
	
	/// 건물의 타입
	private UnitType type;
	
	/// 건물을 지으려고 계획한 위치<br>
	/// 일꾼이 건물을 지으러 가는 도중 해당 위치에 장애물이 있게 되는등 문제가 생기면 이 위치를 중심으로 다시 건물 지을 위치를 탐색해서 정합니다
	private TilePosition desiredPosition;
	
	/// 건물이 실제로 건설되는 위치
	private TilePosition finalPosition;
	
	/// 건물 건설 Construction Task 의 진행 상태
	private int status;
	
	/// 해당 건물의 건설 Construction Task 를 받은 일꾼 유닛
	private Unit constructionWorker;
	
	/// 해당 건물의 건설 Construction 을 실행하는 유닛<br>
	/// buildingUnit 값은 처음에 nullptr 로 세팅되고, construction 이 시작되어 isBeingConstructed, underConstrunction 상태가 되어야 비로소 값이 채워진다
	private Unit buildingUnit;

	/// 해당 건물의 건설 Construction Task 를 받은 일꾼 유닛에게 build 명령을 지시하였는지 여부.<br>
	/// 한번도 안가본 타일에는 build 명령을 내릴 수 없으므로, 일단 buildCommandGiven = false 인 상태로 일꾼을 해당 타일 위치로 이동시킨 후,<br> 
	/// 일꾼이 해당 타일 위치 근처로 오면 buildCommand 지시를 합니다
	private boolean buildCommandGiven;
	
	/// 해당 건물의 건설 Construction Task 를 받은 일꾼 유닛에게 build 명령을 지시한 시각
	private int lastBuildCommandGivenFrame;
	
	/// 해당 건물의 건설 Construction Task 를 최근에 받았던 일꾼 유닛의 ID<br>
	/// 일꾼 유닛이 Construction Task 를 받았지만 실제 수행은 못하는 상태일 경우, 새롭게 일꾼 유닛을 선정해서 Construction Task 를 부여하는데,<br> 
	/// 매번 똑같은 일꾼 유닛이 Construction Task 를 받지 않게 하기 위해서 관리
	private int lastConstructionWorkerID;
	
	/// Construction Task 가 건설 작업 시작했는가 여부
	private boolean underConstruction;

	public ConstructionTask()
	{
		desiredPosition = TilePosition.None;
		finalPosition = TilePosition.None;
		type = UnitType.Unknown;
		buildingUnit = null;
		constructionWorker = null;
		lastBuildCommandGivenFrame = 0;
		lastConstructionWorkerID = 0;
		status = ConstructionStatus.Unassigned.ordinal();
		buildCommandGiven = false;
		underConstruction = false; 
	} 

	public ConstructionTask(UnitType t, TilePosition desiredPosition)
	{
		this.desiredPosition = desiredPosition;
		finalPosition = TilePosition.None;
		type = t;
		buildingUnit = null;
		constructionWorker = null;
		lastBuildCommandGivenFrame = 0;
		lastConstructionWorkerID = 0;
		status = ConstructionStatus.Unassigned.ordinal();
		buildCommandGiven = false;
		underConstruction = false; 
	}

	public UnitType getType() {
		return type;
	}

	public Unit getConstructionWorker() {
		return constructionWorker;
	}

	public void setStatus(int status) {
		this.status = status;
	}

	public int getStatus() {
		return status;
	}

	public TilePosition getDesiredPosition() {
		return desiredPosition;
	}

	public TilePosition getFinalPosition() {
		return finalPosition;
	}

	public Unit getBuildingUnit() {
		return buildingUnit;
	}

	public int getLastConstructionWorkerID() {
		return lastConstructionWorkerID;
	}

	public void setFinalPosition(TilePosition finalPosition) {
		this.finalPosition = finalPosition;
	}

	public void setConstructionWorker(Unit constructionWorker) {
		this.constructionWorker = constructionWorker;
	}

	public void setLastConstructionWorkerID(int lastConstructionWorkerID) {
		this.lastConstructionWorkerID = lastConstructionWorkerID;
	}

	public void setBuildCommandGiven(boolean buildCommandGiven) {
		this.buildCommandGiven = buildCommandGiven;
	}

	public int getLastBuildCommandGivenFrame() {
		return lastBuildCommandGivenFrame;
	}

	public void setLastBuildCommandGivenFrame(int lastBuildCommandGivenFrame) {
		this.lastBuildCommandGivenFrame = lastBuildCommandGivenFrame;
	}

	public boolean isUnderConstruction() {
		return underConstruction;
	}

	public void setUnderConstruction(boolean underConstruction) {
		this.underConstruction = underConstruction;
	}

	public boolean isBuildCommandGiven() {
		return buildCommandGiven;
	}

	public void setType(UnitType type) {
		this.type = type;
	}

	public void setDesiredPosition(TilePosition desiredPosition) {
		this.desiredPosition = desiredPosition;
	}

	public void setBuildingUnit(Unit buildingUnit) {
		this.buildingUnit = buildingUnit;
	}
		
	/// equals override  
	@Override
	public boolean equals(Object obj) {
        if (this == obj) return true;
        if (!(obj instanceof ConstructionTask)) return false;
	
    	/// buildings are equal if their worker unit or building unit are equal		
		ConstructionTask that = (ConstructionTask)obj;
		if (this.type != null) {
			if (this.type.equals(that.type)) {
				
				if (this.desiredPosition != null) {
					if (this.desiredPosition.equals(that.desiredPosition)) {

						if (this.buildingUnit != null) {
							if (this.buildingUnit.equals(that.buildingUnit)) {
								return true;
							}
						}
						else {
							if (that.buildingUnit == null) {
								return true;
							}
						}
						
						if (this.constructionWorker != null) {
							if (this.constructionWorker.equals(that.constructionWorker)) {
								return true;
							}
						}
						else {
							if (that.constructionWorker == null) {
								return true;
							}
						}
						
					}
				}
				
			}
		}
		
		return false;
	}
}