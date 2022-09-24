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

	bool blackCheck = false;
	bool whiteCheck = false;

	Cell& at(int row, int col) {

		if (row > 7 || row < 0 || col>7 || col < 0)
			throw "FUCKING OUT OF RANGE";

		return array::at((8 - row - 1) * 8 + col);
	}
	Cell& at(Pos ppos) {
		return at(ppos.row, ppos.col);
	}

	int getcolcount(Content color) {
		int c = 0;
		for (int col = 0; col < 8; ++col) {
			for (int row = 0; row < 8; ++row) {
				Pos p;
				p.row = row;
				p.col = col;
				if (at(p).color == color)
					++c;

			}
		}
		return c;

	}

	//Previously set as a std::set, probably for particular order of the pieces 
	std::vector<Pos> getCells(Content color) {

		std::vector<Pos> cells;


		for (int col = 0; col < 8; ++col) {
			for (int row = 0; row < 8; ++row) {
				Pos p;
				p.row = row;
				p.col = col;
				if (at(p).color == color)
					cells.push_back(p);

			}
		}
		return cells;
	}

	std::vector<Pos> getmoves(int row, int col, Piece forcedType = NONE) {
		Pos p;
		p.row = row;
		p.col = col;
		return getmoves(p, forcedType);
	}

	std::vector<Pos> getmoves(Pos ppos, Piece forcedType = NONE) {

		auto piece = at(ppos);

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


			if (forcedType != NONE)
				piece.type = forcedType;

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

	//supposed to perform half of the move , ie checks if move is legal or not
	bool trymove(Pos from, Pos to) {

		Board tmpboard(*this);
		try {
			tmpboard.move(from, to);
		}
		catch (...) {
			return false;
		}

		if ((at(from).color == C_WHITE) && (tmpboard.whiteCheck)) {
			return false;
		}
		if ((at(from).color == C_BLACK) && (tmpboard.blackCheck)) {
			return false;
		}

		return true;

	}

	//throws exception if move fails, returns true if eaten or smthing, I have not decided yet
	bool move(Pos from, Pos to) {

		Cell& src = at(from.row, from.col);
		Cell& des = at(to.row, to.col);

		auto srcCol = src.color;
		if (srcCol == C_BLANK)
			throw "Cannot move when there's no piece ";

		auto moves = getmoves(from);
		for (auto& x : moves) {
			if (((x.row) == (to.row)) && ((x.col) == (to.col))) {

				auto revCol = ((srcCol == C_BLACK) ? C_WHITE : C_BLACK);

				des = src;
				src.type = NONE;
				src.color = C_BLANK;


				//Find the kings
				auto whites = getCells(C_WHITE);
				auto blacks = getCells(C_BLACK);

				Pos blackKing; blackKing.row = -1; blackKing.col = -1;
				Pos whiteKing; whiteKing.row = -1; whiteKing.col = -1;

				for (auto& x : whites) {
					if (at(x).type == KING) {
						whiteKing = x;
						break;
					}
				}
				for (auto& x : blacks) {
					if (at(x).type == KING) {
						blackKing = x;
						break;
					}
				}


				blackCheck = false;
				whiteCheck = false;
				//Update the check to king status
				for (int i = PAWN; i < NONE; ++i) {

					if (!whiteCheck) {
						auto whiteEater = getmoves(whiteKing, static_cast<Piece>(i));
						for (auto& x : whiteEater) {
							if (at(x).type == i) {
								whiteCheck = true;
								break;
							}
						}
					}

					if (!blackCheck) {
						auto blackEater = getmoves(blackKing, static_cast<Piece>(i));
						for (auto& x : blackEater) {
							if (at(x).type == i) {
								blackCheck = true;
								break;
							}
						}
					}



				}

				return true;
			}
		}
		throw "No can do this move, not possible at all.";
	}

};

//Mechanism to read board condition from string
//Uppercase is black and lowercase is white
//Obviously the letters are all from english except knight is h

const char defaultBoardCond[] = "\
RHBQKBHR\
PPPPPPPP\
--------\
--------\
--------\
--------\
pppppppp\
rhbqkbhr\
";
const char oneShotCheck[] = "\
RHBQKBHR\
PP-PPq-P\
-----PP-\
--P-----\
--------\
--p-p---\
pp-p-ppp\
rhb-kbhr\
";

bool fillBoard(Board& board, const char* filler) {

	std::string boardval(filler);
	if (boardval.size() < 64)
		return false;

	//since inside "Board" array class data are stored just in order as should be in string,
	//ie black side first row by row , storing of data is just identically sequential

	for (int n = 0; n < 8 * 8; ++n) {
		if (boardval[n] >= 'a' && boardval[n] <= 'z') {
			boardval[n] -= 'a' - 'A';
			board[n].color = C_WHITE;
		}
		else if (boardval[n] >= 'A' && boardval[n] <= 'Z')
			board[n].color = C_BLACK;
		else if (boardval[n] == '-') {
			board[n].type = NONE;
			board[n].color = C_BLANK;
			continue;
		}
		else
			return false;

		switch (boardval[n]) {
		case 'P':
			board[n].type = PAWN;
			break;
		case 'R':
			board[n].type = ROOK;
			break;
		case 'H':
			board[n].type = KNIGHT;
			break;
		case 'B':
			board[n].type = BISHOP;
			break;
		case 'Q':
			board[n].type = QUEEN;
			break;
		case 'K':
			board[n].type = KING;
			break;
		default:
			return false;
		}

	}

	return true;
}




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

	fillBoard(*board, defaultBoardCond);
	fillBoard(*board, oneShotCheck);
	Pos select;
	Pos move;
	select.row = -1;
	select.col = -1;
	move.row = -1;
	move.col = -1;

	Content currplayer = C_WHITE;

	std::string movemessage = "";
	std::vector<Pos> moves;

	while (!WindowShouldClose()) {
		std::string sout = "";

		moves.clear();
		auto curr = board->getCells(currplayer);
		auto opps = board->getCells((currplayer == C_WHITE) ? C_BLACK : C_WHITE);
		if (currplayer == C_BLACK && isAI) {
			//Gets number of pieces still on board
			size_t size = curr.size();
			
			//Gets and stores all possible moves in a vector of vectors
			//I know, a dumb style, but didn't want to do much work really
			//must find a better storage friendly and cpu time friendly method soon
			std::vector<std::vector<Pos>> currmoves;
			for (auto& x : curr)
				currmoves.push_back(board->getmoves(x));



			while (currmoves.size() != 0) {
				//Gets a random choice among available pieces
				size_t choice = GetRandomValue(0, currmoves.size() - 1);

				auto& tmp = currmoves.at(choice);

				if (tmp.size() == 0) {
					currmoves.erase(currmoves.begin() + choice);
					continue;
				}

				size_t n_move = GetRandomValue(0, tmp.size() - 1);
				if (board->trymove(curr.at(choice), currmoves.at(choice).at(n_move))) {
					select = curr.at(choice);
					move = currmoves.at(choice).at(n_move);
					break;

				}
				else {
					tmp.erase(tmp.begin() + n_move);

				}

			}

			////Position of enemy??
			//size_t enemypos = 0;

			////set of all places the enemy has eyes on
			//std::set<Pos> allEnemies;

			////Part that fills the set where all possible enemy moves are stored
			//for (auto m : opps) {
			//	for (auto y : board->getmoves(m)) {
			//		allEnemies.insert(y);
			//	}
			//}
			////Erases those positions from moves set that are already in enemy target
			////Try erasing only if all are not covered
			//if(aiFails > curr.size())
			//	for (auto m : allEnemies) {
			//		moves.erase(m);
			//	}

			////select one of the choices from enemy moves
			//select = *x;
			//if (moves.size() > 0) {
			//	x = moves.begin();
			//	choice = GetRandomValue(0, moves.size() - 1);
			//	for (int i = 0; i < choice; ++i)
			//		++x;

			//	move = *x;
			//}
			//else {
			//	++aiFails;
			//}


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
			auto tmpmoves = board->getmoves(select);
			moves.clear();
			for (auto x:tmpmoves) {
				if (board->trymove(select, x)) {
					moves.push_back(x);
				}
			}
			if (board->trymove(select, move)) {
				board->move(select, move);

				currplayer = (currplayer == C_WHITE) ? C_BLACK : C_WHITE;
				select.row = -1;
				select.col = -1;
				move.row = -1;
				move.col = -1;
			}
			else {

			}
		}
		catch (...) {
			select.row = -1;
			select.col = -1;
			move.row = -1;
			move.col = -1;

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
		if (((currplayer == C_WHITE) ? board->whiteCheck : board->blackCheck)) {
			sout += "CHECK ";
		}
		DrawText(sout.c_str(), contBorder + border, size + contBorder + border, cellSize/3, RED);
		DrawText(movemessage.c_str(), contBorder + border, size + contBorder + border + cellSize /3, cellSize/3, RED);


		EndDrawing();
	}
	delete board;
	return 0;
}
 