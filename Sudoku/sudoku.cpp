#include <raylib-cpp.hpp>
#include <iostream>
#include <thread>
#include <array>
#include <vector>
#include <functional>

int main() {

	InitWindow(1200, 900, "Sudoku");
	SetTargetFPS(60);

	unsigned col = 0;
	unsigned row = 0;

	typedef std::array<std::array<std::array<bool, 9>, 9>, 9> Game;
	Game cells;
	//std::array<bool, 9> cells[9][9];
	for (auto& x : cells) {
		for (auto& y : x) {
			for (auto& z : y)
				z = false;
		}
	}

	std::vector< Game> games;
	Rectangle hintButton{ 945, 5, 250, 45 };
	Rectangle clearButton{ 945, 80, 250, 45 };
	Rectangle saveButton{ 945, 160, 250, 45 };
	Rectangle popButton{ 945,240,250,45 };
	std::array<Rectangle, 7> loadButton;
	loadButton[0]={ 945, 320, 250, 45 };
	loadButton[1]={ 945, 400, 250, 45 };
	loadButton[2]={ 945, 480, 250, 45 };
	loadButton[3]={ 945, 560, 250, 45 };
	loadButton[4]={ 945, 640, 250, 45 };
	loadButton[5]={ 945, 720, 250, 45 };
	loadButton[6]={ 945, 800, 250, 45 };

	auto inRect = CheckCollisionPointRec;

	auto clearExtraHints = [&](bool fill = false)->bool {
		bool hasChanged = false;
		for (int i = 0; i < 9; i++) {
			std::array<bool, 9> temp;
			std::array<bool, 9> temp2;
			for (auto& t : temp)
				t = false;
			for (auto& t : temp2)
				t = false;
			for (int j = 0; j < 9; j++) {
				int flag = 0;
				for (int k = 0; k < 9; k++)
					if (cells[i][j][k]) {
						if (flag == 0)
							flag = k + 1;
						else
							flag = -1;
					}
				if (flag <= 0 && fill)
					for (auto& k : cells[i][j])
						k = true;
				else if(flag >0) {
					temp.at(flag - 1) = true;
					temp2.at(j) = true;
				}
			}
			for (int j = 0; j < 9; j++) {
				if (!temp2[j])
					for (int k = 0; k < 9; k++) {
						if (temp[k])
							if (cells[i][j][k]) {
								cells[i][j][k] = false;
								hasChanged = true;
							}

					}
			}
		}
		for (int j = 0; j < 9; j++) {
			std::array<bool, 9> temp;
			std::array<bool, 9> temp2;
			for (auto& t : temp)
				t = false;
			for (auto& t : temp2)
				t = false;
			for (int i = 0; i < 9; i++) {
				int flag = 0;
				for (int k = 0; k < 9; k++)
					if (cells[i][j][k]) {
						if (flag == 0)
							flag = k + 1;
						else
							flag = -1;
					}
				if (flag > 0) {
					temp.at(flag - 1) = true;
					temp2.at(i) = true;
				}
			}
			for (int i = 0; i < 9; i++) {
				if (!temp2[i])
					for (int k = 0; k < 9; k++) {
						if (temp[k])
							if (cells[i][j][k]) {
								cells[i][j][k] = false;
								hasChanged = true;
							}

					}
			}
		}

		for (int i = 0; i < 9; i++) {
			std::array<bool, 9> temp;
			std::array<bool, 9> temp2;
			for (auto& t : temp)
				t = false;
			for (auto& t : temp2)
				t = false;
			for (int j = 0; j < 9; j++) {
				int flag = 0;
				for (int k = 0; k < 9; k++)
					if (cells[(i / 3) * 3 + j / 3][(i % 3) * 3 + j % 3][k]) {
						if (flag == 0)
							flag = k + 1;
						else
							flag = -1;
					}
				if (flag > 0) {
					temp.at(flag - 1) = true;
					temp2.at(j) = true;
				}
			}
			for (int j = 0; j < 9; j++) {
				if (!temp2[j])
					for (int k = 0; k < 9; k++) {
						if (temp[k]) {
							if (cells[(i / 3) * 3 + j / 3][(i % 3) * 3 + j % 3][k]) {
								cells[(i / 3) * 3 + j / 3][(i % 3) * 3 + j % 3][k] = false;
								hasChanged = true;
							}
						}
					}
			}
		}
		return hasChanged;
	};

	auto countFilled = [](const std::array<bool, 9>& arr)->int {
		int c = 0;
		for (auto x : arr)
			if (x)
				++c;
		return c;
	};

	while (!WindowShouldClose()) {
		ClearBackground(WHITE);

		for (unsigned key = KEY_ONE; key <= KEY_NINE; key++) {
			if (IsKeyReleased(key) || IsKeyReleased(key - KEY_ONE + KEY_KP_1)) {
				cells[row][col].at(key - KEY_ONE) ^= true;
				if (countFilled(cells[row][col])==1)
					while (clearExtraHints()) {

					}

			}
		}



		if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			if (IsCursorOnScreen()) {
				if (GetMouseX() < 900 && GetMouseY() < 900) {
					col = GetMouseX() / 100;
					row = GetMouseY() / 100;
				}
				
				else if (inRect(GetMousePosition(), hintButton)) {
					clearExtraHints(true);
				}


				else if (inRect(GetMousePosition(), saveButton) && games.size() < loadButton.size()) {
					games.push_back(cells);
				}
				else if (inRect(GetMousePosition(), clearButton) ) {
					cells = Game();
				}
				else if (inRect(GetMousePosition(), popButton) ) {
					if (games.size() > 0)
						games.pop_back();
				}
				else {
					for (int i = 0; i < games.size(); i++)
						if (inRect(GetMousePosition(), loadButton[i])) {
							cells = games.at(i);
						}
				}
			}
		}
		if (IsKeyReleased(KEY_UP)) {
			row = (row - 1+9) % 9;
		}
		else if (IsKeyReleased(KEY_DOWN)) {
			row = (row + 1) % 9;
		}
		else if (IsKeyReleased(KEY_LEFT)) {
			col = (col - 1+9) % 9;
		}
		else if (IsKeyReleased(KEY_RIGHT)) {
			col = (col + 1) % 9;
		}
		else if (IsKeyReleased(KEY_TAB)) {
			col++;
			if (col >= 9) {
				col = 0;
				row = (row + 1) % 9;
			}

		}
		else if (IsKeyReleased(KEY_SPACE)) {
			for (auto& x : cells[row][col])
				x = false;
		}
		

		BeginDrawing();
		DrawRectangle(0, row * 100, 900, 100, SKYBLUE);
		DrawRectangle(col*100, 0, 100, 900, SKYBLUE);
		DrawRectangle((col/3)*300, (row/3)*300, 300, 300, SKYBLUE);
		DrawRectangle(col * 100, row * 100, 100, 100, DARKBLUE);


		//ixj => rowxcol => y*x
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 9; j++) {
				//smaller square
				Rectangle r;
				r.x = j * 100;
				r.y = i * 100;
				r.width = 100;
				r.height = 100;
				std::string str[3];
				DrawRectangleLinesEx(r, 3, YELLOW);
				//larger square
				Rectangle r2;
				r2.x = int(j / 3) * 300;
				r2.y = int(i / 3) * 300;
				r2.width = 300;
				r2.height = 300;
				DrawRectangleLinesEx(r2, 3, BLACK);

				int flag = 0;
				for (int k = 0; k < 9; k++) {
					if (cells[i][j].at(k)) {
						str[k / 3] += ' ';
						str[k / 3] += ('1' + k);
						if (flag == 0)
							flag = k + 1;
						else
							flag = -1;
					}
					else {
						str[k / 3] += "  ";
					}
				}
				if (flag < 0) {
					DrawText(str[0].c_str(), r.x + 3, r.y + 3, 30, MAROON);
					DrawText(str[1].c_str(), r.x + 3, r.y + 3 + 30, 30, MAROON);
					DrawText(str[2].c_str(), r.x + 3, r.y + 3 + 60, 30, MAROON);
				}
				else if (flag > 0) {
					str[0] = ' ';
					str[0] += '0' + flag;
					DrawText(str[0].c_str(), r.x + 3, r.y + 3, 90, MAROON);
				}

			}
		}
	
		DrawRectangleRec(popButton, MAROON);
		DrawText("Pop Game ", popButton.x+5, popButton.y+5, 30, DARKBLUE);
		DrawRectangleRec(hintButton, MAROON);
		DrawText("Fill Hints ", hintButton.x+5, hintButton.y+5, 30, DARKBLUE);
		DrawRectangleRec(saveButton, MAROON);
		DrawText(" Save Game ", saveButton.x+5, saveButton.y+5, 30, DARKBLUE);
		DrawRectangleRec(clearButton, MAROON);
		DrawText("Clear Game ", clearButton.x+5, clearButton.y+5, 30, DARKBLUE);
		for (int i = 0; i < games.size(); i++) {
			DrawRectangleRec(loadButton[i], MAROON);
			std::string str = "Load Game " + std::to_string(i + 1);
			DrawText(str.c_str(), loadButton[i].x + 5, loadButton[i].y + 5, 30, DARKBLUE);
		}
		EndDrawing();
	}
	return 0;
}
