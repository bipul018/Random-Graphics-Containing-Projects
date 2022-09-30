#include <raylib-cpp.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <utility>
#include <functional>
#include <algorithm>

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

	//Provide interchangeability between Pos and Vector2
	operator Vector2() {
		Vector2 vec;
		vec.x = row;
		vec.y = col;
		return vec;
	}
	Pos(){}
	Pos(const Vector2& vec) {
		row = vec.x;
		col = vec.y;
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

	//TODO make a weight calculator 

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

	//Returns box (row, col) of chess box on screen based on position on screen
	static Pos pos2box(unsigned xpos, unsigned ypos) {

		Pos res;
		if (xpos > 8 || ypos > 8)
			throw "Invalid xpos ypos";
		res.row = 8 - ypos - 1;
		res.col = xpos;
		return res;
	}
	//Returns positoin on screen ( xpos, ypos) based on row col of box 
	static Vector2 box2pos(unsigned row, unsigned col) {

		Vector2 res = { 0 };
		if (row > 8 || col > 8)
			throw "Invalid row col";
		res.x = col;
		res.y = 8 - row - 1;
		return res;
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




class DrawCell {
public:

	DrawCell(int boardSize = 800,float xpos=0,float ypos=0) {
		posx = xpos;
		posy = ypos;
		board = nullptr;
		size = boardSize;
		cellSize = size / 8.0;
		padding = 0.1 * cellSize;
		border = padding / 2;
		contSize = cellSize - (padding + border) * 2;
		contBorder = contSize * 0.1;

	}

	void operator() (int row, int col, Color box, Color letter) {

		DrawRectangle(col * cellSize, (8 - row - 1) * cellSize, cellSize, cellSize, box);
		DrawText(names[board->at(row, col).type],
			col * cellSize + padding + border + contBorder,
			(8 - row - 1) * cellSize + padding + border + contBorder,
			contSize - contBorder * 2, letter);
	}
	void operator () (int row, int col, Color box) {
		(*this)(row, col, box, (board->at(row, col).color == C_WHITE) ? WHITE : ((board->at(row, col).color == C_BLACK) ? BLACK : BLANK));
	}
	void operator() (int row, int col) {
		(*this)(row, col, ((row + col) % 2) ? LIGHTGRAY : DARKGRAY);
	}
	void operator() () {
		for (int r = 0; r < 8; ++r)
			for (int c = 0; c < 8; ++c)
				(*this)(r, c);
	}

	Board* board;
	float posx;
	float posy;
	float size;
	float cellSize;
	float padding;
	float border;
	float contSize;
	float contBorder;



};


int chess() {

	Board masterBoard;
	DrawCell cellpaint(900);
	cellpaint.board = &masterBoard;

	fillBoard(masterBoard, defaultBoardCond);

	//Game flags , sorry for using plain old bools now
	bool isselect = false;
	bool ismove = false;
	bool iswhite = true;
	bool isai = true;
	bool isaiwhite = false;
	bool gameover = false;

	//Selection row and column
	unsigned selrow = -1;
	unsigned selcol = -1;
	unsigned movrow = -1;
	unsigned movcol = -1;


	InitWindow(900, 1000, "Chess");
	SetTargetFPS(90);

	Rectangle winrec;
	Rectangle chessrec;
	winrec.x = 0;
	winrec.y = 0;
	winrec.width = 900;
	winrec.height = 1000;
	chessrec.width = cellpaint.size;
	chessrec.height = cellpaint.size;
	chessrec.x = cellpaint.posx;
	chessrec.y = cellpaint.posy;
	

	//function alias only
	auto isInRec = CheckCollisionPointRec;
	

	//An ai for selecting next moving row and column
	//Design for moving in a separate thread as much as possible

	auto aithingy = [&]() {
		if (isai && (isaiwhite == iswhite)) {

			auto cells = masterBoard.getCells((iswhite) ? C_WHITE : C_BLACK);

			std::vector<std::pair<float,std::pair<Pos, Pos>>> moves;

			for (auto c : cells) {
				auto ms = masterBoard.getmoves(c);
				for (auto m : ms) {
					if (masterBoard.trymove(c, m)) {
						std::pair<Pos, Pos> p;
						p.first = c;
						p.second = m;
						std::pair<int, std::pair<Pos, Pos>> fp;
						Board tmp = masterBoard;
						tmp.move(c, m);
						fp.first = tmp.getcolcount((iswhite) ? C_WHITE : C_BLACK) - tmp.getcolcount((iswhite) ? C_BLACK : C_WHITE);
						fp.second = p;
						moves.push_back(fp);
					}
				}
			}

			std::sort(moves.begin(), moves.end(), [](std::pair<float, std::pair<Pos, Pos>>& a, std::pair<float, std::pair<Pos, Pos>>& b)->bool {
				return a.first > b.first;

				});

			if (moves.size() > 0) {

				//count highest only
				
				int max = 0;
				while ((max < moves.size()) && (moves.at(max).first == moves.at(0).first))
					++max;

				auto res = GetRandomValue(0, max-1);
				selrow = moves.at(res).second.first.row;
				selcol = moves.at(res).second.first.col;
				movrow = moves.at(res).second.second.row;
				movcol = moves.at(res).second.second.col;
				isselect = true;
				ismove = true;
			}

		}
	};

	while (!WindowShouldClose()) {

		//Clickity stuff works iff not ai and not ai's turn if it is
		if (!isai || (isai && (isaiwhite != iswhite))) {

			//Maybe in future when extra buttons have to be added some of clicky stuff should be outside

			//If user's turn then check if click has occured
			if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
				Vector2 mousepos = GetMousePosition();

				//Check if mouse is in rectangle
				if (isInRec(mousepos, winrec)) {

					//If mouse click is inside the chessboard area then do more stuff
					//Be wary , this section doesnot check the validity of moves, that is done in other sections
					if (isInRec(mousepos, chessrec)) {

						int x = (mousepos.x - chessrec.x) / (cellpaint.cellSize);
						int y = (mousepos.y - chessrec.y) / (cellpaint.cellSize);
						Pos rowcol = Board::pos2box(x, y);

						if (!isselect) {

							selcol = rowcol.col;
							selrow = rowcol.row;
							isselect = true;
						}
						else {

							//if already selected , unselect , else ready for move
							if ((selcol == rowcol.col) && (selrow == rowcol.row))
								isselect = false;
							else {
								ismove = true;
								movrow = rowcol.row;
								movcol = rowcol.col;
							}


						}


					}



				}
			}
		}
		//if ai's turn then do ai stuff
		else {
			aithingy();
		}

		//Selection validation
		if (isselect) {

			//If not turn then deselect 
			if ((iswhite && (masterBoard.at(selrow, selcol).color != C_WHITE)) ||
				(!iswhite && (masterBoard.at(selrow, selcol).color != C_BLACK))) {

				isselect = false;

			}
			else {


				//Now move validation and stuff
				if (ismove) {
					Pos from;
					from.row = selrow;
					from.col = selcol;

					Pos to;
					to.row = movrow;
					to.col = movcol;

					if (masterBoard.trymove(from, to)) {
						masterBoard.move(from, to);
						isselect = false;
						iswhite = !iswhite;
					}
					else {

						//If not a possible move and it is dumb human's turn then make the move location the new select location
						//TODO : this is prolly not working , fix it
						if (!isai || (isaiwhite == iswhite)) {
							selcol = movcol;
							selrow = movrow;
						}
					}
					ismove = false;
				}
			}
		}
		
		ClearBackground(GRAY);
		BeginDrawing();
		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < 8; ++j)
				cellpaint(i, j);

		if (isselect) {
			cellpaint(selrow, selcol, YELLOW);

			auto colors = masterBoard.getmoves(selrow, selcol);
			for (auto& to : colors) {
				Pos from;
				from.row = selrow;
				from.col = selcol;
				if (masterBoard.trymove(from, to))
					cellpaint(to.row, to.col, SKYBLUE);
			}

		}
		EndDrawing();
	}

	return 0;
}