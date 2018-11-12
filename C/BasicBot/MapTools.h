#pragma once

#include "Common.h"

namespace MyBot
{
	class DistanceMap
	{
		/// mapHeight
		int rows;
		/// mapWidth
		int cols;

		/// 시작점의 X 좌표. 지도 상 각 점까지의 거리를 계산하기 위한 시작점.<br>
		/// 처음에는 -1
		int startRow;
		/// 시작점의 Y 좌표. 지도 상 각 점까지의 거리를 계산하기 위한 시작점.<br>
		/// 처음에는 -1
		int startCol;
		
		/// 맵 width * height 2차원 배열의 1차원 형태로 바꾼 것. getIndex 를 사용해서 전환.<br>
		/// 처음에는 전부 -1 (거리갈 수 없는 곳) 로 초기화
		std::vector<int> dist;
		
		/// 처음에는 전부 'X' 로 초기화
		std::vector<char> moveTo;

		/// 거리 순으로 정렬된 벡터
		std::vector<BWAPI::TilePosition> sorted;

		int getIndex(const int row, const int col) const
		{
			return row * cols + col;
		}

		int getIndex(const BWAPI::Position & p) const
		{
			return getIndex(p.y / 32, p.x / 32);
		}

	public:

		DistanceMap()
			: dist(std::vector<int>(BWAPI::Broodwar->mapWidth() * BWAPI::Broodwar->mapHeight(), -1))
			, moveTo(std::vector<char>(BWAPI::Broodwar->mapWidth() * BWAPI::Broodwar->mapHeight(), 'X'))
			, rows(BWAPI::Broodwar->mapHeight()), cols(BWAPI::Broodwar->mapWidth()), startRow(-1), startCol(-1)
		{
			//BWAPI::Broodwar->printf("New Distance Map With Dimensions (%d, %d)", rows, cols);
		}

		int & operator [] (const int index)						{ return dist[index]; }
		int & operator [] (const BWAPI::Position & pos)			{ return dist[getIndex(pos.y / 32, pos.x / 32)]; }
		void setMoveTo(const int index, const char val)			{ moveTo[index] = val; }
		void setDistance(const int index, const int val)		{ dist[index] = val; }
		void setStartPosition(const int sr, const int sc)		{ startRow = sr; startCol = sc; }

		/// reset the distance map
		void reset(const int & rows, const int & cols)
		{
			this->rows = rows;
			this->cols = cols;
			dist = std::vector<int>(rows * cols, -1);
			sorted.clear();
			moveTo = std::vector<char>(rows * cols, 'X');
			startRow = -1;
			startCol = -1;
		}

		const std::vector<BWAPI::TilePosition> & getSortedTiles() const
		{
			return sorted;
		}

		/// reset the distance map
		void reset()
		{
			std::fill(dist.begin(), dist.end(), -1);
			std::fill(moveTo.begin(), moveTo.end(), 'X');
			sorted.clear();
			startRow = -1;
			startCol = -1;
		}

		/// 시작점으로부터 p까지의 거리가 -1 이 아니면 true
		bool isConnected(const BWAPI::Position p) const
		{
			return dist[getIndex(p)] != -1;
		}

		void addSorted(const BWAPI::TilePosition & tp)
		{
			sorted.push_back(tp);
		}

		// given a position, get the position we should move to to minimize distance
		BWAPI::Position getMoveTo(const BWAPI::Position p, const int lookAhead = 1) const
		{
			// the initial row an column
			int row = p.y / 32;
			int col = p.x / 32;

			// for each lookahead
			for (int i = 0; i<lookAhead; ++i)
			{
				// get the index
				int index = getIndex(row, col);

				// adjust the row and column accordingly
				if (moveTo[index] == 'L')
				{
					col -= 1;
				}
				else if (moveTo[index] == 'R')
				{
					col += 1;
				}
				else if (moveTo[index] == 'U')
				{
					row -= 1;
				}
				else
				{
					row += 1;
				}
			}

			// return the position
			return BWAPI::Position(col * 32 + 16, row * 32 + 16);
		}
	};

	/// 지도를 바둑판처럼 Cell 들로 나누기 위해서 정의한 하나의 Cell
	class GridCell
	{
	public:		
		int             timeLastVisited;			///< 가장 마지막에 방문했던 시각이 언제인지 -> Scout 에 활용		
		int             timeLastOpponentSeen;		///< 가장 마지막에 적을 발견했던 시각이 언제인지 -> 적 의도 파악, 적 부대 파악, 전략 수립에 활용
		BWAPI::Unitset  ourUnits;
		BWAPI::Unitset  oppUnits;
		BWAPI::Position center;

		GridCell()
			: timeLastVisited(0)
			, timeLastOpponentSeen(0)
		{
		}
	};

	/// 지도를 바둑판처럼 Cell 들로 나누고, 매 frame 마다 각 Cell 의 timeLastVisited 시간정보, timeLastOpponentSeen 시간정보, ourUnits 와 oppUnits 목록을 업데이트 합니다
	class MapGrid
	{
		MapGrid();
		MapGrid(int mapWidth, int mapHeight, int cellSize);

		int							cellSize;
		int							mapWidth, mapHeight;
		int							rows, cols;
		int							lastUpdated;

		std::vector< GridCell >		cells;

		void						calculateCellCenters();

		void						clearGrid();
		BWAPI::Position				getCellCenter(int x, int y);

	public:
		/// static singleton 객체를 리턴합니다
		static MapGrid &	Instance();

		/// 각 Cell 의 timeLastVisited 시간정보, timeLastOpponentSeen 시간정보, ourUnits 와 oppUnits 목록 등을 업데이트 합니다
		void				update();

		/// 해당 position 근처에 있는 아군 혹은 적군 유닛들의 목록을 UnitSet 에 저장합니다.<br>
		/// BWAPI::Broodwar->self()->getUnitsOnTile, getUnitsInRectangle, getUnitsInRadius, getClosestUnit 함수와 유사하지만 쓰임새가 다릅니다
		void				getUnitsNear(BWAPI::Unitset & units, BWAPI::Position center, int radius, bool ourUnits, bool oppUnits);

		BWAPI::Position		getLeastExplored();

		GridCell &			getCellByIndex(int r, int c)		{ return cells[r*cols + c]; }
		GridCell &			getCell(BWAPI::Position pos)		{ return getCellByIndex(pos.y / cellSize, pos.x / cellSize); }
		GridCell &			getCell(BWAPI::Unit unit)			{ return getCell(unit->getPosition()); }

		int					getCellSize();
		int					getMapWidth();
		int					getMapHeight();
		int					getRows();
		int					getCols();
	};



	/// provides useful tools for analyzing the starcraft map.<br>
	/// calculates connectivity and distances using flood fills
	class MapTools
	{
    
		std::map<BWAPI::Position, DistanceMap>       _allMaps;		///< a cache of already computed distance maps
		std::vector<bool>           _map;							///< the map stored at TilePosition resolution, values are 0/1 for walkable or not walkable
		std::vector<bool>           _units;							///< map that stores whether a unit is on this position
		std::vector<int>            _fringe;						///< the fringe vector which is used as a sort of 'open list'
		int                         _rows;
		int                         _cols;

		MapTools();

		int                     getIndex(int row,int col);			///< return the index of the 1D array from (row,col)
		bool                    unexplored(DistanceMap & dmap,const int index) const;
		void                    reset();							///< resets the distance and fringe vectors, call before each search    
		void                    setBWAPIMapData();					///< reads in the map data from bwapi and stores it in our map format
		void                    resetFringe();
		BWAPI::TilePosition     getTilePosition(int index);		

		/// from 에서 to 까지 지상유닛이 이동할 경우의 거리 (walk distance). 못가는 곳이면 -1 <br>
		/// computeDistance 를 수행해서 _allMaps 에 저장한다.<br>
		/// 반환값 자체는 부정확하다. 지상유닛이 못가는 곳인데도 거리가 산출된다.<br>
		/// getClosestTilesTo 를 계산하는 과정에서 한번 호출된다.<br>
		/// 속도가 느리다.<br>
		int                     getGroundDistance(BWAPI::Position from, BWAPI::Position to);

		void                    computeDistance(DistanceMap & dmap, const BWAPI::Position p); ///< computes walk distance from Position P to all other points on the map

		void                    search(DistanceMap & dmap, const int sR, const int sC);

		/// 지도를 Parsing 해서 파일로 저장해둔다.<br>
		/// 사용하지 않는 API
		void                    parseMap();

		/// 사용하지 않는 API
		void                    fill(const int index, const int region);
	public:
		/// static singleton 객체를 리턴합니다
		static MapTools &       Instance();

		/// pos 에서 가까운 순서대로 정렬된 타일의 목록을 반환한다.<br>
		/// pos 에서 지상유닛이 갈 수 있는 타일만 반환한다
		const std::vector<BWAPI::TilePosition> & getClosestTilesTo(BWAPI::Position pos);
	};

}