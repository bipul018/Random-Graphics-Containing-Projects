#include <raylib-cpp.hpp>

int mousestuff() {


	InitWindow(900, 900, "Muso Chalaune");
	
	SetWindowState(FLAG_WINDOW_UNDECORATED);
	SetWindowState(FLAG_WINDOW_TRANSPARENT);
	//SetWindowState(FLAG_WINDOW_ALWAYS_RUN);

	SetTargetFPS(60);

	Vector2 pos;
	pos.x = 0;
	pos.y = 0;
	

	while (!WindowShouldClose()) {
		pos.x  -=GetFrameTime()*50;
		pos.y  -=GetFrameTime()*30;

		if (pos.x > 2000)
			pos.x = 0;
		if (pos.y > 2000)
			pos.y = 0;

		SetMousePosition(pos.x, pos.y);

		ClearBackground(BLUE);
		BeginDrawing();

		EndDrawing();


	}


	return 0;
}