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
	char board[25];
	unsigned ngoats = 0;
	unsigned neaten = 0;
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
				vb.push_back(i + j * 5);
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

			int row;
			int col;
			boxtorowcol(i, row, col);

			std::string str = std::to_string(row) + "," + std::to_string(col);

			DrawText(str.c_str(), drawrec.x, drawrec.y, drawrec.width / str.length(), YELLOW);

			continue;
			if (board[i] == TIGER) {
				DrawTexture(tigertex, drawrec.x, drawrec.y, WHITE);
			}
			if (board[i] == GOAT) {
				DrawTexture(goattex, drawrec.x, drawrec.y, WHITE);

			}

		}

		EndDrawing();

	}

	return 0;
}