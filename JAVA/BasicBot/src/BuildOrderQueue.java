import java.awt.Point;
import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.Collections;
import java.util.Deque;

import bwapi.TechType;
import bwapi.TilePosition;
import bwapi.UnitType;
import bwapi.UpgradeType;

/// 빌드 오더 목록 자료구조 class
public class BuildOrderQueue {
	
	private enum SeedPositionStrategy { MainBaseLocation, MainBaseBackYard, FirstChokePoint, FirstExpansionLocation, SecondChokePoint, SeedPositionSpecified };
	private SeedPositionStrategy seedLocationStrategy;

	private int highestPriority;
	private int lowestPriority;
	private int defaultPrioritySpacing;

	/// iteration 을 하기 위한 참고값<br>
	/// highest priority 인 BuildOrderItem 으로부터 몇개나 skip 했는가. 
	private int numSkippedItems;
	
	/// BuildOrderItem 들을 Double Ended Queue 자료구조로 관리합니다<br>
	/// lowest priority 인 BuildOrderItem은 front 에, highest priority 인 BuildOrderItem 은 back 에 위치하게 합니다
	private Deque<BuildOrderItem> queue = new ArrayDeque<BuildOrderItem>();
	
	public BuildOrderQueue()
	{
		highestPriority = 0; 
		lowestPriority = 0;
		defaultPrioritySpacing = 10;
		numSkippedItems = 0;
	}

	/// clears the entire build order queue
	public void clearAll() 
	{
		// clear the queue
		queue.clear();

		// reset the priorities
		highestPriority = 0;
		lowestPriority = 0;
	}

	/// returns the highest priority item
	public BuildOrderItem getHighestPriorityItem() 
	{
		// reset the number of skipped items to zero
		numSkippedItems = 0;

		// the queue will be sorted with the highest priority at the back
		// C 에서는 highest 가 back 에 있지만, JAVA 에서는 highest 가 fist 에 있다 
		return queue.getFirst();   //  queue.back(); C++
	}

	/// returns the highest priority item
	public BuildOrderItem getNextItem() 
	{
		// TODO : assert 제거
		//assert(queue.size() - 1 - numSkippedItems >= 0);

		// the queue will be sorted with the highest priority at the back
		Object[] tempArr = queue.toArray();
		
		//return (BuildOrderItem)tempArr[queue.size() - 1 - numSkippedItems];
		return (BuildOrderItem)tempArr[numSkippedItems];
	}

	/// BuildOrderQueue에 해당 type 의 Item 이 몇 개 존재하는지 리턴한다. queryTilePosition 을 입력한 경우, 건물에 대해서 추가 탐색한다
	public int getItemCount(MetaType queryType, TilePosition queryTilePosition)
	{
		// queryTilePosition 을 입력한 경우, 거리의 maxRange. 타일단위
		int maxRange = 16;
		
		int itemCount = 0;

		int reps = queue.size();
		
		Object[] tempArr = queue.toArray();
		
		// for each unit in the queue
		for (int i = 0; i<reps; i++) {
					
			final MetaType item = ((BuildOrderItem)tempArr[queue.size() - 1 - i]).metaType;
			TilePosition itemPosition = ((BuildOrderItem)tempArr[queue.size() - 1 - i]).seedLocation;
			Point seedPositionPoint = null;
			if(queryTilePosition != null)
			{
				seedPositionPoint = new Point(queryTilePosition.getX(), queryTilePosition.getY());
			}
			else
			{
				queryTilePosition = TilePosition.None;
			}

			if (queryType.isUnit() && item.isUnit()) {
				//if (item.getUnitType().getID() == queryType.getUnitType().getID()) {
				if (item.getUnitType() == queryType.getUnitType()) 
				{
					if (queryType.getUnitType().isBuilding() && queryTilePosition != TilePosition.None)
					{
						if (itemPosition.getDistance(new TilePosition((int)seedPositionPoint.getX(), (int)seedPositionPoint.getY())) <= maxRange){
							itemCount++;
						}
					}
					else 
					{
						itemCount++;
					}
				}
			}
			else if (queryType.isTech() && item.isTech()) {
				//if (item.getTechType().getID() == queryType.getTechType().getID()) {
				if (item.getTechType() == queryType.getTechType()) {
					itemCount++;
				}
			}
			else if (queryType.isUpgrade() && item.isUpgrade()) {
				//if (type.getUpgradeType().getID() == queryType.getUpgradeType().getID()) {
				if (item.getUpgradeType() == queryType.getUpgradeType()) {
					itemCount++;
				}
			}
		}
		return itemCount;
	}

	/// BuildOrderQueue에 해당 type 의 Item 이 몇 개 존재하는지 리턴한다. queryTilePosition 을 입력한 경우, 건물에 대해서 추가 탐색한다
	public int getItemCount(MetaType queryType)
	{
		return getItemCount(queryType, null);
	}
	
	/// BuildOrderQueue에 해당 type 의 Item 이 몇 개 존재하는지 리턴한다. queryTilePosition 을 입력한 경우, 건물에 대해서 추가 탐색한다
	public int getItemCount(UnitType unitType, TilePosition queryTilePosition)
	{
		return getItemCount(new MetaType(unitType), queryTilePosition);
	}

	public int getItemCount(UnitType unitType)
	{
		return getItemCount(new MetaType(unitType), null);
	}

	public int getItemCount(TechType techType)
	{
		return getItemCount(new MetaType(techType), null);
	}

	public int getItemCount(UpgradeType upgradeType)
	{
		return getItemCount(new MetaType(upgradeType), null);
	}

	/// increments skippedItems
	public void skipCurrentItem()
	{
		// make sure we can skip
		if (canSkipCurrentItem()) {
			// skip it
			numSkippedItems++;
		}
	}

	public boolean canSkipCurrentItem() 
	{
		// does the queue have more elements
		boolean bigEnough = queue.size() > (int)(1 + numSkippedItems);

		if (!bigEnough) 
		{
			return false;
		}

		// is the current highest priority item not blocking a skip
		Object[] tempArr = queue.toArray();
		//boolean highestNotBlocking = !((BuildOrderItem)tempArr[queue.size() - 1 - numSkippedItems]).blocking;
		boolean highestNotBlocking = !((BuildOrderItem)tempArr[numSkippedItems]).blocking;

		// this tells us if we can skip
		return highestNotBlocking;
	}

	/// queues something with a given priority
	public void queueItem(BuildOrderItem b) 
	{
		// if the queue is empty, set the highest and lowest priorities
		if (queue.isEmpty()) 
		{
			highestPriority = b.priority;
			lowestPriority = b.priority;
		}

		// push the item into the queue
		if (b.priority <= lowestPriority) 
		{
			queue.addLast(b); // C++ :  queue.push_front(b);
		}
		else
		{
			queue.addFirst(b); // C++ :  queue.push_back(b);
		}

		// if the item is somewhere in the middle, we have to sort again
		if ((queue.size() > 1) && (b.priority < highestPriority) && (b.priority > lowestPriority)) 
		{
			// sort the list in ascending order, putting highest priority at the top
			// C++ std::sort(queue.begin(), queue.end());
			Object[] tempArr = queue.toArray();
			Arrays.sort(tempArr);
			queue.clear();
			for(int i=0 ; i<tempArr.length ; i++){
				queue.add((BuildOrderItem)tempArr[i]);
			}
		}

		// update the highest or lowest if it is beaten
		highestPriority = (b.priority > highestPriority) ? b.priority : highestPriority;
		lowestPriority  = (b.priority < lowestPriority)  ? b.priority : lowestPriority;
	}

	/// 빌드오더를 가장 높은 우선순위로 buildQueue 에 추가한다. blocking (다른 것을 생산하지 않고, 이것을 생산 가능하게 될 때까지 기다리는 모드) 기본값은 true 이다
	public void queueAsHighestPriority(MetaType metaType, boolean blocking, int producerID)
	{
		// the new priority will be higher
		int newPriority = highestPriority + defaultPrioritySpacing;

		// queue the item
		queueItem(new BuildOrderItem(metaType, newPriority, blocking, producerID));
	}

	public void queueAsHighestPriority(MetaType metaType, boolean blocking)
	{
		queueAsHighestPriority(metaType, blocking, -1);
	}

	public void queueAsHighestPriority(MetaType metaType, BuildOrderItem.SeedPositionStrategy seedPositionStrategy, boolean blocking){
		int newPriority = highestPriority + defaultPrioritySpacing;
		queueItem(new BuildOrderItem(metaType, seedPositionStrategy, newPriority, blocking, -1));
	}
	
	public void queueAsHighestPriority(MetaType metaType){
		queueAsHighestPriority(metaType, BuildOrderItem.SeedPositionStrategy.MainBaseLocation, true);
	}

	public void queueAsHighestPriority(MetaType metaType, BuildOrderItem.SeedPositionStrategy seedPositionStrategy){
		queueAsHighestPriority(metaType, seedPositionStrategy, true);
	}
	
	public void queueAsHighestPriority(UnitType unitType, BuildOrderItem.SeedPositionStrategy seedPositionStrategy, boolean blocking){
		int newPriority = highestPriority + defaultPrioritySpacing;
		queueItem(new BuildOrderItem(new MetaType(unitType), seedPositionStrategy, newPriority, blocking, -1));
	}
	
	public void queueAsHighestPriority(MetaType metaType, TilePosition seedPosition, boolean blocking)
	{
		int newPriority = highestPriority + defaultPrioritySpacing;
		queueItem(new BuildOrderItem(metaType, seedPosition, newPriority, blocking, -1));
	}
	
	public void queueAsHighestPriority(UnitType unitType, TilePosition seedPosition, boolean blocking)
	{
		int newPriority = highestPriority + defaultPrioritySpacing;
		queueItem(new BuildOrderItem(new MetaType(unitType), seedPosition, newPriority, blocking, -1));
	}

	public void queueAsHighestPriority(UnitType unitType, boolean blocking, int producerID){
		int newPriority = highestPriority + defaultPrioritySpacing;
		queueItem(new BuildOrderItem(new MetaType(unitType), newPriority, blocking, producerID));
	}
	
	public void queueAsHighestPriority(UnitType unitType, boolean blocking){
		queueAsHighestPriority(unitType, blocking, -1);
	}

	public void queueAsHighestPriority(TechType techType, boolean blocking, int producerID){
		int newPriority = highestPriority + defaultPrioritySpacing;
		queueItem(new BuildOrderItem(new MetaType(techType), newPriority, blocking, producerID));
	}
	
	public void queueAsHighestPriority(TechType techType, boolean blocking){
		queueAsHighestPriority(techType, blocking, -1);
	}

	public void queueAsHighestPriority(UpgradeType upgradeType, boolean blocking, int producerID){
		int newPriority = highestPriority + defaultPrioritySpacing;
		queueItem(new BuildOrderItem(new MetaType(upgradeType), newPriority, blocking, producerID));
	}

	public void queueAsHighestPriority(UpgradeType upgradeType, boolean blocking){
		queueAsHighestPriority(upgradeType, blocking, -1);
	}

	/// 빌드오더를 가장 낮은 우선순위로 buildQueue 에 추가한다. blocking (다른 것을 생산하지 않고, 이것을 생산 가능하게 될 때까지 기다리는 모드) 기본값은 true 이다
	public void queueAsLowestPriority(MetaType metaType, boolean blocking, int producerID)
	{
		// the new priority will be higher
		int newPriority = lowestPriority - defaultPrioritySpacing;
		if (newPriority < 0) {
			newPriority = 0;
		}

		// queue the item
		queueItem(new BuildOrderItem(metaType, newPriority, blocking, producerID));
	}

	public void queueAsLowestPriority(MetaType metaType, boolean blocking)
	{
		queueAsLowestPriority(metaType, blocking, -1);
	}

	public void queueAsLowestPriority(MetaType metaType, BuildOrderItem.SeedPositionStrategy seedPositionStrategy, boolean blocking){
		queueItem(new BuildOrderItem(metaType, seedPositionStrategy, 0, blocking, -1));
	}
	
	public void queueAsLowestPriority(UnitType unitType, BuildOrderItem.SeedPositionStrategy seedPositionStrategy, boolean blocking){
		queueItem(new BuildOrderItem(new MetaType(unitType), seedPositionStrategy, 0, blocking, -1));
	}

	public void queueAsLowestPriority(UnitType unitType, BuildOrderItem.SeedPositionStrategy seedPositionStrategy){
		queueAsLowestPriority(unitType, seedPositionStrategy, true);
	}

	public void queueAsLowestPriority(UnitType unitType){
		queueAsLowestPriority(unitType, BuildOrderItem.SeedPositionStrategy.MainBaseLocation, true);
	}

	public void queueAsLowestPriority(MetaType metaType, TilePosition seedPosition, boolean blocking)
	{
		queueItem(new BuildOrderItem(metaType, seedPosition, 0, blocking, -1));
	}

	public void queueAsLowestPriority(UnitType unitType, TilePosition seedPosition, boolean blocking)
	{
		queueItem(new BuildOrderItem(new MetaType(unitType), seedPosition, 0, blocking, -1));
	}

	public void queueAsLowestPriority(UnitType unitType, boolean blocking, int producerID){
		queueItem(new BuildOrderItem(new MetaType(unitType), 0, blocking, producerID));
	}

	public void queueAsLowestPriority(UnitType unitType, boolean blocking){
		queueAsLowestPriority(unitType, blocking, -1);
	}
	
	public void queueAsLowestPriority(TechType techType, boolean blocking, int producerID){
		queueItem(new BuildOrderItem(new MetaType(techType), 0, blocking, producerID));
	}

	public void queueAsLowestPriority(TechType techType, boolean blocking){
		queueAsLowestPriority(techType, blocking, -1);
	}

	public void queueAsLowestPriority(TechType techType){
		queueAsLowestPriority(techType, true);
	}

	public void queueAsLowestPriority(UpgradeType upgradeType, boolean blocking, int producerID){
		queueItem(new BuildOrderItem(new MetaType(upgradeType), 0, blocking, producerID));
	}

	public void queueAsLowestPriority(UpgradeType upgradeType, boolean blocking){
		queueAsLowestPriority(upgradeType, blocking, -1);
	}

	public void queueAsLowestPriority(UpgradeType upgradeType){
		queueAsLowestPriority(upgradeType, true);
	}

	/// removes the highest priority item
	public void removeHighestPriorityItem() 
	{
		// remove the back element of the vector
		// queue.pop_back();
		queue.removeFirst();

		// if the list is not empty, set the highest accordingly
		// highestPriority = queue.isEmpty() ? 0 : queue.back().priority;
		highestPriority = queue.isEmpty() ? 0 : queue.getLast().priority;
		lowestPriority  = queue.isEmpty() ? 0 : lowestPriority;
	}

	/// skippedItems 다음의 item 을 제거합니다
	public void removeCurrentItem() 
	{
		// remove the back element of the vector
		// C++ : queue.erase(queue.begin() + queue.size() - 1 - numSkippedItems);

		Object[] tempArr = queue.toArray();
		BuildOrderItem currentItem = (BuildOrderItem)tempArr[numSkippedItems];
		//System.out.println("BuildOrderQueue currentItem to remove is " + currentItem.metaType.getName());
		queue.remove(currentItem);
		
		//assert((int)(queue.size()) < size);

		// if the list is not empty, set the highest accordingly
		// C++ : highestPriority = queue.isEmpty() ? 0 : queue.back().priority;
		highestPriority = queue.isEmpty() ? 0 : queue.getFirst().priority;
		lowestPriority  = queue.isEmpty() ? 0 : lowestPriority;
	}

	/// returns the size of the queue
	public int size() 
	{
		return queue.size();
	}

	public boolean isEmpty()
	{
		return (queue.size() == 0);
	}

	/// overload the bracket operator for ease of use
	public BuildOrderItem operator(int i)
	{
		Object[] tempArr = queue.toArray();
		return (BuildOrderItem)tempArr[i];
	}

	public Deque<BuildOrderItem> getQueue()
	{
		return queue;
	}
}