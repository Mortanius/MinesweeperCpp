#include<SFML/Graphics.hpp>
#include<iostream>
#include<exception>
#include<string>
#include<vector>
#include<deque>
#include"game.hpp"

class MinesweeperView {
public:
	MinesweeperView(MinesweeperGame& game) : game{ game } { initialize(); }
	~MinesweeperView();
	void gameLoop();

private:
	MinesweeperGame& game;
	sf::RectangleShape **grid;
	// Position of the first cell on the window
	sf::Vector2f gridStart;
	// Used to aid in drawing these objects while iterating over the grid
	std::deque<sf::CircleShape> flags;
	std::deque<sf::CircleShape> bombs;
	std::deque<sf::Text> bombNumbers;
	
	sf::RenderWindow window;
	sf::Font font;
	sf::Vector2f rectSize;

	const sf::Color coveredCellColors[2] = { sf::Color(170, 215, 80, 255), sf::Color(160, 210, 70, 255) };
	const sf::Color uncoveredCellColors[2] = { sf::Color(230, 195, 160, 255), sf::Color(215, 185, 155, 255) };
	const sf::Color mouseOverCoveredCellColor = sf::Color(190, 225, 125, 255);
	const sf::Color mouseOverNumberedCellColor = sf::Color(235, 210, 180, 255);
	const sf::Color flagColor = sf::Color(240, 55, 10);
	const sf::Color bombColor = sf::Color::Black;

	// The selected cell. (-1, -1) if None
	sf::Vector2i selectedCell = sf::Vector2i(-1, -1);
	// Instances to be drawn
	std::vector<const sf::Drawable*> drawables;

	void initialize();
	// Keep the rectangles with equal width and height
	void onResize();
	// Read the game state and update the drawable instances
	void updateDrawables();
	void draw();

	sf::Vector2i mousePositionOnGrid();

};

void MinesweeperView::initialize() {
	// Initialize view grid
	this->grid = new sf::RectangleShape*[game.ROWS];
	for (int i = 0; i < game.ROWS; i++) {
		this->grid[i] = new sf::RectangleShape[game.COLUMNS];
	}

	// Initialize bombs and flags
	for (int i = 0; i < game.BOMBS; i++) {
		sf::CircleShape flag(rectSize.y, 3);
		flag.setFillColor(this->flagColor);
		this->flags.push_back(flag);
		sf::CircleShape bomb = sf::CircleShape(rectSize.y);
		bomb.setFillColor(sf::Color::Black);
		this->bombs.push_back(bomb);
	}

	if (!this->font.loadFromFile("arial.ttf")) {
		std::cerr << "Error: font not found" << std::endl;
	}
	// Initialize cell numbers
	for (int i = 0; i < game.ROWS; i++) {
		for (int j = 0; j < game.COLUMNS; j++) {
			if (game.getGrid()[i][j].bombsNearby == 0) {
				continue;
			}
			sf::Text number;
			number.setFont(this->font);
			this->bombNumbers.push_back(number);
		}
	}


	window.create(sf::VideoMode(500, 400), "Minesweeper");

	onResize();
}

void MinesweeperView::updateDrawables() {
	this->drawables.clear();

	const CellInfo** gameGrid = game.getGrid();

	// Set colors on cells
	int whichColorFirst = 0;
	float Y = this->gridStart.y;
	for (int i = 0; i < game.ROWS; i++, Y += rectSize.y, whichColorFirst = (!whichColorFirst & 0b1)) {
		int whichColor = whichColorFirst;
		float X = this->gridStart.x;
		for (int j = 0; j < game.COLUMNS; j++, X += this->rectSize.x, whichColor = (!whichColor & 0b1)) {
			if (gameGrid[i][j].hasFlag) {
				sf::RectangleShape &rect = grid[i][j];
				rect.setSize(rectSize);
				rect.setPosition(sf::Vector2f(X, Y));
				rect.setFillColor(this->coveredCellColors[whichColor]);
				this->drawables.push_back(&rect);

				if (game.isFinished() && gameGrid[i][j].hasBomb) {
					sf::CircleShape bomb = this->bombs.front();
					this->bombs.pop_front();
					bomb.setRadius(rectSize.y / 3.f);
					bomb.setPosition(sf::Vector2f(X + bomb.getRadius() / 2.f, Y + bomb.getRadius() / 2.f));
					this->bombs.push_back(bomb);
					this->drawables.push_back(&this->bombs.back());
				}

				sf::CircleShape flag = this->flags.front();
				this->flags.pop_front();
				flag.setRadius(rectSize.y / 2.f);
				flag.setOrigin(sf::Vector2f(flag.getRadius(), flag.getRadius()));
				flag.setRotation(90.f);
				flag.setPosition(sf::Vector2f(X + flag.getRadius(), Y + flag.getRadius()));
				this->flags.push_back(flag);
				this->drawables.push_back(&this->flags.back());
			}
			else if (gameGrid[i][j].isCovered) {
				sf::RectangleShape &rect = grid[i][j];
				rect.setSize(rectSize);
				rect.setPosition(sf::Vector2f(X, Y));
				rect.setFillColor(this->coveredCellColors[whichColor]);
				this->drawables.push_back(&rect);
				
				if (game.isFinished() && gameGrid[i][j].hasBomb) {
					sf::CircleShape bomb = this->bombs.front();
					this->bombs.pop_front();
					bomb.setRadius(rectSize.x / 3.f);
					bomb.setPosition(sf::Vector2f(X + bomb.getRadius() / 2.f, Y + bomb.getRadius() / 2.f));
					this->bombs.push_back(bomb);
					this->drawables.push_back(&this->bombs.back());
				}
			}
			else {
				sf::RectangleShape &rect = grid[i][j];
				rect.setSize(rectSize);
				rect.setPosition(sf::Vector2f(X, Y));
				rect.setFillColor(this->uncoveredCellColors[whichColor]);
				this->drawables.push_back(&rect);

				if (gameGrid[i][j].bombsNearby > 0) {
					sf::Text number = this->bombNumbers.front();
					this->bombNumbers.pop_front();
					number.setPosition(sf::Vector2f(X, Y));
					number.setString(std::to_string(static_cast<unsigned>(game.getGrid()[i][j].bombsNearby)));
					number.setCharacterSize(this->rectSize.x);
					this->bombNumbers.push_back(number);
					this->drawables.push_back(&this->bombNumbers.back());
				}
			}
		}
	}
}

void MinesweeperView::draw() {
	// Highlight selected cell under mouse cursor

	sf::Color prevColor;
	bool restoreColor = false;
	int sx = this->selectedCell.x, sy = this->selectedCell.y;
	if (sx >= 0 && sy >= 0) {
		const CellInfo& cell = this->game.getGrid()[sx][sy];
		sf::RectangleShape& rect = this->grid[sx][sy];
		if (cell.isCovered) {
			prevColor = sf::Color(rect.getFillColor());
			rect.setFillColor(this->mouseOverCoveredCellColor);
			restoreColor = true;
		}
		else if (cell.bombsNearby > 0) {
			prevColor = sf::Color(rect.getFillColor());
			rect.setFillColor(this->mouseOverNumberedCellColor);
			restoreColor = true;
		}
	}

	// Draw
	for (auto drawable : this->drawables) {
		window.draw(*drawable);
	}

	// Restore cell color
	if (restoreColor && sx >= 0 && sy >= 0) {
		this->grid[sx][sy].setFillColor(prevColor);
	}
	
}

MinesweeperView::~MinesweeperView()
{
	for (int i = 0; i < this->game.ROWS; i++) {
		delete[] this->grid[i];
	}
	delete[] this->grid;
}

sf::Vector2i MinesweeperView::mousePositionOnGrid() {
	sf::Vector2i mp = sf::Mouse::getPosition(window);
	mp.y = (mp.y - this->gridStart.y) / rectSize.y;
	mp.x = (mp.x - this->gridStart.x) / rectSize.x;
	return mp;
}

void MinesweeperView::onResize() {
	const unsigned W = window.getSize().x, H = window.getSize().y;
	window.setView(sf::View(sf::FloatRect(0, 0, W, H)));
	float x0 = 0.f, y0 = 0.f;
	float rectW = static_cast<float>(W) / this->game.COLUMNS;
	float rectH = static_cast<float>(H) / this->game.ROWS;
	if (rectW < rectH) {
		rectH = rectW;
		float diff = static_cast<float>(H) - rectH * this->game.ROWS;
		y0 = diff / 2.f;
		this->rectSize = sf::Vector2f(rectW, rectH);
	}
	else if (rectW > rectH) {
		rectW = rectH;
		float diff = static_cast<float>(W) - rectW * this->game.COLUMNS;
		x0 = diff / 2.f;
	}
	this->rectSize = sf::Vector2f(rectW, rectH);
	this->gridStart = sf::Vector2f(x0, y0);
	updateDrawables();
}

void MinesweeperView::gameLoop() {
	updateDrawables();

	sf::Event event;
	while (window.isOpen() && !game.isFinished())
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::Resized)
			{
				onResize();
			}
			else if (event.type == sf::Event::MouseMoved) {
				sf::Vector2i mp = mousePositionOnGrid();

				if (mp.y >= 0 && mp.y < game.ROWS && mp.x >= 0 && mp.x < game.COLUMNS) {
					this->selectedCell = sf::Vector2i(mp.y, mp.x);
				}
				else {
					this->selectedCell = sf::Vector2i(-1, -1);
				}
			}
			else if (event.type == sf::Event::MouseButtonPressed) {
				sf::Vector2i mp = mousePositionOnGrid();
				if (mp.y >= 0 && mp.y < game.ROWS && mp.x >= 0 && mp.x < game.COLUMNS) {
					bool viewChanged = false;
					switch (event.mouseButton.button) {
					case sf::Mouse::Button::Left: {
						viewChanged = game.uncoverCell(mp.y, mp.x);
						if (viewChanged) {
							updateDrawables();
						}
						break;
					}
					case sf::Mouse::Button::Right: {
						if (!game.getGrid()[mp.y][mp.x].hasFlag) {
							viewChanged = game.placeFlag(mp.y, mp.x);
						}
						else {
							viewChanged = game.removeFlag(mp.y, mp.x);
						}
						if (viewChanged) {
							updateDrawables();
						}
						break;
					}
					}
				}
			}
		}
		window.clear();
		draw();// drawGrid(window, gridView, rows, columns);
		window.display();
	}
	
	if (game.isWon()) {
		std::cout << "You won!!" << std::endl;
	}
	else {
		std::cout << "Game Over!" << std::endl;
	}

	while (window.isOpen())
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::Resized)
			{
				onResize();
			}
		}
		window.clear();
		draw();
		window.display();
	}
}

int main(int argc, char *argv[])
{
	int rows = 8, columns = 10, bombs = 10;
	if (argc != 1 && argc != 4) {
		std::cout << "Usage: minesweeper ROWS COLUMNS BOMBS" << std::endl;
	}
	else if (argc == 4) {
		rows = std::stoi(argv[1]);
		columns = std::stoi(argv[2]);
		bombs = std::stoi(argv[3]);
	}
	try {
		MinesweeperGame game(rows, columns, bombs);
		MinesweeperView gameView(game);
		gameView.gameLoop();
	} catch (const std::exception& e) {
		std::cerr << e.what();
		return -1;
	}

	return 0;
}