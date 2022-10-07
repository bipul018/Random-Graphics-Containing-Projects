#include <raylib-cpp.hpp>
#include <array>

int baghchal() {
	int width = 900;
	int height = 900;

	InitWindow(width, height, "Tiger Move");
	SetTargetFPS(90);


	Rectangle gamerec;
	gamerec.x = 0;
	gamerec.y = 0;
	gamerec.width = width;
	gamerec.height = height;

	//coordinate to vertex number fxn
	//let's number it this way for now :
	//	0		1		2		3		4
	// 		\		/		\		/
	//	5		6		7		8		9
	//		/		\		/		\
	//	10		11		12		13		14
	//		\		/		\		/
	//	15		16		17		18		19
	//		/		\		/		\
	//	20		21		22		23		24


	//convert boxno to row column , takes in refrence arguments
	auto boxtorowcol = [](int boxn, int& row, int& col) {

		if (boxn >= 0 && boxn < 25) {
			col = boxn % 5;
			row = boxn / 5;
		}

	};

	//Returns -1 if either outside the designated areas or near borders between 
	auto coortobox = [&gamerec](float x, float y) {
		if (x < gamerec.x || y < gamerec.y || x >= (gamerec.x + gamerec.width) || y >= (gamerec.y + gamerec.height))
			return -1;

		int row = 5 * (y - gamerec.y) / gamerec.width;
		int col = 5 * (x - gamerec.x) / gamerec.height;

		return row * 5 + col;

	};

	//boxno to rectangle
	auto boxtorec = [&gamerec](int boxno) {
		Rectangle rec = { 0 };
		if (boxno < 0 || boxno >= 25)
			return rec;
		rec = gamerec;
		rec.width /= 5;
		rec.height /= 5;

		rec.x += (boxno % 5) * rec.width;
		rec.y += (boxno / 5) * rec.height;

		return rec;

	};

	//rectangle to downsized rectangle for drawing purposes
	auto downrec = [](Rectangle rin, float fac = (1.0/2)) {
		rin.x += rin.width * fac *0.5;
		rin.y += rin.height * fac*0.5;

		rin.width *= fac;
		rin.height *= fac;
		return rin;

	};


	//array of chars to denote the pieces as following enum
	enum Piece {
		NONE,
		GOAT,
		TIGER
	};

	//game informations 
	using Board = std::array<char, 25>;

	Board board;
	unsigned ngoats = 0;
	unsigned neaten = 0;

	//additional game informations
	//Variable to store selected box
	int selectboxn = -1;
	//Some bool flags that may mo may not be used
	bool isselect = false;
	bool istiger = false;			//Denotes turn of tiger or goat
	bool istigerai = false;			//Denotes if tiger is to be played by an ai
	bool isgoatai = false;			//Denotes if goat is to be played by an ai
	bool isgameover = false;		//Will be true once game is over
	bool isgamestart = true;		//To denote if game has startes


	//Decided to hard code all transitions
	//16 possible transitions where path 
	std::array<std::vector<int>, 16> transitions;

	//fills rows and columns and diagonals
	{
		std::vector<int> bigdiag1;
		std::vector<int> bigdiag2;
		for (int i = 0; i < 5; ++i) {
			std::vector<int> va, vb;
			for (int j = 0; j < 5; ++j) {
				//fills each row elements
				va.push_back(j * 5 + i);
				//fills each col elements
				vb.push_back(i * 5 + j);
			}
			transitions.at(i) = va;
			transitions.at(5 + i) = vb;

			//fills main big diagonal
			bigdiag1.push_back(i * 6);
			//fills secondary big diagonal
			bigdiag2.push_back((i + 1) * 4);
		}
		transitions.at(10) = bigdiag1;
		transitions.at(11) = bigdiag2;
	}
	//fills small diagonals
	transitions.at(12).clear();
	transitions.at(13).clear();
	transitions.at(14).clear();
	transitions.at(15).clear();
	for (int i = 0; i < 3; ++i) {
		//for 2,6,10
		transitions.at(12).push_back(4 * i + 2);
		//for 2,8,14
		transitions.at(13).push_back(2 + 6 * i);
		//for 22,16,10
		transitions.at(14).push_back(22 - 6 * i);
		//for 22,18,14
		transitions.at(15).push_back(22 - 4 * i);
	}
	
	//validates the move and moves the thing if possible so takes in board information
	//Unfortunately this wont validate the trying to put goat in 
	auto trymove = [&transitions](int from, int to, Board& board,unsigned& ngoats, unsigned &neaten) {

		//returns false if trying to move to/from out of board or trying to move to same location
		if (from < 0 || to < 0 || from >= 25 || to >= 25 )
			return false;

		//determine if from is empty or not
		if (board.at(from) == NONE)
			return false;

		//if from has goat but goat is yet to be put
		if ((board.at(from) == GOAT) && ngoats < 20)
			return false;


		//find the transition group containing both from and to
		for (auto& tran : transitions) {

			//records a valid position if from and to are found else -1
			int fromloc = -1;
			int	toloc = -1;

			//searches whether to and from are in tran
			for (int i = 0; i < tran.size();++i) {
				if (tran.at(i) == from)
					fromloc = i;
				if (tran.at(i) == to)
					toloc = i;
			}

			//if both are in tran decide whether move is possible
			if (fromloc >= 0 && toloc >= 0) {

				//if consecutive and to is empty , transition is possible
				if ((board.at(to) == NONE) && (abs(fromloc - toloc) == 1)) {
					board.at(to) = board.at(from);
					board.at(from) = NONE;
					return true;
				}

				//else if from is tiger, from and to are 2 units apart and there is a goat in between
				//since each elements in each transitions are in arithmetic progression , from and to average works!!!
				if ((board.at(from) == TIGER) && (abs(fromloc - toloc) == 2) && (board.at((from + to) / 2) == GOAT)) {
					board.at(to) = TIGER;
					board.at((from + to) / 2) = NONE;
					board.at(from) = NONE;
					neaten++;
					return true;
				}

				//if none of above work then not possible
				return false;
			}

		}
		return false;

	};

	//Yet another function just for putting goats only
	auto putgoat = [](int to, Board& board, unsigned& ngoats) {
		if (to < 0 || to >= 25)
			return false;
		if (ngoats >= 20)
			return false;
		if (board.at(to) != NONE)
			return false;
		board.at(to) = GOAT;
		ngoats++;
	};

	//string to initialize game
	const char defaultboard[] =
		"\
t---t\
-----\
-----\
-----\
t---t";

	//board filler helper function
	auto fillboard = [&board](std::string input) {
		for (int i = 0; i < 25; ++i) {
			switch (input[i]) {
			case 't':
				board[i] = TIGER;
				break;
			case 'g':
				board[i] = GOAT;
				break;
			case '-':
				board[i] = NONE;
				break;
			default:
				return;
			}
		}
	};

	fillboard(defaultboard);

	//load tiger and goat images
	Image tiger= LoadImage("tiger.png");
	Image goat = LoadImage("goat.png");

	//Sample box
	Rectangle samplerec = downrec(boxtorec(0));

	//resizing image to sample rectangle
	ImageResize(&tiger, samplerec.width, samplerec.height);
	ImageResize(&goat, samplerec.width, samplerec.height);

	//Image textures size 
	Texture2D tigertex = LoadTextureFromImage(tiger);
	Texture2D goattex = LoadTextureFromImage(goat);

	

	while (!WindowShouldClose()) {

		//Filter mouse click event is game is not over
		if (!isgameover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			//store mouse clicked position 
			Vector2 mpos = GetMousePosition();

			//compare mouse positon against game rectangle
			if (CheckCollisionPointRec(mpos, gamerec)) {

				int boxn = coortobox(mpos.x, mpos.y);

				//If a box is indeed chosen
				if (boxn >= 0) {

					//If not selected already check if valid box is chosen and select or if goat to put , put
					if (!isselect) {

						//if goat can be put and is goat's turn then do that
						if (!istiger && putgoat(boxn,board,ngoats)) {
							istiger = !istiger;
						}
						//If selected on tiger on tiger turn and ... select
						else if(((board.at(boxn) == GOAT) != istiger)&&((board.at(boxn) == TIGER ) == istiger)) {
							selectboxn = boxn;
							isselect = true;
						}

					}
					else {
						//If can move , then move else reset selection
						if (trymove(selectboxn, boxn, board, ngoats, neaten)) {
							isselect = false;
							istiger = !istiger;
						}
						else  {
							isselect = false;
						}
					}
				}


			}

		}


		ClearBackground(BLACK);
		BeginDrawing();

		//first box
		Rectangle firstrec = boxtorec(0);
		//last box
		Rectangle lastrec = boxtorec(24);
		//mid box
		Rectangle midrec = boxtorec(12);

		//Converting all rectangles to their centers for below work
		firstrec.x += firstrec.width * 0.5;
		firstrec.y += firstrec.height * 0.5;
		
		lastrec.x += lastrec.width * 0.5;
		lastrec.y += lastrec.height * 0.5;
		
		midrec.x += midrec.width * 0.5;
		midrec.y += midrec.height * 0.5;

		//Draws each row and column lines
		for (int i = 0; i < 5; ++i) {

			DrawLine(firstrec.x, firstrec.y + i * firstrec.height, lastrec.x, firstrec.y + i * firstrec.height, WHITE);
			DrawLine(firstrec.x + i * firstrec.width, firstrec.y, firstrec.x + i * firstrec.width, lastrec.y, WHITE);

		}

		//Draw main two diagonals
		DrawLine(firstrec.x, firstrec.y, lastrec.x, lastrec.y, WHITE);
		DrawLine(lastrec.x, firstrec.y, firstrec.x, lastrec.y, WHITE);

		//Draw shorter diagonals
		DrawLine(midrec.x, firstrec.y, firstrec.x, midrec.y, WHITE);
		DrawLine(midrec.x, firstrec.y, lastrec.y, midrec.y, WHITE);
		DrawLine(midrec.x, lastrec.y, firstrec.x, midrec.y, WHITE);
		DrawLine(midrec.x, lastrec.y, lastrec.x, midrec.y, WHITE);


		for (int i = 0; i < 25; ++i) {

			Rectangle drawrec = downrec(boxtorec(i));

			//Prints row,col instead of actual pieces
			//int row;
			//int col;
			//boxtorowcol(i, row, col);

			//std::string str = std::to_string(row) + "," + std::to_string(col);

			//DrawText(str.c_str(), drawrec.x, drawrec.y, drawrec.width / str.length(), YELLOW);

			//continue;

			if (board[i] == TIGER) {
				DrawTexture(tigertex, drawrec.x, drawrec.y, WHITE);
			}
			if (board[i] == GOAT) {
				DrawTexture(goattex, drawrec.x, drawrec.y, WHITE);

			}

		}

		//draw selected cell if selected
		if (isselect) {
			Rectangle rec = boxtorec(selectboxn);
			Color c = YELLOW;
			c.a = 100;
			float radius;
			if (rec.width < rec.height)
				radius = rec.width / 2;
			else
				radius = rec.height / 2;
			DrawCircle(rec.x + rec.width / 2, rec.y + rec.height / 2, radius, c);

		}
		EndDrawing();

	}

	return 0;
}