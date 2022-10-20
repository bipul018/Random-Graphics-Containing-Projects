#include <raylib.h>
#define ADD_GUI_IMPL
#include <MyGUI.h>


template<unsigned m = 3,unsigned n = m>
class Tac {
public:
	unsigned k = n;
	char data[m * n + 1];

	char& state = data[m * n];

	enum State {
		PLAY,
		DRAW,
		XWIN,
		OWIN
	} ;

	Tac(unsigned wincount = n) :k(wincount) {
		for (auto& d : data) {
			d = ' ';
		}
		data[m * n] = PLAY;
	}

	bool validate(int row, int col) {
		return (row >= 0 && col >= 0 && row < m&& col < n);
	}

	//Considers that the range is valid for now
	char& at(int row, int col) {
		return data[row * n + col];
	}

	//Updates game state and returns true if game is finished
	bool updateState() {

		return false;

	}

};

template<unsigned m = 3, unsigned n = m>
RenderTexture2D& draw(RenderTexture2D& tex, Tac<m, n> game) {
	float wid = tex.texture.width / (n + 1);
	float hei = tex.texture.height / (m + 1);
	BeginTextureMode(tex);
	ClearBackground(DARKBLUE);

	int col = 0;
	for (float x = wid/2; x <= tex.texture.width - 3*wid/2; x+=wid) {
		int row = 0;
		for (float y = hei / 2; y <= tex.texture.height - 3*hei / 2; y+=hei) {

			Rectangle rec;
			rec.x = x;
			rec.y = y;
			rec.width = wid;
			rec.height = hei;

			DrawRectangleLinesEx(rec, 2, WHITE);
			float fsize = wid;
			Vector2 size;
			do {
				size = MeasureTextEx(GetFontDefault(), " X ", fsize, 0);
				fsize -= 1.0;
			} while ((size.x >= wid) || (size.y >= hei));
			fsize += 1.0;
			switch (game.at(row, col)) {
			case 'x':
				DrawTextEx(GetFontDefault(), " X ", Vector2{ x, y + hei / 2 - size.y / 2 }, fsize,0, WHITE);
				break;
			case 'o':
				DrawTextEx(GetFontDefault(), " O ", Vector2{ x, y + hei / 2 - size.y / 2 }, fsize, 0, WHITE);
				break;
			}
			row++;

		}
		col++;
	}

	EndTextureMode();
	return tex;

}

int main() {

	int width = 1200;
	int height = 900;

	InitWindow(width, height, "Tic Tac Toe");

	Button butt;
	butt.box.x = width/3;
	butt.box.y = height/3;
	butt.box.width = width/3;
	butt.box.height = height / 3;

	ScrollBar slide;
	slide.box.x = 10;
	slide.box.y = 10;
	slide.box.width = 50;
	slide.box.height = 200;
	slide.maxValue = 199;



	Tac<3,3> game;
	game.at(0, 1) = 'x';
	game.at(1, 0) = 'o';

	RenderTexture2D tex = LoadRenderTexture(width/3, height/3);
	Rectangle viewrec;
	viewrec.x = 0;
	viewrec.y = 0;
	viewrec.width =   (float)tex.texture.width;
	viewrec.height = -(float)tex.texture.height / 2;

	float scrollH = tex.texture.height + viewrec.height;


	while (!WindowShouldClose() ) {
		draw(tex, game);
		BeginDrawing();
		ClearBackground(RAYWHITE);

		

		// NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom) 
		DrawTextureRec(tex.texture, Rectangle{ 0, viewrec.height-(slide.value-slide.minValue) * scrollH / (slide.maxValue - slide.minValue), viewrec.width, viewrec.height}, Vector2{(float)width / 3, (float)height / 3}, WHITE);

		BeginScissorMode(slide.box.x, slide.box.y, slide.box.width * 2, slide.box.height / 2);
		ClearBackground(GREEN);
		slide.doStuff();
		EndScissorMode();
		EndDrawing();

	}
	UnloadRenderTexture(tex);
	CloseWindow();
	return 0;
}