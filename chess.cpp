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

struct Pos {
	int row;
	int col;
};

struct Cell {
	Content color;
	Piece type;
};

using Board = std::array<Cell, 8 * 8>;

constexpr int size = 800;
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
		if (row > 7 || row < 0 || col>7 || col < 0)
			throw "FUCKING OUT OF RANGE";

		return board->at((8 - row - 1) * 8 + col);
	}

	static Board* board;
};

class DrawCell {
public:
	static void draw(int row, int col, Color box, Color letter) {

		DrawRectangle(col * cellSize, (8 - row - 1) * cellSize, cellSize, cellSize, box);
		DrawText(names[GetCell::get(row, col).type],
			col * cellSize + padding + border + contBorder,
			(8 - row - 1) * cellSize + padding + border + contBorder,
			contSize - contBorder * 2, letter);
	}
	static void draw(int row, int col, Color box) {
		draw(row, col, box, (GetCell::get(row, col).color == C_WHITE) ? WHITE : ((GetCell::get(row, col).color == C_BLACK) ? BLACK : BLANK));
	}
	static void draw(int row, int col) {
		draw(row, col, ((row + col) % 2) ? LIGHTGRAY : DARKGRAY);
	}
};

std::vector<Pos> getPossibleMoves(Pos ppos) {

	auto& piece = GetCell::get(ppos.row, ppos.col);
	
	std::vector<Pos> moves;
	moves.clear();
	if (piece.color != C_BLANK) {

		auto compCol = (piece.color == C_WHITE) ? C_BLACK : C_WHITE;

		auto fillpawn = [&]() {
			Pos p;
			p.col = ppos.col;
			auto frontrow = ppos.row + ((piece.color == C_WHITE) ? 1 : -1);
			p.row = frontrow;
			try {
				if (GetCell::get(frontrow, ppos.col).color == C_BLANK) {
					moves.push_back(p);
					try {
						if (piece.color == C_WHITE && ppos.row == 1) {
							if (GetCell::get(3, ppos.col).color == C_BLANK) {
								p.row = 3;
								p.col = ppos.col;
								moves.push_back(p);
							}
						}
						else if (piece.color == C_BLACK && ppos.row == 6) {
							if (GetCell::get(4, ppos.col).color == C_BLANK) {
								p.row = 4;
								p.col = ppos.col;
								moves.push_back(p);
							}
						}
					}
					catch (...) {

					}
				}
			}
			catch (...) {
			}
			try {
				if (GetCell::get(frontrow, ppos.col + 1).color == compCol) {
					p.col = ppos.col + 1;
					p.row = frontrow;
					moves.push_back(p);
				}
			}
			catch (...) {

			}
			try {
				if (GetCell::get(frontrow, ppos.col - 1).color == compCol) {
					p.col = ppos.col - 1;
					p.row = frontrow;
					moves.push_back(p);
				}

			}
			catch (...) {

			}


		};

		//pushes a move if the place is not occupied by friend , returns true it it was empty
		auto tmpfunc = [&](Pos p)->bool {
			if (GetCell::get(p.row, p.col).color == C_BLANK)
				moves.push_back(p);
			else {
				if (GetCell::get(p.row, p.col).color == compCol)
					moves.push_back(p);
				return false;
			}
			return true;
		};

		auto fillrook = [&]() {
			Pos p;
			p.row = ppos.row;
			p.col = ppos.col;

			while (((++p.row) < 8) && tmpfunc(p));
			p.row = ppos.row;
			while (((--p.row) >= 0) && tmpfunc(p));
			p.row = ppos.row;
			while (((++p.col) < 8) && tmpfunc(p));
			p.col = ppos.col;
			while (((--p.col) >= 0) && tmpfunc(p));
		};
		auto fillknight = [&]() {

		};
		auto fillbishop = [&]() {
			Pos p;
			p.row = ppos.row;
			p.col = ppos.col;
			while (((++p.row) < 8)&& ((++p.col) < 8) && tmpfunc(p));

			p.row = ppos.row;
			p.col = ppos.col;
			while (((++p.row) < 8)&& ((--p.col) >= 0) && tmpfunc(p));

			p.row = ppos.row;
			p.col = ppos.col;
			while (((--p.row) >= 0)&& ((++p.col) < 8) && tmpfunc(p));

			p.row = ppos.row;
			p.col = ppos.col;
			while (((--p.row) >= 0)&& ((--p.col) >= 0) && tmpfunc(p));

		};
		auto fillqueen=[&]() {
			Pos p;
			p.row = ppos.row;
			p.col = ppos.col;
			fillbishop();
			fillrook();
		};
		auto fillking=[&]() {
			Pos p;
			p.row = ppos.row;
			p.col = ppos.col;
			try {
				++p.row;
				tmpfunc(p);
			}
			catch (...) {

			}
			try {
				++p.col;
				tmpfunc(p);
			}
			catch (...) {

			}
			try {
				p.col -= 2;
				tmpfunc(p);
			}
			catch (...) {

			}
			try {
				--p.row;
				tmpfunc(p);
			}
			catch (...) {

			}
			try {
				p.col += 2;
				tmpfunc(p);
			}
			catch (...) {

			}
			try {
				--p.row;
				tmpfunc(p);
			}
			catch (...) {

			}
			try {
				--p.col;
				tmpfunc(p);
			}
			catch (...) {

			}
			try {
				--p.col;
				tmpfunc(p);
			}
			catch (...) {

			}
		};


		switch (piece.type) {

		case PAWN:

			fillpawn();
			break;

		case ROOK:

			fillrook();
			break;

		case KNIGHT:
			fillknight();
			break;
		case BISHOP:
			fillbishop();
			break;
		case QUEEN:
			fillqueen();
			break;
		case KING:
			fillking();
			break;


		default:
			break;
		}

	}
	return moves;
}

std::vector<Pos> getPossibleMoves(int row, int col) {
	Pos p;
	p.row = row;
	p.col = col;
	return getPossibleMoves(p);
}
Board* GetCell::board = nullptr;

int chess() {

	InitWindow(size, size*1.1, "Sudoku");
	SetTargetFPS(60);

	Board* board = new Board;

	GetCell::board = board;

	for (auto& c : *board) {
		c.color = C_BLANK;
		c.type = NONE;
	}


	for (int col = 0; col < 8; ++col) {
		GetCell::get(1, col).color = C_WHITE;
		GetCell::get(1, col).type = PAWN;

		GetCell::get(6, col).color = C_BLACK;
		GetCell::get(6, col).type = PAWN;

	}

	//initializing rook
	GetCell::get(0, 0).color = C_WHITE;
	GetCell::get(0, 0).type = ROOK;
	GetCell::get(7, 0).color = C_BLACK;
	GetCell::get(7, 0).type = ROOK;
	GetCell::get(0, 7).color = C_WHITE;
	GetCell::get(0, 7).type = ROOK;
	GetCell::get(7, 7).color = C_BLACK;
	GetCell::get(7, 7).type = ROOK;
	
	//initializing knight
	GetCell::get(0, 1).color = C_WHITE;
	GetCell::get(0, 1).type = KNIGHT;
	GetCell::get(7, 1).color = C_BLACK;
	GetCell::get(7, 1).type = KNIGHT;
	GetCell::get(0, 6).color = C_WHITE;
	GetCell::get(0, 6).type = KNIGHT;
	GetCell::get(7, 6).color = C_BLACK;
	GetCell::get(7, 6).type = KNIGHT;
	
	//initializing bishop
	GetCell::get(0, 2).color = C_WHITE;
	GetCell::get(0, 2).type = BISHOP;
	GetCell::get(7, 2).color = C_BLACK;
	GetCell::get(7, 2).type = BISHOP;
	GetCell::get(0, 5).color = C_WHITE;
	GetCell::get(0, 5).type = BISHOP;
	GetCell::get(7, 5).color = C_BLACK;
	GetCell::get(7, 5).type = BISHOP;
	
	//initializing queen
	GetCell::get(0, 3).color = C_WHITE;
	GetCell::get(0, 3).type = QUEEN;
	GetCell::get(7, 3).color = C_BLACK;
	GetCell::get(7, 3).type = QUEEN;

	//Initializing king
	GetCell::get(0, 4).color = C_WHITE;
	GetCell::get(0, 4).type = KING;
	GetCell::get(7, 4).color = C_BLACK;
	GetCell::get(7, 4).type = KING;

	Pos select;
	Pos move;
	select.row = -1;
	select.col = -1;
	move.row = -1;
	move.col = -1;
	bool isCheck = false;
	Content currplayer = C_WHITE;
	while (!WindowShouldClose()) {
		std::vector<Pos> moves;
		moves.clear();
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			auto pos = GetMousePosition();
			if (pos.x >= 0 && pos.x < size && pos.y >= 0 && pos.y < size) {
				int r = pos.y / cellSize;
				int c = pos.x / cellSize;
				r = 8 - r - 1;

				//If a cell is already selected
				if (select.row >= 0 && select.col >= 0) {

					//If clicked on selected cell , unselect
					if (select.row == r && select.col == c) {
						select.row = -1;
						select.col = -1;
					}
					else {
						move.row = -1;
						move.col = -1;
						//if selected in one of valid moves , set moving to that cell
						moves = getPossibleMoves(select);
						for (auto& s : moves) {
							if (r == s.row && c == s.col) {
								move = s;
								break;
							}
						}
						//if not selected in any of valid moves , 
						if ((move.row < 0 || move.col < 0) && (GetCell::get(r, c).color == currplayer)) {
							select.row = r;
							select.col = c;
						}
					}

				}
				else if(GetCell::get(r, c).color == currplayer) {
					select.row = r;
					select.col = c;
				}
			
			}
		}

		//If movable then move it, try catch block will manage if not movable
		try {
			moves = getPossibleMoves(select);
			Cell& src = GetCell::get(select.row, select.col);
			Cell& des = GetCell::get(move.row, move.col);
			des = src;
			src.type = NONE;
			src.color = C_BLANK;

			//Check for check to enemy king
			auto newMoves = getPossibleMoves(move);
			auto enemyCol = ((GetCell::get(move.row, move.col).color == C_WHITE) ? C_BLACK : C_WHITE);
			isCheck = false;
			for (auto& c : newMoves) {
				if (GetCell::get(c.row, c.col).color == enemyCol && GetCell::get(c.row, c.col).type == KING) {
					isCheck = true;
					break;
				}
			}

			select.row = -1;
			select.col = -1;
			move.row = -1;
			move.col = -1;
			currplayer = (currplayer == C_WHITE) ? C_BLACK : C_WHITE;
		}
		catch (...) {

		}


		ClearBackground(LIGHTGRAY);
		BeginDrawing();
		for (int row = 0; row < 8; ++row)
			for (int col = 0; col < 8; ++col) {
				if (row != select.row || col != select.col)
					DrawCell::draw(row, col);
				else
					DrawCell::draw(row, col, BLUE);
			}

		for (auto& c : moves) {
			DrawCell::draw(c.row, c.col, SKYBLUE);
		}
		std::string sout = "PLAYER : " + std::string(((currplayer == C_WHITE) ? "WHITE " : "BLACK "));

		if (isCheck) {
			sout += "CHECK ";
		}
		
		DrawText(sout.c_str(), contBorder + border, size + contBorder + border, cellSize/3, RED);


		EndDrawing();
	}
	delete board;
	return 0;
}
 