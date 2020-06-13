#include<iostream>
#include<string>
#include"game.hpp"

void printGrid(const CellInfo** grid, int rows, int columns) {
	std::cout << "   ";
	for (int i = 0; i < columns; i++) {
		std::cout << i << " ";
	}
	std::cout << std::endl;
	std::cout << "   ";
	for (int i = 0; i < columns; i++) {
		std::cout << "- ";
	}
	std::cout << std::endl;
	for (int i = 0; i < rows; i++) {
		std::cout << i << "| ";
		for (int j = 0; j < columns; j++) {
			if (grid[i][j].hasFlag) {
				std::cout << "> ";
			}
			else if (grid[i][j].isCovered) {
				std::cout << "X ";
			}
			else if (grid[i][j].hasBomb) {
				std::cout << "@ ";
			}
			else {
				std::cout << (unsigned)grid[i][j].bombsNearby << " ";
			}
		}
		std::cout << std::endl;
	}
}

void printGridCheat(const CellInfo** grid, int rows, int columns) {
	std::cout << "   ";
	for (int i = 0; i < columns; i++) {
		std::cout << i << " ";
	}
	std::cout << std::endl;
	std::cout << "   ";
	for (int i = 0; i < columns; i++) {
		std::cout << "- ";
	}
	std::cout << std::endl;
	for (int i = 0; i < rows; i++) {
		std::cout << i << "| ";
		for (int j = 0; j < columns; j++) {
			if (grid[i][j].hasBomb) {
				if (grid[i][j].hasFlag) {
					std::cout << "> ";
				}
				else {
					std::cout << "@ ";
				}
			}
			else {
				std::cout << (unsigned)grid[i][j].bombsNearby << " ";
			}
		}
		std::cout << std::endl;
	}
}



void gameLoop(MinesweeperGame& game) {
	char charInput;
	while (!game.isFinished()) {
		std::cout << std::endl;
		printGrid(game.getGrid(), game.ROWS, game.COLUMNS);
		const CellInfo **grid = game.getGrid();
		std::cout << "Cells remaining: " << game.getCellsRemaining() << std::endl;
		std::cout << "Flags remaining: " << game.BOMBS - game.getFlagsCount() << std::endl;
		std::cout << "[D]: Dig" << std::endl;
		std::cout << "[F]: Place a flag" << std::endl;
		std::cout << "[R]: Remove a flag" << std::endl;
		
		std::cin >> charInput;
		switch (charInput)
		{
		case 'D': {
			int i = 0, j = 0;
			std::cout << "Row: ";
			std::cin >> i;
			std::cout << "Column: ";
			std::cin >> j;
			bool ret = game.uncoverCell(i, j);
			break;
		}
		case 'F': {
			int i = 0, j = 0;
			std::cout << "Row: ";
			std::cin >> i;
			std::cout << "Column: ";
			std::cin >> j;
			bool ret = game.placeFlag(i, j);
			break;
		}
		case 'R': {
			int i = 0, j = 0;
			std::cout << "Row: ";
			std::cin >> i;
			std::cout << "Column: ";
			std::cin >> j;
			bool ret = game.removeFlag(i, j);
		}
		default:
			break;
		}
	}
	printGridCheat(game.getGrid(), game.ROWS, game.COLUMNS);
	if (game.isWon()) {
		std::cout << "You won!!" << std::endl;
	}
	else {
		std::cout << "You lose!" << std::endl;
	}
}

int main()
{
	int rows, columns, bombs;
	std::cout << "~~Minesweeper~~" << std::endl << std::endl;
	std::cout << "Number of rows: ";
	std::cin >> rows;
	std::cout << "Number of columns: ";
	std::cin >> columns;
	std::cout << "Number of bombs: ";
	std::cin >> bombs;
	try {
		MinesweeperGame game(rows, columns, bombs);
		gameLoop(game);
	}
	catch (const std::invalid_argument& e) {
		std::cerr << e.what();
		return -1;
	}
	return 0;
}