import java.util.Vector;

import bwapi.Position;
import bwapi.TilePosition;
import bwapi.Unit;
import bwapi.UnitType;
import bwapi.Unitset;
import bwta.BWTA;

public class DistanceMap {

	// 지도를 바둑판처럼 Cell 들로 나누고, 매 frame 마다 각 Cell 의 timeLastVisited 시간정보, timeLastOpponentSeen 시간정보, ourUnits 와 oppUnits 목록을 업데이트 한다
	// 가장 마지막에 방문했던 시각이 언제인지 -> Scout 에 활용
	// 가장 마지막에 적을 발견했 시각이 언제인지 -> 적 의도 파악, 적 부대 파악, 전략 수립에 활용
	class GridCell
	{
		private int timeLastVisited;
		private int timeLastOpponentSeen;
		private Unitset ourUnits;
		private Unitset oppUnits;
		private Position center;

		public GridCell()
		{
			timeLastVisited = 0;
			timeLastOpponentSeen = 0;
		}
	};
	
	private int cellSize;
	private int mapWidth;
	private int mapHeight;
	private int cols;
	private int rows;
	private int cells;
	private int lastUpdated;
	private int startRow;
	private int startCol;

	private int[] dist;
	private char[] moveTo;
	private Vector<GridCell> gridCells;
	private Vector<TilePosition> sorted = new Vector<TilePosition>();

	private static DistanceMap instance = new DistanceMap();
	
	public static DistanceMap Instance() {
		return instance;
	}
	
	public DistanceMap()
	{
		this.dist = new int[MyBotModule.Broodwar.mapWidth() * MyBotModule.Broodwar.mapHeight()];
		for(int i = 0 ; i < this.dist.length ; i++)
		{
			this.dist[i] = -1;
		}
		this.moveTo = new char[MyBotModule.Broodwar.mapWidth() * MyBotModule.Broodwar.mapHeight()];
		for(int j = 0 ; j < this.moveTo.length ; j++)
		{
			this.moveTo[j] = 'X';
		}
		this.rows = MyBotModule.Broodwar.mapHeight();
		this.cols = MyBotModule.Broodwar.mapWidth();
		this.startRow = -1;
		this.startCol = -1;
	}
	
	public DistanceMap(int mapWidth, int mapHeight, int cellSize)
	{
		this.mapWidth = mapWidth;
		this.mapHeight = mapHeight;
		this.cellSize = cellSize;
		this.cols = (mapWidth + cellSize - 1) / cellSize;
		this.rows = (mapHeight + cellSize - 1) / cellSize;
		this.cells = rows * cols;
		this.lastUpdated = 0;
	}

	public int getDistItem(int index)
	{
		return dist[index];
	}
	
	public int getDistItem(Position p)
	{
		return dist[getIndex(p.getY() / 32, p.getX() / 32)];
	}
	
	public final int getIndex(final int row, final int col)
	{
		return row * cols + col;
	}

	public final int getIndex(final Position p)
	{
		return getIndex(p.getY() / 32, p.getX() / 32);
	}
	
	public void setMoveTo(final int index, final char val)
	{
		moveTo[index] = val;
	}
	
	public void setDistance(final int index, final int val)
	{
		dist[index] = val;
	}
	
	public void setStartPosition(final int sr, final int sc)
	{
		startRow = sr;
		startCol = sc;
	}

	public void addSorted(final TilePosition tp)
	{
		sorted.add(tp);
	}
	
	public Position getLeastExplored()
	{
		int minSeen = 1000000;
		double minSeenDist = 0;
		int leastRow = 0;
		int leastCol = 0;

		for (int r = 0; r<rows; ++r)
		{
			for (int c = 0; c<cols; ++c)
			{
				// get the center of this cell
				Position cellCenter = getCellCenter(r, c);

				// don't worry about places that aren't connected to our start locatin
				if (!BWTA.isConnected(cellCenter.toTilePosition(), MyBotModule.Broodwar.self().getStartLocation()))
				{
					continue;
				}

				Position home = MyBotModule.Broodwar.self().getStartLocation().toPosition();
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

	public void calculateCellCenters()
	{
		for (int r = 0; r < rows; ++r)
		{
			for (int c = 0; c < cols; ++c)
			{
				GridCell cell = getCellByIndex(r, c);

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

				cell.center = new Position(centerX, centerY);
				assert(cell.center.isValid());
			}
		}
	}

	public Position getCellCenter(int row, int col)
	{
		return getCellByIndex(row, col).center;
	}

	public GridCell getCellByIndex(int r, int c)
	{
		return gridCells.get(r * cols + c); 
	}
	
	// clear the vectors in the grid
	public void clearGrid() {
		for (int i = 0; i< gridCells.size(); ++i)
		{
			gridCells.get(i).ourUnits.getLoadedUnits().clear();
			gridCells.get(i).oppUnits.getLoadedUnits().clear();
		}
	}

	// populate the grid with units
	public void update()
	{
		// clear the grid
		clearGrid();

		//MyBotModule.Broodwar.printf("MapGrid info: WH(%d, %d)  CS(%d)  RC(%d, %d)  C(%d)", mapWidth, mapHeight, cellSize, rows, cols, cells.size());

		// add our units to the appropriate cell
		for (Unit unit : MyBotModule.Broodwar.self().getUnits())
		{
			getCell(unit).ourUnits.getLoadedUnits().add(unit);
			getCell(unit).timeLastVisited = MyBotModule.Broodwar.getFrameCount();
		}

		// add enemy units to the appropriate cell
		for (Unit unit : MyBotModule.Broodwar.enemy().getUnits())
		{
			if (unit.getHitPoints() > 0)
			{
				getCell(unit).oppUnits.getLoadedUnits().add(unit);
				getCell(unit).timeLastOpponentSeen = MyBotModule.Broodwar.getFrameCount();
			}
		}
	}

	public GridCell getCell(Unit unit)
	{
		return getCell(unit.getPosition()); 
	}
	
	public GridCell getCell(Position pos)
	{
		return getCellByIndex(pos.getY() / cellSize, pos.getX() / cellSize);
	}
	
	public void getUnitsNear(Unitset units, Position center, int radius, boolean ourUnits, boolean oppUnits)
	{
		final int x0 = Math.max((center.getX() - radius) / cellSize, 0);
		final int x1 = Math.min((center.getX() + radius) / cellSize, cols-1);
		final int y0 = Math.max((center.getY() - radius) / cellSize, 0);
		final int y1 = Math.min((center.getY() + radius) / cellSize, rows-1);
		final int radiusSq = radius * radius;
		for (int y = y0; y <= y1; ++y)
		{
			for (int x = x0; x <= x1; ++x)
			{
				int row = y;
				int col = x;

				GridCell cell = getCellByIndex(row, col);
				if (ourUnits)
				{
					for (Unit unit : cell.ourUnits.getLoadedUnits())
					{
						Position d = new Position(unit.getPosition().getX() - center.getX(), unit.getPosition().getY() - center.getY());
						if (d.getX() * d.getX() + d.getY() * d.getY() <= radiusSq)
						{
							if (!units.getLoadedUnits().contains(unit))
							{
								units.getLoadedUnits().add(unit);
							}
						}
					}
				}
				if (oppUnits)
				{
					for (Unit unit : cell.oppUnits.getLoadedUnits()) 
					{
						if (unit.getType() != UnitType.Unknown && unit.isVisible())
						{
							Position d = new Position(unit.getPosition().getX() - center.getX(), unit.getPosition().getY() - center.getY());
							if (d.getX() * d.getX() + d.getY() * d.getY() <= radiusSq)
							{
								if (!units.getLoadedUnits().contains(unit))
								{
									units.getLoadedUnits().add(unit);
								}
							}
						}
					}
				}
			}
		}
	}

	int	getCellSize()
	{
		return cellSize;
	}

	int getMapWidth(){
		return mapWidth;
	}

	int getMapHeight(){
		return mapHeight;
	}

	int getRows()
	{
		return rows;
	}

	int getCols()
	{
		return cols;
	}
	
	public final Vector<TilePosition> getSortedTiles()
	{
		return sorted;
	}
}