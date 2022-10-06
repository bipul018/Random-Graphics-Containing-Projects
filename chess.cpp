#include <raylib-cpp.hpp>
#include <iostream>
#include <array>
#include <vector>
#include <utility>
#include <functional>
#include <algorithm>
#include <chrono>
#include <thread>

//Debug counters
//todo:: delete on finalizaion
unsigned checkkingcount = 0;
unsigned getmovescount = 0;
unsigned checkkinggetmoves = 0;
unsigned projmakemoves = 0;

enum Piece {
	PAWN,
	BISHOP,
	KNIGHT,
	ROOK,
	QUEEN,
	KING,
	NONE
};

const char* names[] = {
	"P","B","H","R","Q","K",""
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
	Pos(int r, int c) :row(r), col(c) {}

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
	
	//trying to optimize by having a already ready getmoves vector for using
	std::vector<Pos> tmpuse;

	//for optimization again 


	Cell* at(int row, int col) {

		if (row > 7 || row < 0 || col>7 || col < 0)
			return nullptr;

		return &array::at((8 - row - 1) * 8 + col);
	}
	Cell* at(Pos ppos) {
		return at(ppos.row, ppos.col);
	}

	int getcolcount(Content color) {
		int c = 0;
		for (int col = 0; col < 8; ++col) {
			for (int row = 0; row < 8; ++row) {
				Pos p;
				p.row = row;
				p.col = col;
				if (at(p)->color == color)
					++c;

			}
		}
		return c;

	}

	//calculates a wt favourale for white 
	//if a colour is provided then count only that colour's wt. always +ve
	float getwt(Content type = C_BLANK) {
		//for now just assign the wt as per they are arranged in enum + 1

		float netwt = 0.0;
		for (auto& c : (std::array<Cell, 8 * 8>)(*this)) {
			if ((c.color == C_WHITE)&&(type!=C_BLACK))
				netwt += c.type + 1;
			if ((c.color == C_BLACK)&&(type!=C_WHITE))
				netwt -= c.type + 1;

		}
		if (type == C_BLACK)
			netwt = -netwt;
		return netwt;

	}
	//Previously set as a std::set, probably for particular order of the pieces 
	std::vector<Pos> getCells(Content color) {

		std::vector<Pos> cells;
		cells.reserve(16);

		for (int col = 0; col < 8; ++col) {
			for (int row = 0; row < 8; ++row) {
				Pos p;
				p.row = row;
				p.col = col;
				//Debug stuff , if doesn't improve remove
				auto ptr = at(p);
				if (ptr->color == color)
					cells.push_back(p);

			}
		}
		return cells;
	}

	//Trying to change stuff , may need later if doesnot work out
	//std::vector<Pos> getmoves(int row, int col, Piece forcedType = NONE) {
	//	Pos p;
	//	p.row = row;
	//	p.col = col;
	//	return getmoves(p, forcedType);
	//}
	
	//also send a pos vector for performance issues
	std::vector<Pos>& getmoves(std::vector<Pos> & moves,Pos ppos, Piece forcedType = NONE) {
	
		//debug variables
		getmovescount++;

		Cell *pieceptr = at(ppos);
		//std::vector<Pos> moves;
		//moves.reserve(32);	//reserve maximum moves of queen worst case, serious speed boost
		if (pieceptr == nullptr)
			return moves;
		Cell piece = *pieceptr;
		moves.clear();
		if (piece.color != C_BLANK) {

			auto compCol = (piece.color == C_WHITE) ? C_BLACK : C_WHITE;

			auto fillpawn = [&,this]() {
				Pos p;
				p.col = ppos.col;
				auto frontrow = ppos.row + ((piece.color == C_WHITE) ? 1 : -1);
				p.row = frontrow;
				Cell* ptr = nullptr;
				{
					ptr = at(frontrow, ppos.col);
					if (ptr!=nullptr && ptr->color == C_BLANK) {
						moves.push_back(p);
						{
							if (piece.color == C_WHITE && ppos.row == 1) {
								ptr = at(3, ppos.col);
								if (ptr!=nullptr && ptr->color == C_BLANK) {
									p.row = 3;
									p.col = ppos.col;
									moves.push_back(p);
								}
							}
							else if (piece.color == C_BLACK && ppos.row == 6) {
								ptr = at(4, ppos.col);
								if (ptr!=nullptr&&ptr->color == C_BLANK) {
									p.row = 4;
									p.col = ppos.col;
									moves.push_back(p);
								}
							}
						}
					}
				}
				//TODO: gonna have to check for program control along with converting all at() fns to pointer version
					ptr = at(frontrow, ppos.col + 1);
					if (ptr!=nullptr && ptr->color == compCol) {
						p.col = ppos.col + 1;
						p.row = frontrow;
						moves.push_back(p);
					}
					ptr = at(frontrow, ppos.col - 1);
					if (ptr != nullptr && ptr->color == compCol) {
						p.col = ppos.col - 1;
						p.row = frontrow;
						moves.push_back(p);
					}


			};

			//pushes a move if the place is not occupied by friend , returns true it it was empty
			auto tmpfunc = [&,this](Pos p)->bool {
				Cell* ptr = at(p.row, p.col);
					if (ptr!=nullptr && ptr->color == C_BLANK)
						moves.push_back(p);
					else {
						if (ptr != nullptr && ptr->color == compCol)
							moves.push_back(p);
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

	bool updateCheckStat(Content color) {
		if (color == C_BLANK)
			return false;

		//debug stuff
		checkkingcount++;
		unsigned getmovecounttmp = getmovescount;
		checkkinggetmoves = 0;

		//Find the kings
		auto cells = getCells(color);

		Pos King; King.row = -1; King.col = -1;

		for (auto& x : cells) {
			if (at(x)->type == KING) {
				King = x;
				break;
			}
		}

		bool& check = ((color == C_WHITE) ? whiteCheck : blackCheck);
		check = false;
		//Update the check to king status
		for (int i = PAWN; i < NONE; ++i) {

			if (!check) {
				auto &whiteEater = getmoves(tmpuse,King, static_cast<Piece>(i));
				for (auto& x : whiteEater) {
					if (at(x)->type == i) {
						check = true;
						break;
					}
				}
			}

		}

		//debug stuff
		checkkinggetmoves = getmovescount - getmovecounttmp;
		getmovescount = getmovecounttmp;

	}

	//supposed to perform half of the move , ie checks if move is legal or not
	bool trymove(Pos from, Pos to) {

		Board tmpboard(*this);


		try {
			if (!tmpboard.move(from, to))
				return false;
		}
		catch (...) {
			return false;
		}

		//if previous block returns true following statements are guarenteed to work
		if ((at(from)->color == C_WHITE) && (tmpboard.whiteCheck)) {
			return false;
		}
		if ((at(from)->color == C_BLACK) && (tmpboard.blackCheck)) {
			return false;
		}

		return true;

	}

	//first tries to move , if fail returns false , else moves and returns true
	bool tryNmove(Pos from, Pos to) {
		//Going to have to check if nullptr or not 

		if (at(from) == nullptr || at(to) == nullptr)
			return false;

		Cell fcell = *at(from);
		Cell tcell = *at(to);

		try {
			if (!move(from, to))
				return false;
		}
		catch (...) {
			return false;
		}

	
	
		if ( ((fcell.color == C_BLACK) && blackCheck) || ((fcell.color == C_WHITE) && (whiteCheck))) {
			*at(from) = fcell;
			*at(to) = tcell;
			return false;
		}
		return true;
	}

	//throws exception if move fails, return false also if move fails , prolly removing this exception throwing condn
	//this moves nonetheless if piece is there
	bool move(Pos from, Pos to) {
		
		Cell* srcptr = at(from.row, from.col);
		Cell* desptr = at(to.row, to.col);

		if (srcptr == nullptr || desptr == nullptr)
			return false;

		Cell& src = *srcptr;
		Cell& des = *desptr;
		auto srcCol = src.color;
		if (srcCol == C_BLANK)
			throw "Cannot move when there's no piece ";

		auto &moves = getmoves(tmpuse,from);
		for (auto& x : moves) {
			if (((x.row) == (to.row)) && ((x.col) == (to.col))) {

				auto revCol = ((srcCol == C_BLACK) ? C_WHITE : C_BLACK);

				des = src;
				src.type = NONE;
				src.color = C_BLANK;

				//update check status
				updateCheckStat(C_WHITE);
				updateCheckStat(C_BLACK);

				//debug variable
				projmakemoves++;

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
const char pracBoard[] = "\
---K-B-R\
-RP-P-P-\
-P---H--\
Pbp-pr-P\
p------p\
--h---p-\
-b--q--R\
r---k---\
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
		if (row > 7 || col > 7 || row < 0 || col < 0)
			throw"Out of range";
		DrawRectangle(col * cellSize, (8 - row - 1) * cellSize, cellSize, cellSize, box);
		DrawText(names[board->at(row, col)->type],
			col * cellSize + padding + border + contBorder,
			(8 - row - 1) * cellSize + padding + border + contBorder,
			contSize - contBorder * 2, letter);
	}
	void operator () (int row, int col, Color box) {
		if (row > 7 || col > 7 || row < 0 || col < 0)
			throw"Out of range";
		(*this)(row, col, box, (board->at(row, col)->color == C_WHITE) ? WHITE : ((board->at(row, col)->color == C_BLACK) ? BLACK : BLANK));
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

//This is an object for evaluating using ai, for some reason lamda couldn't do it , so here it is
class AIeval {
public:
	Board& masterBoard;
	int depth = 0;
	bool isaiwhite = true;
	unsigned* selrow = nullptr;
	unsigned* selcol = nullptr;
	unsigned* movrow = nullptr;
	unsigned* movcol = nullptr;

	//a vector for temporary use , trying for speed boost if possible
	std::vector<Pos> tmpuse;
	AIeval(Board& board):masterBoard(board){
		tmpuse.reserve(32);
	}

	//An ai for selecting next moving row and column
	//Design for moving in a separate thread as much as possible

	//For now a minmax algorithm only, plss update this to a better weighted algorithm
	float evaluate() {
		totalevals++;
		auto cells = masterBoard.getCells((isaiwhite) ? C_WHITE : C_BLACK);

		std::vector<std::pair<float, std::pair<Pos, Pos>>> moves;

		Board tmp = masterBoard;
		tmp.tmpuse.reserve(32);
		for (auto c : cells) {
			auto &ms = masterBoard.getmoves(tmpuse,c);
			for (auto m : ms) {
				if (tmp.tryNmove(c, m)) {
					std::pair<Pos, Pos> p;
					p.first = c;
					p.second = m;
					std::pair<int, std::pair<Pos, Pos>> fp;
					if (depth > 0) {
						AIeval *aiunit = new AIeval(tmp);
						aiunit->depth = depth - 1;
						aiunit->isaiwhite = !isaiwhite;
						try {
							fp.first = -aiunit->evaluate();
						}
						catch (...) {
							//this is when there are no further moves , in this case there should be high weight to this one
							//so let's sum weights of the colors of this only and multiply by 2
							fp.first = tmp.getwt((tmp.at(c)->color)) * 2;
						}
						delete aiunit;
					}
					else {
						fp.first = tmp.getwt();

						if (!isaiwhite)
							fp.first = -fp.first;
					}
					fp.second = p;
					moves.push_back(fp);
					tmp = masterBoard;


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

			auto res = GetRandomValue(0, max - 1);
			if(selrow != nullptr && selcol != nullptr && movrow != nullptr && movcol != nullptr) {
				*selrow = moves.at(res).second.first.row;
				*selcol = moves.at(res).second.first.col;
				*movrow = moves.at(res).second.second.row;
				*movcol = moves.at(res).second.second.col;
			}
			return moves.at(0).first;
		}
		throw "NO MOVES LEFT";
	}
	static int totalevals;
};
int AIeval::totalevals = 0;

int chess() {

	Board masterBoard;
	DrawCell cellpaint(900);
	cellpaint.board = &masterBoard;

	fillBoard(masterBoard, defaultBoardCond);
	//fillBoard(masterBoard, pracBoard);
	
	//a vector for using with getmoves later on to avoid realllocating all the time
	std::vector<Pos> tmpusegetmoves;
	tmpusegetmoves.reserve(32);
	masterBoard.tmpuse.reserve(32);
	
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


	//Window stuff
	InitWindow(900, 1000, "Chess");
	SetTargetFPS(90);

	//timer stuff
	double prevMoveTime = GetTime();
	double prevTakenTime = 0;

	//Board area designation stuff
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

	bool stopAI = false;
	bool runAI = false;

	//todo:: somehow ai had the super power to convert king into queen(both enemy and own) ?? gonna have to deug that for sure
	auto aithingy = [&]() {
		//while (!stopAI) {
			using namespace std::chrono_literals;
			if (!runAI) {
				std::this_thread::sleep_for(2ms);
			//	continue;
			}

			if (isai && (isaiwhite == iswhite)) {

				try {
					AIeval mainai(masterBoard);
					mainai.depth = 4;
					mainai.isaiwhite = isaiwhite;
					mainai.selrow = &selrow;
					mainai.selcol = &selcol;
					mainai.movrow = &movrow;
					mainai.movcol = &movcol;
					mainai.evaluate();


					isselect = true;
					ismove = true;
				}
				catch (...) {
					int kk = 3;
				}
			}
			//runAI = false;
		//}
	};

	//ai thread
	//std::thread aiThrd(aithingy);
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
			//if ai thread not started then start it 
			aithingy();
			//if (aiThrd == nullptr)
			//	aiThrd = new std::thread(aithingy);
		}

		//Selection validation
		if (isselect) {

			//If not turn then deselect 

			if (masterBoard.at(selrow, selcol) != nullptr) {
				if ((iswhite && (masterBoard.at(selrow, selcol)->color != C_WHITE)) ||
					(!iswhite && (masterBoard.at(selrow, selcol)->color != C_BLACK))) {

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
							//reset things after move success
							prevTakenTime = GetTime() - prevMoveTime;
							prevMoveTime = GetTime();
						}
						else {

							//If not a possible move and it is dumb human's turn then make the move location the new select location
							//TODO : this is prolly not working , fix it
							if (!isai || (isaiwhite != iswhite)) {
								selcol = movcol;
								selrow = movrow;
							}
						}
						ismove = false;
					}
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

			auto& colors = masterBoard.getmoves(tmpusegetmoves, Pos(selrow, selcol));
			for (auto& to : colors) {
				Pos from;
				from.row = selrow;
				from.col = selcol;
				if (masterBoard.trymove(from, to))
					cellpaint(to.row, to.col, SKYBLUE);
			}

		}

		DrawText(std::to_string(prevTakenTime).c_str(), 0, chessrec.y + chessrec.height, 0.9 * (winrec.height - chessrec.height - chessrec.y), MAROON);

		EndDrawing();
	}

	return 0;
}