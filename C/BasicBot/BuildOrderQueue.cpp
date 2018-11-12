#include "BuildOrderQueue.h"

using namespace MyBot;

BuildOrderQueue::BuildOrderQueue() 
	: highestPriority(0), 
	  lowestPriority(0),
	  defaultPrioritySpacing(10),
	  numSkippedItems(0)
{
	
}

void BuildOrderQueue::clearAll() 
{
	// clear the queue
	queue.clear();

	// reset the priorities
	highestPriority = 0;
	lowestPriority = 0;
}

BuildOrderItem BuildOrderQueue::getHighestPriorityItem() 
{
	// reset the number of skipped items to zero
	numSkippedItems = 0;

	// the queue will be sorted with the highest priority at the back
	return queue.back();
}

BuildOrderItem BuildOrderQueue::getNextItem() 
{
	assert(queue.size() - 1 - numSkippedItems >= 0);

	// the queue will be sorted with the highest priority at the back
	return queue[queue.size() - 1 - numSkippedItems];
}

// BuildOrderQueue에 해당 type 의 Item 이 존재하는지 카운트한다. queryTilePosition 을 입력한 경우, 건물에 대해서 추가 탐색한다
int BuildOrderQueue::getItemCount(MetaType queryType, BWAPI::TilePosition queryTilePosition)
{
	// queryTilePosition 을 입력한 경우, 거리의 maxRange. 타일단위
	int maxRange = 16;
	
	int itemCount = 0;

	size_t reps = queue.size();
	// for each unit in the queue
	for (size_t i(0); i<reps; i++) {
				
		const MetaType & item = queue[queue.size() - 1 - i].metaType;
		BWAPI::TilePosition itemPosition = queue[queue.size() - 1 - i].seedLocation;
		const BWAPI::Point<int, 32> seedPositionPoint(queryTilePosition.x, queryTilePosition.y);

		if (queryType.isUnit() && item.isUnit()) {
			//if (item.getUnitType().getID() == queryType.getUnitType().getID()) {
			if (item.getUnitType() == queryType.getUnitType()) 
			{
				if (queryType.getUnitType().isBuilding() && queryTilePosition != BWAPI::TilePositions::None)
				{
					if (itemPosition.getDistance(seedPositionPoint) <= maxRange){
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
int BuildOrderQueue::getItemCount(BWAPI::UnitType         _unitType, BWAPI::TilePosition queryTilePosition)
{
	return getItemCount(MetaType(_unitType), queryTilePosition);
}

int BuildOrderQueue::getItemCount(BWAPI::TechType         _techType)
{
	return getItemCount(MetaType(_techType));
}

int BuildOrderQueue::getItemCount(BWAPI::UpgradeType      _upgradeType)
{
	return getItemCount(MetaType(_upgradeType));
}



void BuildOrderQueue::skipCurrentItem()
{
	// make sure we can skip
	if (canSkipCurrentItem()) {
		// skip it
		numSkippedItems++;
	}
}

bool BuildOrderQueue::canSkipCurrentItem() 
{
	// does the queue have more elements
	bool bigEnough = queue.size() > (size_t)(1 + numSkippedItems);

	if (!bigEnough) 
	{
		return false;
	}

	// is the current highest priority item not blocking a skip
	bool highestNotBlocking = !queue[queue.size() - 1 - numSkippedItems].blocking;

	// this tells us if we can skip
	return highestNotBlocking;
}

void BuildOrderQueue::queueItem(BuildOrderItem b) 
{
	// if the queue is empty, set the highest and lowest priorities
	if (queue.empty()) 
	{
		highestPriority = b.priority;
		lowestPriority = b.priority;
	}

	// push the item into the queue
	if (b.priority <= lowestPriority) 
	{
		queue.push_front(b);
	}
	else
	{
		queue.push_back(b);
	}

	// if the item is somewhere in the middle, we have to sort again
	if ((queue.size() > 1) && (b.priority < highestPriority) && (b.priority > lowestPriority)) 
	{
		// sort the list in ascending order, putting highest priority at the top
		std::sort(queue.begin(), queue.end());
	}

	// update the highest or lowest if it is beaten
	highestPriority = (b.priority > highestPriority) ? b.priority : highestPriority;
	lowestPriority  = (b.priority < lowestPriority)  ? b.priority : lowestPriority;
}

void BuildOrderQueue::queueAsHighestPriority(MetaType                _metaType, bool blocking, int _producerID)
{
	// the new priority will be higher
	int newPriority = highestPriority + defaultPrioritySpacing;

	// queue the item
	queueItem(BuildOrderItem(_metaType, newPriority, blocking, _producerID));
}


// buildQueue 에 넣는다. 기본적으로 blocking 모드 (다른 것을 생산하지 않고, 이것을 생산 가능하게 될 때까지 기다리는 모드) 
// SeedPositionStrategy 를 갖고 결정
void BuildOrderQueue::queueAsHighestPriority(MetaType                _metaType, BuildOrderItem::SeedPositionStrategy _seedPositionStrategy, bool _blocking){
	int newPriority = highestPriority + defaultPrioritySpacing;
	queueItem(BuildOrderItem(_metaType, _seedPositionStrategy, newPriority, _blocking));
}
// SeedPositionStrategy 를 갖고 결정
void BuildOrderQueue::queueAsHighestPriority(BWAPI::UnitType         _unitType, BuildOrderItem::SeedPositionStrategy _seedPositionStrategy, bool _blocking){
	int newPriority = highestPriority + defaultPrioritySpacing;
	queueItem(BuildOrderItem(MetaType(_unitType), _seedPositionStrategy, newPriority, _blocking));
}
// SeedPosition 을 갖고 결정
void BuildOrderQueue::queueAsHighestPriority(MetaType                _metaType, BWAPI::TilePosition _seedPosition, bool _blocking)
{
	int newPriority = highestPriority + defaultPrioritySpacing;
	queueItem(BuildOrderItem(_metaType, _seedPosition, newPriority, _blocking));
}
// SeedPosition 을 갖고 결정
void BuildOrderQueue::queueAsHighestPriority(BWAPI::UnitType         _unitType, BWAPI::TilePosition _seedPosition, bool _blocking)
{
	int newPriority = highestPriority + defaultPrioritySpacing;
	queueItem(BuildOrderItem(MetaType(_unitType), _seedPosition, newPriority, _blocking));
}

void BuildOrderQueue::queueAsHighestPriority(BWAPI::TechType         _techType, bool blocking, int _producerID){
	int newPriority = highestPriority + defaultPrioritySpacing;
	queueItem(BuildOrderItem(MetaType(_techType), newPriority, blocking, _producerID));
}
void BuildOrderQueue::queueAsHighestPriority(BWAPI::UpgradeType      _upgradeType, bool blocking, int _producerID){
	int newPriority = highestPriority + defaultPrioritySpacing;
	queueItem(BuildOrderItem(MetaType(_upgradeType), newPriority, blocking, _producerID));
}

void BuildOrderQueue::queueAsLowestPriority(MetaType                _metaType, bool blocking, int _producerID)
{
	// the new priority will be higher
	int newPriority = lowestPriority - defaultPrioritySpacing;
	if (newPriority < 0) {
		newPriority = 0;
	}

	// queue the item
	queueItem(BuildOrderItem(_metaType, newPriority, blocking, _producerID));
}

// buildQueue 에 넣는다. 기본적으로 blocking 모드 (다른 것을 생산하지 않고, 이것을 생산 가능하게 될 때까지 기다리는 모드) 
// SeedPositionStrategy 를 갖고 결정
void BuildOrderQueue::queueAsLowestPriority(MetaType                _metaType, BuildOrderItem::SeedPositionStrategy _seedPositionStrategy, bool _blocking){
	queueItem(BuildOrderItem(_metaType, _seedPositionStrategy, 0, _blocking));
}
// SeedPositionStrategy 를 갖고 결정
void BuildOrderQueue::queueAsLowestPriority(BWAPI::UnitType         _unitType, BuildOrderItem::SeedPositionStrategy _seedPositionStrategy, bool _blocking){
	queueItem(BuildOrderItem(MetaType(_unitType), _seedPositionStrategy, 0, _blocking));
}
// SeedPosition 을 갖고 결정
void BuildOrderQueue::queueAsLowestPriority(MetaType                _metaType, BWAPI::TilePosition _seedPosition, bool _blocking)
{
	queueItem(BuildOrderItem(_metaType, _seedPosition, 0, _blocking));
}

// SeedPosition 을 갖고 결정
void BuildOrderQueue::queueAsLowestPriority(BWAPI::UnitType         _unitType, BWAPI::TilePosition _seedPosition, bool _blocking)
{
	queueItem(BuildOrderItem(MetaType(_unitType), _seedPosition, 0, _blocking));
}

void BuildOrderQueue::queueAsLowestPriority(BWAPI::TechType         _techType, bool blocking, int _producerID){
	queueItem(BuildOrderItem(MetaType(_techType), 0, blocking, _producerID));
}
void BuildOrderQueue::queueAsLowestPriority(BWAPI::UpgradeType      _upgradeType, bool blocking, int _producerID){
	queueItem(BuildOrderItem(MetaType(_upgradeType), 0, blocking, _producerID));
}

void BuildOrderQueue::removeHighestPriorityItem() 
{
	// remove the back element of the vector
	queue.pop_back();

	// if the list is not empty, set the highest accordingly
	highestPriority = queue.empty() ? 0 : queue.back().priority;
	lowestPriority  = queue.empty() ? 0 : lowestPriority;
}

void BuildOrderQueue::removeCurrentItem() 
{
	// remove the back element of the vector
	queue.erase(queue.begin() + queue.size() - 1 - numSkippedItems);

	//assert((int)(queue.size()) < size);

	// if the list is not empty, set the highest accordingly
	highestPriority = queue.empty() ? 0 : queue.back().priority;
	lowestPriority  = queue.empty() ? 0 : lowestPriority;
}

size_t BuildOrderQueue::size() 
{
	return queue.size();
}

bool BuildOrderQueue::isEmpty()
{
	return (queue.size() == 0);
}

BuildOrderItem BuildOrderQueue::operator [] (int i)
{
	return queue[i];
}

std::deque< BuildOrderItem > * BuildOrderQueue::getQueue()
{
	return &queue;
}
