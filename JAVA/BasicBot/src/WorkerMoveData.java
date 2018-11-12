import bwapi.Position;

public class WorkerMoveData
{
	private int mineralsNeeded;
	private int gasNeeded;
	private Position position;

	public WorkerMoveData(int m, int g, Position p)
	{
		mineralsNeeded = m;
		gasNeeded = g;
		position = p;
	}

	public int getMineralsNeeded() {
		return mineralsNeeded;
	}

	public int getGasNeeded() {
		return gasNeeded;
	}

	public Position getPosition() {
		return position;
	}
};