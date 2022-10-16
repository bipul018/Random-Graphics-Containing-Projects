#include <raylib-cpp.hpp>
#include "MyGUI.h"
#include <array>
#include <functional>

int baghchal() {
	int width = 1220;
	int height = 900;

	InitWindow(width, height, "Tiger Move");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(90);

	//Rectangle for game
	Rectangle gamerec;
	//Remaining rectangle
	Rectangle guirec;

	auto setuprecs = [&]() {

		width = GetScreenWidth();
		height = GetScreenHeight();

		gamerec.x = 0;
		gamerec.y = 0;
		gamerec.width = ((width > height) ? height : width);
		gamerec.height = ((width > height) ? height : width);


		guirec.x = gamerec.x;
		guirec.y = gamerec.y;
		if (width > height) {
			guirec.x += gamerec.width;
			guirec.width = width - guirec.x;
			guirec.height = gamerec.height;
		}
		else {
			guirec.y += gamerec.width;
			guirec.height = height - guirec.y;
			guirec.width = gamerec.width;
		}
	};

	setuprecs();

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
	auto downrec = [](Rectangle rin, float fac = (1.0 / 2)) {
		rin.x += rin.width * fac * 0.5;
		rin.y += rin.height * fac * 0.5;

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

	enum GameState {
		PLAY,			//game is still on
		DRAW,			//game is draw
		T_WIN,			//tiger has won
		G_WIN			//goat has won
	};

	//game informations 
	using Board = std::array<char, 25>;

	//A struct that entirely describes a game
	struct GameUnit {
		Board board;
		unsigned ngoats = 0;
		unsigned neaten = 0;

		//game final state
		GameState state = PLAY;
	} main_game;

	//additional game informations
	//Variable to store selected box
	int selectboxn = -1;

	//Some bool flags that may mo may not be used
	bool isselect = false;
	bool istiger = false;			//Denotes turn of tiger or goat
	bool istigerai = true;			//Denotes if tiger is to be played by an ai
	bool isgoatai = true;			//Denotes if goat is to be played by an ai
	bool isgamestart = false;		//To denote if game has startes


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
	
	//Takes in board information and locations in transition sets to move from a cell to to a cell
	//also takes another refrence variable at last for if a goat is eaten
	//If from location and to location are equal it implies trying to put a goat
	//Only works if game is not over 
	//All invalid cases return false
	auto checkmove = [&transitions](const GameUnit &game, int tran_no, int fromloc, int toloc,int & empty) {

		if (game.state != PLAY)
			return false;
		
		if (tran_no < 0 || tran_no >= 16)
			return false;

		auto& tran = transitions.at(tran_no);

		if (fromloc >= tran.size() || toloc >= tran.size() || fromloc < 0 || toloc < 0)
			return false;

		int to = tran.at(toloc);
		int from = tran.at(fromloc);

		//case for when goats are yet to be put
		if (game.ngoats < 20) {
			//If goat is yet to be put and goat is encountered
			if (game.board.at(from) == GOAT)
				return false;

			//Else check for putting goat , this is really inefficient so as possible avoid this case
			if (from == to) {
				return (game.board.at(from) == NONE);
			}

		}

		//if to is empty , transition may be possible
		if (game.board.at(to) == NONE) {
			//If consecutive then just move
			if (abs(fromloc - toloc) == 1) {
				empty = -1;
				return true;
			}

			//else if from is tiger, from and to are 2 units apart and there is a goat in between
			//since each elements in each transitions are in arithmetic progression , from and to average works!!!
			if ((game.board.at(from) == TIGER) && (abs(fromloc - toloc) == 2) && (game.board.at((from + to) / 2) == GOAT)) {
				empty = (from + to) / 2;
				return true;
			}
		}
		//if none of above work then not possible
		return false;
	};
	
	//Tries to move and moves if possible , takes in board positions and finds thier transitions for movement
	//If from == to then goat is to be put
	auto trymove = [&transitions,&checkmove](int from, int to, GameUnit & game) {

		//If game is already over
		if (game.state != PLAY)
			return false;

		//returns false if trying to move to/from out of board 
		if (from < 0 || to < 0 || from >= 25 || to >= 25 )
			return false;

		//Case for if goat has yet to be put
		if (game.ngoats < 20) {

			//Trying to move goat when goat is left
			if (game.board.at(from) == GOAT)
				return false;

			//Trying to put goat
			if (from == to) {

				if (game.board.at(from) == NONE) {
					game.board.at(from) = GOAT;
					game.ngoats++;
					return true;
				}
				else
					return false;
			}
		}

		//find the transition group containing both from and to
		for (int tn = 0; tn < transitions.size();++tn) {
			auto& tran = transitions.at(tn);
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
				int eatpos = -1;
				if (checkmove(game, tn, fromloc, toloc, eatpos)) {
					game.board.at(to) = game.board.at(from);
					game.board.at(from) = NONE;
					if (eatpos >= 0) {
						game.board.at(eatpos) = NONE;
						game.neaten++;
					}
					return true;
				}
				else
					return false;
			}

		}
		return false;

	};

	//Proxy function for putting goats
	auto putgoat = [&trymove](int to, GameUnit& game) {
		return trymove(to, to, game);
	};

	//Returns the possible moves based on given game and whether it is goat or tiger we require
	auto getmoves = [&transitions, &checkmove](const GameUnit& game, bool isGoat) {

		//The vector of pairs of locations on board
		std::vector<std::pair<int, int>> moves;
		
		//If not playable
		if (game.state != PLAY)
			return moves;

		//If max no of goats are eaten
		if (game.neaten >= 5)
			return moves;

		//Pre reserve some amount of space for moves
		moves.reserve(16);

		//If goat is yet to be placed and is goat turn
		if (isGoat && (game.ngoats < 20)) {
			for (int i = 0; i < game.board.size(); ++i)
				if (game.board.at(i) == NONE)
					moves.push_back(std::pair<int, int>(i, i));
			return moves;
		}

		//Else go to each transition sets now checking for goat or tiger it is
		Piece curr = ((isGoat) ? GOAT : TIGER);

		//go through each set
		for (int tn = 0; tn < transitions.size(); ++tn) {
			auto& tran = transitions.at(tn);

			//For each element of set
			for (int from = 0; from < tran.size(); ++from) {

				//With each another element of that set
				for (int to = 0; to < tran.size(); ++to) {

					//If the from element has our desired piece
					if (game.board.at(tran.at(from)) == curr) {
						int dummy;
						//If the move is possible
						if (checkmove(game, tn, from, to, dummy)) {
							moves.push_back(std::pair<int, int>(tran.at(from), tran.at(to)));
						}
					}


				}
			}

		}

		return moves;

	};

	//Updates the game state , also takes in whether it's tiger's turn or not , returns the moves vector
	auto updateGame = [&getmoves](GameUnit& game, bool istiger) {

		

		auto moves = getmoves(game, !istiger);

		if (game.state != PLAY)
			return moves;

		if (game.neaten >= 5) {
			game.state = T_WIN;
			return moves;
		}

		if (istiger && moves.empty()) {
			game.state = G_WIN;
			return moves;
		}

		if (!istiger && moves.empty()) {
			game.state = DRAW;
			return moves;
		}
		return moves;


	};

	//string to initialize game
	const char defaultboard[] =
		"\
t---t\
-----\
-----\
-----\
t---t";
	const char trialboard[] =
		"\
ggggg\
ggggg\
ggggg\
gt-tg\
gtggt";

	//board filler helper function , maybe in future initializes game too
	auto fillboard = [](std::string input,GameUnit &game) {
		game.ngoats = 0;
		game.neaten = 0;
		game.state = PLAY;
		for (int i = 0; i < 25; ++i) {
			switch (input[i]) {
			case 't':
				game.board[i] = TIGER;
				break;
			case 'g':
				game.board[i] = GOAT;
				game.ngoats++;
				break;
			case '-':
				game.board[i] = NONE;
				break;
			default:
				return;
			}
		}
	};

	fillboard(defaultboard,main_game);
	//fillboard(trialboard,main_game);

		//All required GUI elements are to be introduced here
	//Introduction panel, ie startup panel
	Panel intropan;
	intropan.box = guirec;

	//Human or ai selection
	Label ply1;
	ply1.msg = "Player 1";

	DropBox play1by;
	play1by.list.push_back("Human");
	play1by.list.push_back("Computer");

	Panel plygrp1;
	plygrp1.units.push_back(&ply1);
	plygrp1.units.push_back(&play1by);

	Label ply2;
	ply2.msg = "Player 2";

	DropBox play2by = play1by;

	Panel plygrp2;
	plygrp2.units.push_back(&ply2);
	plygrp2.units.push_back(&play2by);

	GroupBox settgrp;
	settgrp.title = "Setup";
	settgrp.units.push_back(&plygrp1);
	settgrp.units.push_back(&plygrp2);


	Button playbut;
	playbut.msg = "PLAY";

	//Couple empty labels for accomodating drop boxes
	Label empt1, empt2;
	empt1.msg = "\t";
	empt2 = empt1;

	Button exitbut;
	exitbut.msg = "EXIT";


	intropan.units.push_back(&playbut);
	intropan.units.push_back(&settgrp);
	intropan.units.push_back(&empt1);
	intropan.units.push_back(&empt2);
	intropan.units.push_back(&exitbut);

	auto resetStartUI = [&]() {

		intropan.box = guirec;

		bool hori = (width > height);
		hori = true;

		playbut.packToUnits();

		ply1.packToUnits();
		ply2.packToUnits();
		if (ply1.box.width < intropan.box.width/2)
			ply1.box.width = intropan.box.width/2;


		if (ply2.box.width < intropan.box.width/2)
			ply2.box.width = intropan.box.width/2;




		play1by.justchanged = true;
		play2by.justchanged = true;
		play1by.packToUnits();
		play2by.packToUnits();

		plygrp1.stackChildren(hori);
		plygrp2.stackChildren(hori);

		plygrp1.packToUnits();
		plygrp2.packToUnits();

		empt1.packToUnits();
		empt2.packToUnits();
		exitbut.packToUnits();

		settgrp.stackChildren((!hori));
		settgrp.packToUnits();

		intropan.stackChildren(hori);

	};

	//Stuff shown during game ,ie stats
	Panel gamepanel;
	gamepanel.box = guirec;

	LabelPair gputinfo;
	gputinfo.msg1 = "Goats Put ";
	gputinfo.msg2 = "20";

	LabelPair geatinfo;
	geatinfo.msg1 = "Goates Eaten ";
	geatinfo.msg2 = "0";

	LabelPair statinfo;
	statinfo.msg1 = "Status : ";
	statinfo.msg2 = "Game Running";

	Button resetbut;
	resetbut.msg = "Restart Game";

	gamepanel.units.push_back(&statinfo);
	gamepanel.units.push_back(&gputinfo);
	gamepanel.units.push_back(&geatinfo);
	gamepanel.units.push_back(&resetbut);

	auto resetInfoUI = [&]() {
		gamepanel.box = guirec;
		bool hori = (width > height);
		hori = true;

		std::string tmp = statinfo.msg2;
		statinfo.msg2 = "Restart Game";
		statinfo.packToUnits();
		statinfo.msg2 = tmp;

		tmp = gputinfo.msg2;
		gputinfo.msg2 = "MM";
		gputinfo.packToUnits();
		gputinfo.msg2 = tmp;

		tmp = geatinfo.msg2;
		geatinfo.msg2 = "MM";
		geatinfo.packToUnits();
		geatinfo.msg2 = tmp;

		resetbut.packToUnits();

		if (statinfo.box.width < gamepanel.box.width)
			statinfo.box.width = gamepanel.box.width;

		gamepanel.stackChildren(hori);

	};

	auto updateInfoUI = [&]() {

		gputinfo.msg2 = std::to_string(main_game.ngoats);
		geatinfo.msg2 = std::to_string(main_game.neaten);
		switch (main_game.state) {
		case PLAY:
			statinfo.msg2 = "Game Running";
			break;
		case DRAW:
			statinfo.msg2 = "Game Draw";
			break;
		case T_WIN:
			statinfo.msg2 = "Tigers Win";
			break;
		case G_WIN:
			statinfo.msg2 = "Goats Win";
			break;
		}

	};

	//Do all intial resizing stuff of gui
	resetStartUI();
	resetInfoUI();

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

	//Now begins the AI part, completely experimental

	//Let's try neural networks evolution stuff
	

	//AI versions 1 lets say
	//simple min max algorithm , basis for weight is the number of goats not eaten yet
	struct AI1 {
		//The base game ai will evaluate upon
		GameUnit base;
		//Specifies the role of ai
		bool isaitiger;
		//Specifies whether goat or tiger is to be optimized,i.e ai should lose or win
		bool isoptgoat;
		//The level of evaluation the AI has to perform
		int level;
		//A vector of moves for use by ai1 
		std::vector<std::pair<int, int>> moves;
		//A move to order the current ai to perform first if invalid move then just perform without doing anything
		std::pair<int, int> move;
		//Final calculated weight will be stored in , favoured towards goat
		float wt = 0;

	};

	//Some extra steps for proper recursion of lambda functions
	std::function<std::pair<int, int>(AI1&)> runai1;

	//AI version 1 runner
	runai1 = [&](AI1& ai)->std::pair<int, int> {
		
		//perform the evaluation first
		trymove(ai.move.first, ai.move.second, ai.base);

		//updates the state of base game and obtains the set of moves if set of moves is empty
		
		ai.moves = updateGame(ai.base, ai.isaitiger);

		//If not playable, or depth is final , set no of goats as wt and return invalid move
		if ((ai.base.state != PLAY)||(ai.level<=0))  {
			//letting weight = no of goat not eaten
			ai.wt = (5 - ai.base.neaten)/5.0;

			//Let's add one more form of wt measurement
			//For each group in transitions if there is both tiger and goat together 
			//Add ratio of empty places to total places in the group


			//for (auto& tran : transitions) {
			//
			//	float ng = 0;
			//	float nt = 0;
			//	for (auto& tn : tran) {
			//		switch (ai.base.board.at(tn)) {
			//		case TIGER:
			//			nt += 1;
			//			break;
			//		case GOAT:
			//			ng += 1;
			//			break;
			//		}
			//	}
			//	if (ng > 0 && nt > 0) {
			//		ai.wt -= ( ng - nt) / tran.size();
			//	}
			//
			//}
			//If won/lost then set wt accordingly
			if (ai.base.state == T_WIN)
				ai.wt = -INFINITY;
			if (ai.base.state == G_WIN)
				ai.wt = INFINITY;

			return std::pair<int, int>(-1, -1);
		}

		//Variable for child
		AI1 child;
		//Vector that stores list of most optimal moves
		std::vector<std::pair<int, int>> optmoves;
		//Set wt to least optimum value based on turn of ai
		if (ai.isoptgoat)
			ai.wt = -INFINITY;
		else
			ai.wt = +INFINITY;

		for (auto step : ai.moves) {

			//Debug breakpoint helper

			if ((ai.move.first == -1) && (step.first == 4)) {
				int in = 9;
				in = 9 + in * in;
			}

			//Reset child
			child.base = ai.base;
			child.isaitiger = !ai.isaitiger;
			child.isoptgoat = !ai.isoptgoat;
			child.level = ai.level - 1;
			child.moves.clear();
			child.move = step;

			//Discard the returning opt move as it is only used for top level
			runai1(child);
			
			//if child has equal wt then add child to the list
			if (ai.wt == child.wt)
				optmoves.push_back(child.move);
			else {
				//Now if child is more optimal then clear the optimal list first
				//If goat is to be optimized, choose child with greatest wt
				if (ai.isoptgoat) {
					if (child.wt > ai.wt) {
						ai.wt = child.wt;
						optmoves.clear();
						optmoves.push_back(child.move);
					}
				}
				//If tiger is to be optimized, choose child with lowest wt
				else {
					if (child.wt < ai.wt) {
						ai.wt = child.wt;
						optmoves.clear();
						optmoves.push_back(child.move);
					}
				}
			}
		}


		//Choose one randomly from optimal moves
		if (optmoves.empty())
			return std::pair<int, int>(-1, -1);
		int index = GetRandomValue(0, optmoves.size() - 1);
		return optmoves.at(index);




	};

	//A time delay for when both ai are enabled so as to be able to view the results
	double aitimer = GetTime();
	//The delay amount in seconds
	double aiwait = 0.7;


	while (!WindowShouldClose() && !exitbut.isactive) {

		//If window has been resized
		if (IsWindowResized()) {

			setuprecs();

			resetInfoUI();

			resetStartUI();

			//Sample box
			Rectangle samplerec = downrec(boxtorec(0));

			//resizing image to sample rectangle
			ImageResize(&tiger, samplerec.width, samplerec.height);
			ImageResize(&goat, samplerec.width, samplerec.height);

			//Image textures size 
			Texture2D tigertex = LoadTextureFromImage(tiger);
			Texture2D goattex = LoadTextureFromImage(goat);


		}

		//Stuff for when game has not started
		if (!isgamestart) {

			if (playbut.isactive) {

				isgoatai = (play1by.choice == 1);
				istigerai = (play2by.choice == 1);

				isgamestart = true;


			}

		}
		//Stuff for when game has started
		else {

			if (resetbut.isactive) {
				//Restart Game
				isgamestart = false;
				fillboard(defaultboard,main_game);
				istiger = false;
				isselect = false;
			}
			else {
				if (main_game.state == PLAY) {
					//if still playing , update game
					updateGame(main_game, istiger);
					updateInfoUI();

					//First check if ai is to be used
					if ((istiger && istigerai) || (!istiger && isgoatai)) {

						//If both ai are enabled, then wait for timer else just run damn ai
						if (!istigerai || !isgoatai || (GetTime() > (aitimer + aiwait))) {
							aitimer = GetTime();

							AI1 ai;
							ai.base = main_game;
							ai.isaitiger = istiger;
							ai.isoptgoat = !istiger;
							ai.level = 4;
							if (main_game.ngoats >= 20)
								ai.level += 2;
							ai.move = std::pair<int, int>(-1, -1);
							ai.moves.clear();

							std::pair<int, int> result = runai1(ai);

							if (trymove(result.first, result.second, main_game)) {
								istiger = !istiger;

							}
						}

					}
					else {

						//Filter mouse click event if ai is not to be used
						if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
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
										if (!istiger && putgoat(boxn, main_game)) {
											istiger = !istiger;
										}
										//If selected on tiger on tiger turn and ... select
										else if (((main_game.board.at(boxn) == GOAT) != istiger) && ((main_game.board.at(boxn) == TIGER) == istiger)) {
											selectboxn = boxn;
											isselect = true;
										}

									}
									else {
										//If can move , then move else reset selection
										if (trymove(selectboxn, boxn, main_game)) {
											isselect = false;
											istiger = !istiger;
										}
										else {
											isselect = false;
										}
									}
								}


							}

						}
					}
				}

				else {
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
		DrawLine(midrec.x, firstrec.y, lastrec.x, midrec.y, WHITE);
		DrawLine(midrec.x, lastrec.y, firstrec.x, midrec.y, WHITE);
		DrawLine(midrec.x, lastrec.y, lastrec.x, midrec.y, WHITE);


		for (int i = 0; i < 25; ++i) {

			Rectangle drawrec = downrec(boxtorec(i));


			if (main_game.board[i] == TIGER) {
				DrawTexture(tigertex, drawrec.x, drawrec.y, WHITE);
			}
			if (main_game.board[i] == GOAT) {
				DrawTexture(goattex, drawrec.x, drawrec.y, WHITE);

			}
			////Prints row,col instead of actual pieces
			//int row;
			//int col;
			//boxtorowcol(i, row, col);
			//
			//std::string str = std::to_string(row) + "," + std::to_string(col);
			//
			//Color c = RAYWHITE;
			//c.a = 190;
			//DrawText(str.c_str(), drawrec.x, drawrec.y, drawrec.width / str.length(), c);
			

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

		//If game is not playable print in big letters
		{

			switch (main_game.state) {
			case DRAW:
				Color c = RED;
				c.a = 150;
				DrawText("GAME\nDRAW", gamerec.x + gamerec.width / 3, gamerec.y + gamerec.height / 3, gamerec.width / 15, c);
				break;
			case T_WIN:
				c = GREEN;
				c.a = 150;
				DrawText("TIGER\nWINS", gamerec.x + gamerec.width / 3, gamerec.y + gamerec.height / 3, gamerec.width / 15, c);
				break;
			case G_WIN:
				c = GREEN;
				c.a = 150;
				DrawText("GOAT\nWINS", gamerec.x + gamerec.width / 3, gamerec.y + gamerec.height / 3, gamerec.width / 15, c);
				break;

			}
		}

		//If start page then do that
		if (!isgamestart)
			intropan.doStuff();
		else
			gamepanel.doStuff();

		EndDrawing();

	}

	return 0;
}