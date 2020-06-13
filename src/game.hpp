#pragma once

#include<stdexcept>
#include<random>
#include<algorithm>

struct CellInfo {
	// 1 byte integer
	char bombsNearby = 0;
	bool hasBomb = false;
	bool hasFlag = false;
	bool isCovered = true;
};

class MinesweeperGame
{
public:
	const int ROWS, COLUMNS, BOMBS;
	MinesweeperGame(int rows, int columns, int bombs) : ROWS{ rows }, COLUMNS{ columns }, BOMBS{ bombs } { initialize(); }
	~MinesweeperGame();

	// Player Input
	// Uncover cell at position
	bool uncoverCell(int row, int column);
	// Place a flag at position
	bool placeFlag(int row, int column);
	// Remove a flag at position
	bool removeFlag(int row, int column);

	// Game Output
	bool isFinished() { return this->finished; }
	bool isWon() { return this->cellsRemaining == 0; }
	int getCellsRemaining() { return this->cellsRemaining; };
	int getFlagsCount() { return this->flagsCount; };
	const CellInfo** getGrid() { return const_cast<const CellInfo**>(this->grid); };
private:
	// When zero, the player wins the game
	int cellsRemaining;
	// Number of flags placed
	int flagsCount = 0;
	CellInfo **grid;
	bool finished = false;
	// Initialize game
	void initialize();
	// Uncover cell and alter game state (decrease cellsRemaining)
	void uncoverCell(CellInfo& cell);
	// Reveal surrounding cells when the user uncovers a cell containing 0 nearby bombs
	void uncoverSurroundingCells(int row, int column);
	// Place/Remove flag and alter game state (decrease flagsCount)
	void placeFlag(CellInfo& cell);
	void removeFlag(CellInfo& cell);

};

void MinesweeperGame::initialize()
{
	if (this->ROWS <= 0 || this->COLUMNS <= 0) {
		throw std::invalid_argument{ "Row and Column numbers must be positive" };
	}
	if (this->BOMBS >= this->ROWS * this->COLUMNS) {
		throw std::invalid_argument{ "Too many bombs" };
	}
	// Cells needed to be uncovered to win the game
	cellsRemaining = this->ROWS * this->COLUMNS - this->BOMBS;

	// Initialize grid with bombs and numbers
	this->grid = new CellInfo* [this->ROWS];
	for (int i = 0; i < this->ROWS; i++) {
		this->grid[i] = new CellInfo[this->COLUMNS];
	}

	// Init Random Number Generation
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist_rows(0, this->ROWS - 1),
															 dist_columns(0, this->COLUMNS - 1);
	// Place bombs in the grid randomly
	for (int b = BOMBS; b > 0; b--) {
		int i = 0, j = 0;
		do {
			i = dist_rows(rng);
			j = dist_columns(rng);
		} while (this->grid[i][j].hasBomb);
		this->grid[i][j].hasBomb = true;
	}

	// Place number of nearby bombs on each cell
	for (int i = 0; i < this->ROWS; i++) {
		for (int j = 0; j < this->COLUMNS; j++) {
			if (this->grid[i][j].hasBomb) {
				continue;
			}
			// Count nearby bombs
			for (int x = std::max(0, i - 1); x <= std::min(i + 1, this->ROWS-1); x++) {
				for (int y = std::max(0, j - 1); y <= std::min(j + 1, this->COLUMNS-1); y++) {
					if (x == i && y == j) {
						continue;
					}
					if (this->grid[x][y].hasBomb) {
						this->grid[i][j].bombsNearby++;
					}
				}
			}
		}
	}
}

MinesweeperGame::~MinesweeperGame()
{
	for (int i = 0; i < this->ROWS; i++) {
		delete[] this->grid[i];
	}
	delete[] this->grid;
}

bool MinesweeperGame::uncoverCell(int row, int column) {
	if (row < 0 || row >= this->ROWS || column < 0 || column >= this->COLUMNS) {
		return false;
	}
	CellInfo& cell = this->grid[row][column];
	if (cell.hasFlag) {
		// Do not uncover a cell with a flag
		return false;
	}
	if (!cell.isCovered) {
		// Already has been uncovered
		return false;
	}
	if (cell.hasBomb) {
		// Has a bomb, game over
		this->finished = true;
		return true;
	}
	if (cell.bombsNearby > 0) {
		// If there are at least 1 bomb nearby, just uncover this one
		uncoverCell(cell);
	}
	else {
		// If there is no bomb nearby, reveal this and all nearby cells with nearby bombs
		uncoverSurroundingCells(row, column);
	}

	// Check win game condition
	if (this->cellsRemaining == 0) {
		this->finished = true;
	}

	return true;
}

bool MinesweeperGame::placeFlag(int row, int column)
{
	if (this->flagsCount == this->BOMBS ||
		row < 0 || row >= this->ROWS || column < 0 || column >= this->COLUMNS) {
		return false;
	}
	CellInfo& cell = this->grid[row][column];
	if (cell.hasFlag || !cell.isCovered) {
		return false;
	}
	placeFlag(cell);
	return true;
}

void MinesweeperGame::placeFlag(CellInfo& cell) {
	cell.hasFlag = true;
	this->flagsCount++;
}

bool MinesweeperGame::removeFlag(int row, int column)
{
	if (this->flagsCount == 0 ||
		row < 0 || row >= this->ROWS || column < 0 || column >= this->COLUMNS) {
		return false;
	}
	CellInfo& cell = this->grid[row][column];
	if (!cell.hasFlag) {
		return false;
	}
	removeFlag(cell);
	return true;
}

void MinesweeperGame::removeFlag(CellInfo& cell) {
	cell.hasFlag = false;
	this->flagsCount--;
}

void MinesweeperGame::uncoverCell(CellInfo& cell) {
	if (!cell.isCovered || cell.hasBomb) return;
	if (cell.hasFlag) {
		// Remove existing flag before uncovering
		removeFlag(cell);
	}
	cell.isCovered = false;
	this->cellsRemaining--;
}

void MinesweeperGame::uncoverSurroundingCells(int row, int column) {
	uncoverCell(this->grid[row][column]);
	for (int x = std::max(0, row - 1); x <= std::min(row + 1, this->ROWS - 1); x++) {
		for (int y = std::max(0, column - 1); y <= std::min(column + 1, this->COLUMNS - 1); y++) {
			if (x == row && y == column) {
				continue;
			}
			CellInfo& cell = this->grid[x][y];
			if (cell.bombsNearby > 0) {
				uncoverCell(cell);
			} else if (cell.isCovered) {
				// If the cell in position (x, y) also has 0 bombs nearby and is still covered,
				// recursively uncover nearby cells
				uncoverSurroundingCells(x, y);
			}
		}
	}
}