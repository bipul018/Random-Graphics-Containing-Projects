#include <raylib-cpp.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <utility>
#include <functional>
#include <set>

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
	bool operator <(Pos b)const {
		return ((row * 8 + col) < (b.row * 8 + b.col));
	}
};

struct Cell {
	Content color;
	Piece type;
};

//Chess follow bottom to top & left to right model FYI
//Row is denoted by 1 2 ... column y A B ...

class Board : public std::array<Cell, 8 * 8>{
public:

	Cell& at(int row, int col) {

		if (row > 7 || row < 0 || col>7 || col < 0)
			throw "FUCKING OUT OF RANGE";

		return array::at((8 - row - 1) * 8 + col);
	}
	Cell& at(Pos ppos) {
		return at(ppos.row, ppos.col);
	}

	std::vector<Pos> getmoves(int row, int col) {
		Pos p;
		p.row = row;
		p.col = col;
		return getmoves(p);
	}

	std::vector<Pos> getmoves(Pos ppos) {

		auto& piece = at(ppos);

		std::vector<Pos> moves;
		moves.clear();
		if (piece.color != C_BLANK) {

			auto compCol = (piece.color == C_WHITE) ? C_BLACK : C_WHITE;

			auto fillpawn = [&,this]() {
				Pos p;
				p.col = ppos.col;
				auto frontrow = ppos.row + ((piece.color == C_WHITE) ? 1 : -1);
				p.row = frontrow;
				try {
					if (at(frontrow, ppos.col).color == C_BLANK) {
						moves.push_back(p);
						try {
							if (piece.color == C_WHITE && ppos.row == 1) {
								if (at(3, ppos.col).color == C_BLANK) {
									p.row = 3;
									p.col = ppos.col;
									moves.push_back(p);
								}
							}
							else if (piece.color == C_BLACK && ppos.row == 6) {
								if (at(4, ppos.col).color == C_BLANK) {
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
					if (at(frontrow, ppos.col + 1).color == compCol) {
						p.col = ppos.col + 1;
						p.row = frontrow;
						moves.push_back(p);
					}
				}
				catch (...) {

				}
				try {
					if (at(frontrow, ppos.col - 1).color == compCol) {
						p.col = ppos.col - 1;
						p.row = frontrow;
						moves.push_back(p);
					}

				}
				catch (...) {

				}


			};

			//pushes a move if the place is not occupied by friend , returns true it it was empty
			auto tmpfunc = [&,this](Pos p)->bool {
				try {
					if (at(p.row, p.col).color == C_BLANK)
						moves.push_back(p);
					else {
						if (at(p.row, p.col).color == compCol)
							moves.push_back(p);
						return false;
					}
				}
				catch (...) {
					return false;
				}
				return true;
			};

			auto fillrook = [&,this]() {
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
			auto fillknight = [&,this]() {
				Pos p;
				p.row = ppos.row;
				p.col = ppos.col;

				p.row += 2; ++p.col;
				tmpfunc(p);

				--p.row; ++p.col;
				tmpfunc(p);

				p.row -= 2;
				tmpfunc(p);

				--p.row; --p.col;
				tmpfunc(p);

				p.col -= 2;
				tmpfunc(p);

				++p.row; --p.col;
				tmpfunc(p);

				p.row += 2;
				tmpfunc(p);

				++p.row; ++p.col;
				tmpfunc(p);

			};
			auto fillbishop = [&,this]() {
				Pos p;
				p.row = ppos.row;
				p.col = ppos.col;
				while (((++p.row) < 8) && ((++p.col) < 8) && tmpfunc(p));

				p.row = ppos.row;
				p.col = ppos.col;
				while (((++p.row) < 8) && ((--p.col) >= 0) && tmpfunc(p));

				p.row = ppos.row;
				p.col = ppos.col;
				while (((--p.row) >= 0) && ((++p.col) < 8) && tmpfunc(p));

				p.row = ppos.row;
				p.col = ppos.col;
				while (((--p.row) >= 0) && ((--p.col) >= 0) && tmpfunc(p));

			};
			auto fillqueen = [&,this]() {
				Pos p;
				p.row = ppos.row;
				p.col = ppos.col;
				fillbishop();
				fillrook();
			};
			auto fillking = [&,this]() {
				Pos p;
				p.row = ppos.row;
				p.col = ppos.col;
				++p.row;
				tmpfunc(p);

				++p.col;
				tmpfunc(p);

				p.col -= 2;
				tmpfunc(p);

				--p.row;
				tmpfunc(p);

				p.col += 2;
				tmpfunc(p);

				--p.row;
				tmpfunc(p);

				--p.col;
				tmpfunc(p);

				--p.col;
				tmpfunc(p);
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
};
//
//using Board = std::array<Cell, 8 * 8>;



constexpr int size = 800;
float cellSize = size / 8.0;
float padding = 0.1 * cellSize;
float border = padding / 2;
float contSize = cellSize - (padding + border) * 2;
float contBorder = contSize * 0.1;



class DrawCell {
public:
	static void draw(int row, int col, Color box, Color letter) {

		DrawRectangle(col * cellSize, (8 - row - 1) * cellSize, cellSize, cellSize, box);
		DrawText(names[board->at(row, col).type],
			col * cellSize + padding + border + contBorder,
			(8 - row - 1) * cellSize + padding + border + contBorder,
			contSize - contBorder * 2, letter);
	}
	static void draw(int row, int col, Color box) {
		draw(row, col, box, (board->at(row, col).color == C_WHITE) ? WHITE : ((board->at(row, col).color == C_BLACK) ? BLACK : BLANK));
	}
	static void draw(int row, int col) {
		draw(row, col, ((row + col) % 2) ? LIGHTGRAY : DARKGRAY);
	}
	static Board* board;
};
Board* DrawCell::board = nullptr;

int chess() {

	std::cout << "Enter one of following character \n  vs human : h || vs computer : c";
	char ch;
	std::cin >> ch;

	bool isAI = (ch != 'h');


	InitWindow(size, size*1.1, "Sudoku");
	SetTargetFPS(60);

	Board* board = new Board;

	DrawCell::board = board;

	for (auto& c : *board) {
		c.color = C_BLANK;
		c.type = NONE;
	}


	for (int col = 0; col < 8; ++col) {
		board->at(1, col).color = C_WHITE;
		board->at(1, col).type = PAWN;

		board->at(6, col).color = C_BLACK;
		board->at(6, col).type = PAWN;

	}

	//initializing rook
	board->at(0, 0).color = C_WHITE;
	board->at(0, 0).type = ROOK;
	board->at(7, 0).color = C_BLACK;
	board->at(7, 0).type = ROOK;
	board->at(0, 7).color = C_WHITE;
	board->at(0, 7).type = ROOK;
	board->at(7, 7).color = C_BLACK;
	board->at(7, 7).type = ROOK;
	
	//initializing knight
	board->at(0, 1).color = C_WHITE;
	board->at(0, 1).type = KNIGHT;
	board->at(7, 1).color = C_BLACK;
	board->at(7, 1).type = KNIGHT;
	board->at(0, 6).color = C_WHITE;
	board->at(0, 6).type = KNIGHT;
	board->at(7, 6).color = C_BLACK;
	board->at(7, 6).type = KNIGHT;
	
	//initializing bishop
	board->at(0, 2).color = C_WHITE;
	board->at(0, 2).type = BISHOP;
	board->at(7, 2).color = C_BLACK;
	board->at(7, 2).type = BISHOP;
	board->at(0, 5).color = C_WHITE;
	board->at(0, 5).type = BISHOP;
	board->at(7, 5).color = C_BLACK;
	board->at(7, 5).type = BISHOP;
	
	//initializing queen
	board->at(0, 3).color = C_WHITE;
	board->at(0, 3).type = QUEEN;
	board->at(7, 3).color = C_BLACK;
	board->at(7, 3).type = QUEEN;

	//Initializing king
	board->at(0, 4).color = C_WHITE;
	board->at(0, 4).type = KING;
	board->at(7, 4).color = C_BLACK;
	board->at(7, 4).type = KING;

	Pos select;
	Pos move;
	select.row = -1;
	select.col = -1;
	move.row = -1;
	move.col = -1;
	bool isCheck = false;
	Content currplayer = C_WHITE;
	std::set<Pos> whites;
	std::set<Pos> blacks;

	for (int col = 0; col < 8; ++col) {
		Pos p;
		p.col = col;
		p.row = 0;
		whites.insert(p);
		p.row = 1;
		whites.insert(p);

		p.row = 6;
		blacks.insert(p);
		p.row = 7;
		blacks.insert(p);
	}

	std::string movemessage = "";
	size_t aiFails = 0;
	while (!WindowShouldClose()) {
		std::string sout = "";

		std::vector<Pos> moves;
		//sets set of current and opposing player cells
		auto& curr = (currplayer == C_WHITE) ? whites : blacks;
		auto& opps = (currplayer == C_BLACK) ? whites : blacks;


		moves.clear();
		if (currplayer == C_BLACK && isAI) {

			//Gets number of pieces still on board
			size_t size = curr.size();

			//Gets a random choice among available pieces
			size_t choice = GetRandomValue(0, size - 1);
			
			//Part that iterates and receives the choice
			auto  x = curr.begin();
			for (int i = 0; i < choice; ++i)
				++x;
			std::set<Pos> moves;

			//sets all possible moves of that pieces in moves set
			for (auto m : board->getmoves(*x))
				moves.insert(m);

			//Position of enemy??
			size_t enemypos = 0;

			//set of all places the enemy has eyes on
			std::set<Pos> allEnemies;

			//Part that fills the set where all possible enemy moves are stored
			for (auto m : opps) {
				for (auto y : board->getmoves(m)) {
					allEnemies.insert(y);
				}
			}
			//Erases those positions from moves set that are already in enemy target
			//Try erasing only if all are not covered
			if(aiFails > curr.size())
				for (auto m : allEnemies) {
					moves.erase(m);
				}

			//select one of the choices from enemy moves
			select = *x;
			if (moves.size() > 0) {
				x = moves.begin();
				choice = GetRandomValue(0, moves.size() - 1);
				for (int i = 0; i < choice; ++i)
					++x;

				move = *x;
			}
			else {
				++aiFails;
			}


		}
		else {

			if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || (GetTouchPointCount()>0)) {
				auto pos = GetMousePosition();
				if (GetTouchPointCount() > 0) {
					pos = GetTouchPosition(0);
				}
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
							moves = board->getmoves(select);
							for (auto& s : moves) {
								if (r == s.row && c == s.col) {
									move = s;
									break;
								}
							}
							//if not selected in any of valid moves , 
							if ((move.row < 0 || move.col < 0) && (board->at(r, c).color == currplayer)) {
								select.row = r;
								select.col = c;
							}
						}

					}
					else if (board->at(r, c).color == currplayer) {
						select.row = r;
						select.col = c;
					}

				}
			}
		}
		//If movable then move it, try catch block will manage if not movable
		try {
			moves = board->getmoves(select);
			Cell& src = board->at(select.row, select.col);
			Cell& des = board->at(move.row, move.col);

			curr.erase(select);
			if (des.color != C_BLANK)
				opps.erase(move);
			curr.insert(move);

			des = src;
			src.type = NONE;
			src.color = C_BLANK;

			//Check for check to enemy king
			auto newMoves = board->getmoves(move);
			auto enemyCol = ((board->at(move.row, move.col).color == C_WHITE) ? C_BLACK : C_WHITE);
			isCheck = false;
			for (auto& c : newMoves) {
				if (board->at(c.row, c.col).color == enemyCol && board->at(c.row, c.col).type == KING) {
					isCheck = true;
					break;
				}
			}

			movemessage = char('A' + select.col) + std::to_string(select.row+1) + " -> " + char('A' + move.col)  + std::to_string(move.row+1);


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

		sout += "PLAYER : " + std::string(((currplayer == C_WHITE) ? "WHITE " : "BLACK "));

		for (auto c : curr) {
			Color tmp = YELLOW;
			tmp.a = 150;
			DrawCell::draw(c.row, c.col, tmp);
		}

		for (auto& c : moves) {
			Color tmp = SKYBLUE;
			tmp.a = 170;
			DrawCell::draw(c.row, c.col, tmp);
		}
		if (isCheck) {
			sout += "CHECK ";
		}
		DrawText(sout.c_str(), contBorder + border, size + contBorder + border, cellSize/3, RED);
		DrawText(movemessage.c_str(), contBorder + border, size + contBorder + border + cellSize /3, cellSize/3, RED);


		EndDrawing();
	}
	delete board;
	return 0;
}
 