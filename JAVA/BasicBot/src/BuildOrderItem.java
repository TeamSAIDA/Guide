import java.util.Set;

import bwapi.Color;
import bwapi.TilePosition;
import bwapi.UnitType;

/// 빌드 명령
public class BuildOrderItem {

	public MetaType metaType;			///< the thing we want to 'build'
	public int priority;				///< the priority at which to place it in the queue
	public boolean blocking;			///< whether or not we block further items
	public TilePosition seedLocation; 	///< 건설 위치
	public int producerID;				///< producer unitID (건물ID, 유닛ID)
	
	/// 건설위치 초안 결정 정책
	/// 향후 적진 길목, 언덕 위 등 추가
	public enum SeedPositionStrategy { 
		MainBaseLocation,			///< 아군 베이스
		MainBaseBackYard,			///< 아군 베이스 뒷편
		FirstChokePoint,			///< 아군 첫번째 길목
		FirstExpansionLocation,		///< 아군 첫번째 앞마당
		SecondChokePoint,			///< 아군 두번째 길목
		SeedPositionSpecified		///< 별도 지정 위치
	};
	
	public SeedPositionStrategy seedLocationStrategy;	///< 건설위치 초안 결정 정책
	
	/// 건설 위치는 SeedPositionStrategy::MainBaseLocation 을 통해 찾는다
	/// @param metaType : 빌드 대상 타입
	/// @param priority : 0 = 가장 낮은 우선순위. 숫자가 클수록 더 높은 우선순위. 큐에 있는 아이템들 중에서 가장 높은 우선순위의 아이템 (우선순위가 높으면 먼저 큐에 넣은 것)이 먼저 생산 진행됨. 
	/// @param blocking : true = 지금 이것을 생산할 수 없으면, 이것 생산 가능해질때까지 기다림.  false = 지금 이것을 생산을 할 수 없으면 다음것 먼저 생산 실행.
	/// @param producerID : producerID 를 지정하면 해당 unit 이 빌드를 실행하게 함
	public BuildOrderItem(MetaType metaType, int priority, boolean blocking, int producerID)
	{
		this.metaType = metaType;
		this.priority = priority;
		this.blocking = blocking;
		this.producerID = producerID;
		this.seedLocation = TilePosition.None;
		this.seedLocationStrategy = SeedPositionStrategy.MainBaseLocation;
	}
	
	public BuildOrderItem(MetaType metaType, int priority, boolean blocking)
	{
		this(metaType, priority, blocking, -1);
	}

	public BuildOrderItem(MetaType metaType, int priority){
		this(metaType, priority, true, -1);
	}

	public BuildOrderItem(MetaType metaType){
		this(metaType, 0, true, -1);
	}

	/// 건설 위치를 seedLocation 주위에서 찾는다
	public BuildOrderItem(MetaType metaType, TilePosition seedLocation, int priority, boolean blocking, int producerID)
	{
		this.metaType = metaType;
		this.priority = priority;
		this.blocking = blocking;
		this.producerID = producerID;
		this.seedLocation = seedLocation;
		this.seedLocationStrategy = SeedPositionStrategy.SeedPositionSpecified;
	}

	public BuildOrderItem(MetaType metaType, TilePosition seedLocation){
		this(metaType, seedLocation, 0, true, -1);
	}

	public BuildOrderItem(MetaType metaType, TilePosition seedLocation, int priority){
		this(metaType, seedLocation, priority, true, -1);
	}

	/// 건설 위치를 seedPositionStrategy 를 이용해서 찾는다. 못찾을 경우, 다른 SeedPositionStrategy 로 계속 찾는다
	public BuildOrderItem(MetaType metaType, SeedPositionStrategy seedPositionStrategy, int priority, boolean blocking, int producerID)
	{
		this.metaType = metaType;
		this.priority = priority;
		this.blocking = blocking;
		this.producerID = producerID;
		this.seedLocation = TilePosition.None;
		this.seedLocationStrategy = seedPositionStrategy;
	}

	public BuildOrderItem(MetaType metaType, SeedPositionStrategy seedPositionStrategy) {
		this(metaType, seedPositionStrategy, 0, true, -1);
	}

	public BuildOrderItem(MetaType metaType, SeedPositionStrategy seedPositionStrategy, int priority) {
		this(metaType, seedPositionStrategy, priority, true, -1);
	}
	
	/// equals override  
	@Override
	public boolean equals(Object obj) {
        if (this == obj) return true;
        if (!(obj instanceof BuildOrderItem)) return false;
	
        BuildOrderItem that = (BuildOrderItem)obj;
		if (this.metaType != null && that.metaType != null) {
			if (this.metaType.equals(that)) 
			{
				if (this.priority == that.priority 
						&& this.blocking == that.blocking
						&& this.producerID == that.producerID) 
				{					
					if (this.seedLocation != null) {
						if (this.seedLocation.equals(that.seedLocation)){
							return true;
						}
					}
					else {
						if (that.seedLocation == null){
							return true;
						}
					}
				}
			}
		}
		
		return false;
	}

}