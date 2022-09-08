#include <raylib-cpp.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <utility>
#include <functional>

enum Piece {
	PAWN,
	ROOK,
	KNIGHT,
	BISHOP,
	QUEEN,
	KING,
	NONE
};

const char* names[] = {
	"P","R","H","B","Q","K",""
};

enum Content {
	C_BLANK,
	C_WHITE,
	C_BLACK
};

using Cell = std::pair<Content, Piece>;
using Board = std::array<Cell, 8 * 8>;

constexpr int size = 900;
float cellSize = size / 8.0;
float padding = 0.1 * cellSize;
float border = padding / 2;
float contSize = cellSize - (padding + border) * 2;
float contBorder = contSize * 0.1;


//Chess follow bottom to top & left to right model FYI
//Row is denoted by 1 2 ... column y A B ...
class GetCell {
public:
	
	static Cell& get(int row, int col) {
		return board->at((8 - row - 1) * 8 + col);
	}

	static Board* board;
};

class DrawCell {
public:
	static void draw(int row, int col) {

		DrawRectangle(col * cellSize, (8 - row - 1) * cellSize, cellSize, cellSize, ((row + col) % 2) ? LIGHTGRAY : DARKGRAY);
		switch (GetCell::get(row, col).first) {
		case C_WHITE:
			//DrawText(names[GetCell::get(row, col).second], col * cellSize + padding + border, (8 - row-1) * cellSize + padding + border, contSize, BLACK);
			DrawText(names[GetCell::get(row, col).second], col * cellSize + padding + border + contBorder, (8 - row - 1) * cellSize + padding + border + contBorder, contSize - contBorder * 2, WHITE);
			break;
		case C_BLACK:
			//DrawText(names[GetCell::get(row, col).second], col * cellSize + padding + border, (8 - row-1) * cellSize + padding + border, contSize, WHITE);
			DrawText(names[GetCell::get(row, col).second], col * cellSize + padding + border + contBorder, (8 - row - 1) * cellSize + padding + border + contBorder, contSize - contBorder * 2, BLACK);
			break;
		}
	}
};


class Iterator {
public:
	Iterator(int row, int col)  {
		cell_row = row;
		cell_col = col;
		curr_row = row;
		curr_col = col;
	}

	Cell& get_next() {
		
	}

	int cell_row;
	int cell_col;
	int curr_row;
	int curr_col;
};


Board* GetCell::board = nullptr;

int chess() {

	InitWindow(size, size, "Sudoku");
	SetTargetFPS(60);

	Board* board = new Board;

	GetCell::board = board;

	for (int col = 0; col < 8; ++col) {
		GetCell::get(0, col).first = C_WHITE;
		GetCell::get(0, col).second = PAWN;

		GetCell::get(7, col).first = C_BLACK;
		GetCell::get(7, col).second = PAWN;

	}

	//initializing rook
	GetCell::get(1, 0).first = C_WHITE;
	GetCell::get(1, 0).second = ROOK;
	GetCell::get(6, 0).first = C_BLACK;
	GetCell::get(6, 0).second = ROOK;
	GetCell::get(1, 7).first = C_WHITE;
	GetCell::get(1, 7).second = ROOK;
	GetCell::get(6, 7).first = C_BLACK;
	GetCell::get(6, 7).second = ROOK;
	
	//initializing knight
	GetCell::get(1, 1).first = C_WHITE;
	GetCell::get(1, 1).second = KNIGHT;
	GetCell::get(6, 1).first = C_BLACK;
	GetCell::get(6, 1).second = KNIGHT;
	GetCell::get(1, 6).first = C_WHITE;
	GetCell::get(1, 6).second = KNIGHT;
	GetCell::get(6, 6).first = C_BLACK;
	GetCell::get(6, 6).second = KNIGHT;
	
	//initializing bishop
	GetCell::get(1, 2).first = C_WHITE;
	GetCell::get(1, 2).second = BISHOP;
	GetCell::get(6, 2).first = C_BLACK;
	GetCell::get(6, 2).second = BISHOP;
	GetCell::get(1, 5).first = C_WHITE;
	GetCell::get(1, 5).second = BISHOP;
	GetCell::get(6, 5).first = C_BLACK;
	GetCell::get(6, 5).second = BISHOP;
	
	//initializing queen
	GetCell::get(1, 3).first = C_WHITE;
	GetCell::get(1, 3).second = QUEEN;
	GetCell::get(6, 3).first = C_BLACK;
	GetCell::get(6, 3).second = QUEEN;

	//Initializing king
	GetCell::get(1, 4).first = C_WHITE;
	GetCell::get(1, 4).second = KING;
	GetCell::get(6, 4).first = C_BLACK;
	GetCell::get(6, 4).second = KING;




	while (!WindowShouldClose()) {
		ClearBackground(WHITE);
		BeginDrawing();
		for (int row = 0; row < 8; ++row)
			for (int col = 0; col < 8; ++col)
				DrawCell::draw(row, col);

		EndDrawing();
	}
	delete board;
	return 0;
}
