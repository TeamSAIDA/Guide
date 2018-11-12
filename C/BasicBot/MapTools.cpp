#include "MapTools.h"

using namespace MyBot;

MapGrid & MapGrid::Instance()
{
	static MapGrid instance(BWAPI::Broodwar->mapWidth() * 32, BWAPI::Broodwar->mapHeight() * 32, Config::Tools::MAP_GRID_SIZE);
	return instance;
}

MapGrid::MapGrid() 
{
}

MapGrid::MapGrid(int mapWidth, int mapHeight, int cellSize)
	: mapWidth(mapWidth)
	, mapHeight(mapHeight)
	, cellSize(cellSize)
	, cols((mapWidth + cellSize - 1) / cellSize)
	, rows((mapHeight + cellSize - 1) / cellSize)
	, cells(rows * cols)
	, lastUpdated(0)
{
	calculateCellCenters();
}

BWAPI::Position MapGrid::getLeastExplored()
{
	int minSeen = 1000000;
	double minSeenDist = 0;
	int leastRow(0), leastCol(0);

	for (int r = 0; r<rows; ++r)
	{
		for (int c = 0; c<cols; ++c)
		{
			// get the center of this cell
			BWAPI::Position cellCenter = getCellCenter(r, c);

			// don't worry about places that aren't connected to our start locatin
			if (!BWTA::isConnected(BWAPI::TilePosition(cellCenter), BWAPI::Broodwar->self()->getStartLocation()))
			{
				continue;
			}

			BWAPI::Position home(BWAPI::Broodwar->self()->getStartLocation());
			double dist = home.getDistance(getCellByIndex(r, c).center);
			int lastVisited = getCellByIndex(r, c).timeLastVisited;
			if (lastVisited < minSeen || ((lastVisited == minSeen) && (dist > minSeenDist)))
			{
				leastRow = r;
				leastCol = c;
				minSeen = getCellByIndex(r, c).timeLastVisited;
				minSeenDist = dist;
			}
		}
	}

	return getCellCenter(leastRow, leastCol);
}

void MapGrid::calculateCellCenters()
{
	for (int r = 0; r < rows; ++r)
	{
		for (int c = 0; c < cols; ++c)
		{
			GridCell & cell = getCellByIndex(r, c);

			int centerX = (c * cellSize) + (cellSize / 2);
			int centerY = (r * cellSize) + (cellSize / 2);

			// if the x position goes past the end of the map
			if (centerX > mapWidth)
			{
				// when did the last cell start
				int lastCellStart = c * cellSize;

				// how wide did we go
				int tooWide = mapWidth - lastCellStart;

				// go half the distance between the last start and how wide we were
				centerX = lastCellStart + (tooWide / 2);
			}
			else if (centerX == mapWidth)
			{
				centerX -= 50;
			}

			if (centerY > mapHeight)
			{
				// when did the last cell start
				int lastCellStart = r * cellSize;

				// how wide did we go
				int tooHigh = mapHeight - lastCellStart;

				// go half the distance between the last start and how wide we were
				centerY = lastCellStart + (tooHigh / 2);
			}
			else if (centerY == mapHeight)
			{
				centerY -= 50;
			}

			cell.center = BWAPI::Position(centerX, centerY);
			assert(cell.center.isValid());
		}
	}
}

BWAPI::Position MapGrid::getCellCenter(int row, int col)
{
	return getCellByIndex(row, col).center;
}

// clear the vectors in the grid
void MapGrid::clearGrid() {

	for (size_t i(0); i<cells.size(); ++i)
	{
		cells[i].ourUnits.clear();
		cells[i].oppUnits.clear();
	}
}

// populate the grid with units
void MapGrid::update()
{
	// clear the grid
	clearGrid();

	//BWAPI::Broodwar->printf("MapGrid info: WH(%d, %d)  CS(%d)  RC(%d, %d)  C(%d)", mapWidth, mapHeight, cellSize, rows, cols, cells.size());

	// add our units to the appropriate cell
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		getCell(unit).ourUnits.insert(unit);
		getCell(unit).timeLastVisited = BWAPI::Broodwar->getFrameCount();
	}

	// add enemy units to the appropriate cell
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getHitPoints() > 0)
		{
			getCell(unit).oppUnits.insert(unit);
			getCell(unit).timeLastOpponentSeen = BWAPI::Broodwar->getFrameCount();
		}
	}
}

void MapGrid::getUnitsNear(BWAPI::Unitset & units, BWAPI::Position center, int radius, bool ourUnits, bool oppUnits)
{
	const int x0(std::max((center.x - radius) / cellSize, 0));
	const int x1(std::min((center.x + radius) / cellSize, cols - 1));
	const int y0(std::max((center.y - radius) / cellSize, 0));
	const int y1(std::min((center.y + radius) / cellSize, rows - 1));
	const int radiusSq(radius * radius);
	for (int y(y0); y <= y1; ++y)
	{
		for (int x(x0); x <= x1; ++x)
		{
			int row = y;
			int col = x;

			GridCell & cell(getCellByIndex(row, col));
			if (ourUnits)
			{
				for (auto & unit : cell.ourUnits)
				{
					BWAPI::Position d(unit->getPosition() - center);
					if (d.x * d.x + d.y * d.y <= radiusSq)
					{
						if (!units.contains(unit))
						{
							units.insert(unit);
						}
					}
				}
			}
			if (oppUnits)
			{
				for (auto & unit : cell.oppUnits) if (unit->getType() != BWAPI::UnitTypes::Unknown && unit->isVisible())
				{
					BWAPI::Position d(unit->getPosition() - center);
					if (d.x * d.x + d.y * d.y <= radiusSq)
					{
						if (!units.contains(unit))
						{
							units.insert(unit);
						}
					}
				}
			}
		}
	}
}


int	MapGrid::getCellSize()
{
	return cellSize;
}

int MapGrid::getMapWidth(){
	return mapWidth;
}

int MapGrid::getMapHeight(){
	return mapHeight;
}

int MapGrid::getRows()
{
	return rows;
}

int MapGrid::getCols()
{
	return cols;
}

MapTools & MapTools::Instance()
{
    static MapTools instance;
    return instance;
}

// constructor for MapTools
MapTools::MapTools()
    : _rows(BWAPI::Broodwar->mapHeight())
    , _cols(BWAPI::Broodwar->mapWidth())
{
    _map    = std::vector<bool>(_rows*_cols,false);
    _units  = std::vector<bool>(_rows*_cols,false);
    _fringe = std::vector<int>(_rows*_cols,0);

    setBWAPIMapData();
}

// return the index of the 1D array from (row,col)
inline int MapTools::getIndex(int row,int col)
{
    return row * _cols + col;
}

bool MapTools::unexplored(DistanceMap & dmap,const int index) const
{
    return (index != -1) && dmap[index] == -1 && _map[index];
}

// resets the distance and fringe vectors, call before each search
void MapTools::reset()
{
    std::fill(_fringe.begin(),_fringe.end(),0);
}

// reads in the map data from bwapi and stores it in our map format
void MapTools::setBWAPIMapData()
{
    // for each row and column
    for (int r(0); r < _rows; ++r)
    {
        for (int c(0); c < _cols; ++c)
        {
            bool clear = true;

            // check each walk tile within this TilePosition
            for (int i=0; i<4; ++i)
            {
                for (int j=0; j<4; ++j)
                {
                    if (!BWAPI::Broodwar->isWalkable(c*4 + i,r*4 + j))
                    {
                        clear = false;
                        break;
                    }

                    if (clear)
                    {
                        break;
                    }
                }
            }

            // set the map as binary clear or not
            _map[getIndex(r,c)] = clear;
        }
    }
}

void MapTools::resetFringe()
{
    std::fill(_fringe.begin(),_fringe.end(),0);
}

int MapTools::getGroundDistance(BWAPI::Position origin,BWAPI::Position destination)
{
    // if we have too many maps, reset our stored maps in case we run out of memory
    if (_allMaps.size() > 20)
    {
        _allMaps.clear();

        //BWAPI::Broodwar->printf("Cleared stored distance map cache");
    }

    // if we haven't yet computed the distance map to the destination
    if (_allMaps.find(destination) == _allMaps.end())
    {
		std::cout << "we haven't yet" << std::endl;
		
		// if we have computed the opposite direction, we can use that too
        if (_allMaps.find(origin) != _allMaps.end())
        {
			std::cout << "we have opposite" << std::endl;
			
			return _allMaps[origin][destination];
        }

		std::cout << "compute it" << std::endl;

        // add the map and compute it
        _allMaps.insert(std::pair<BWAPI::Position,DistanceMap>(destination,DistanceMap()));
        computeDistance(_allMaps[destination],destination);
    }

	std::cout << "get it" << std::endl;

    // get the distance from the map
    return _allMaps[destination][origin];
}


// computes walk distance from Position P to all other points on the map
void MapTools::computeDistance(DistanceMap & dmap,const BWAPI::Position p)
{
    search(dmap,p.y / 32,p.x / 32);
}

// does the dynamic programming search
void MapTools::search(DistanceMap & dmap,const int sR,const int sC)
{
    // reset the internal variables
    resetFringe();

    // set the starting position for this search
    dmap.setStartPosition(sR,sC);

    // set the distance of the start cell to zero
    dmap[getIndex(sR,sC)] = 0;

    // set the fringe variables accordingly
    int fringeSize(1);
    int fringeIndex(0);
    _fringe[0] = getIndex(sR,sC);
    dmap.addSorted(getTilePosition(_fringe[0]));

    // temporary variables used in search loop
    int currentIndex,nextIndex;
    int newDist;

    // the size of the map
    int size = _rows*_cols;

    // while we still have things left to expand
    while (fringeIndex < fringeSize)
    {
        // grab the current index to expand from the fringe
        currentIndex = _fringe[fringeIndex++];
        newDist = dmap[currentIndex] + 1;

        // search up
        nextIndex = (currentIndex > _cols) ? (currentIndex - _cols) : -1;
        if (unexplored(dmap,nextIndex))
        {
            // set the distance based on distance to current cell
            dmap.setDistance(nextIndex,newDist);
            dmap.setMoveTo(nextIndex,'D');
            dmap.addSorted(getTilePosition(nextIndex));

            // put it in the fringe
            _fringe[fringeSize++] = nextIndex;
        }

        // search down
        nextIndex = (currentIndex + _cols < size) ? (currentIndex + _cols) : -1;
        if (unexplored(dmap,nextIndex))
        {
            // set the distance based on distance to current cell
            dmap.setDistance(nextIndex,newDist);
            dmap.setMoveTo(nextIndex,'U');
            dmap.addSorted(getTilePosition(nextIndex));

            // put it in the fringe
            _fringe[fringeSize++] = nextIndex;
        }

        // search left
        nextIndex = (currentIndex % _cols > 0) ? (currentIndex - 1) : -1;
        if (unexplored(dmap,nextIndex))
        {
            // set the distance based on distance to current cell
            dmap.setDistance(nextIndex,newDist);
            dmap.setMoveTo(nextIndex,'R');
            dmap.addSorted(getTilePosition(nextIndex));

            // put it in the fringe
            _fringe[fringeSize++] = nextIndex;
        }

        // search right
        nextIndex = (currentIndex % _cols < _cols - 1) ? (currentIndex + 1) : -1;
        if (unexplored(dmap,nextIndex))
        {
            // set the distance based on distance to current cell
            dmap.setDistance(nextIndex,newDist);
            dmap.setMoveTo(nextIndex,'L');
            dmap.addSorted(getTilePosition(nextIndex));

            // put it in the fringe
            _fringe[fringeSize++] = nextIndex;
        }
    }
}

const std::vector<BWAPI::TilePosition> & MapTools::getClosestTilesTo(BWAPI::Position pos)
{
    // make sure the distance map is calculated with pos as a destination
    int a = getGroundDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()),pos);

    return _allMaps[pos].getSortedTiles();
}

BWAPI::TilePosition MapTools::getTilePosition(int index)
{
    return BWAPI::TilePosition(index % _cols,index / _cols);
}

void MapTools::parseMap()
{
    //BWAPI::Broodwar->printf("Parsing Map Information");
    std::ofstream mapFile;
    std::string file = "c:\\scmaps\\" + BWAPI::Broodwar->mapName() + ".txt";
    mapFile.open(file.c_str());

    mapFile << BWAPI::Broodwar->mapWidth()*4 << "\n";
    mapFile << BWAPI::Broodwar->mapHeight()*4 << "\n";

    for (int j=0; j<BWAPI::Broodwar->mapHeight()*4; j++) 
    {
        for (int i=0; i<BWAPI::Broodwar->mapWidth()*4; i++) 
        {
            if (BWAPI::Broodwar->isWalkable(i,j)) 
            {
                mapFile << "0";
            }
            else 
            {
                mapFile << "1";
            }
        }

        mapFile << "\n";
    }

    //BWAPI::Broodwar->printf(file.c_str());

    mapFile.close();
}
