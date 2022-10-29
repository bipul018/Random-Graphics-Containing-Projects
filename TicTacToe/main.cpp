#include <raylib.h>
#define ADD_GUI_IMPL
#include <MyGUI.h>
#include <thread>
#include <mutex>
#include <array>
#include <chrono>
#include <random>
#include <memory>
#include <cassert>
#include <iostream>

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
	SetTargetFPS(30);

	//Button butt;
	//butt.box.x = width/3;
	//butt.box.y = height/3;
	//butt.box.width = width/3;
	//butt.box.height = height / 3;
	//
	//ScrollBar slide;
	//slide.box.x = 10;
	//slide.box.y = 10;
	//slide.box.width = 50;
	//slide.box.height = 200;
	//slide.maxValue = 199;



	//Tac<3,3> game;
	//game.at(0, 1) = 'x';
	//game.at(1, 0) = 'o';
	//
	//RenderTexture2D tex = LoadRenderTexture(width/3, height/3);
	//Rectangle viewrec;
	//viewrec.x = 0;
	//viewrec.y = 0;
	//viewrec.width =   (float)tex.texture.width;
	//viewrec.height = -(float)tex.texture.height / 2;
	//
	//float scrollH = tex.texture.height + viewrec.height;

	std::mt19937_64 prng{ std::random_device{}()};
	

	std::array<Rectangle, 25> boxes;

	float boxlen = ((width > height) ? height : width) / sqrtf(9*static_cast<float>(boxes.size()));
	auto boxcol = ColorFromNormalized(QuaternionScale(QuaternionAdd(ColorNormalize(RED), ColorNormalize(RED)), 0.5f));

	//Generate a box 
	auto genbox = [&prng](Rectangle srcrec, float wid, float hei) {

		Rectangle rec;
		rec.width = wid;
		rec.height = hei;

		std::uniform_real_distribution<float> dist1{ srcrec.x,srcrec.x + srcrec.width - wid };	
		std::uniform_real_distribution<float> dist2{ srcrec.y,srcrec.y + srcrec.height - hei };

		rec.x = dist1(prng);
		rec.y = dist2(prng);

		return rec;

	};

	Rectangle winrec{ 0,0,width,height };


	auto genseparatebox = [&]( unsigned index , float paddingfac = 2.5f) {

		bool coincide = false;
		do {
			boxes.at(index) = genbox(winrec, boxlen*paddingfac, boxlen*paddingfac);
			coincide = false;
			for (int i = 0; i < boxes.size(); ++i) {
				if (i != index)
					if (CheckCollisionRecs(boxes.at(i), boxes.at(index))) {
						coincide = true;
						break;
					}
			}
		} while (coincide);
		boxes.at(index).x += boxes.at(index).width * 0.5f;
		boxes.at(index).y += boxes.at(index).height * 0.5f;
		boxes.at(index).width /= paddingfac;
		boxes.at(index).height /= paddingfac;
		boxes.at(index).x -= boxes.at(index).width * 0.5f;
		boxes.at(index).y -= boxes.at(index).height * 0.5f;
	};

	boxes.fill(Rectangle{ 0,0,0,0 });

	for (int i = 0; i < boxes.size(); ++i) {
		genseparatebox(i,2.5f);
	}

	//Now the ball
	Vector2 center{ width / 2,height / 2 };
	float radius = boxlen;
	Vector2 vel{ 0,0 };
	auto c_color = BLUE;

	//A fxn that updates values based on deltime given
	auto updatestuff = [&](float deltime) {

		center = Vector2Add(center, Vector2Scale(vel, deltime));

		if (center.x < winrec.x + radius) {
			center.x = winrec.x + radius;
			vel.x = -vel.x;
		}
		if (center.x > winrec.x + winrec.width - radius) {
			center.x = winrec.x + winrec.width - radius;
			vel.x = -vel.x;
		}

		if (center.y < winrec.y + radius) {
			center.y = winrec.y + radius;
			vel.y = -vel.y;
		}
		if (center.y > winrec.y + winrec.height - radius) {
			center.y = winrec.y + winrec.height - radius;
			vel.y = -vel.y;
		}


		for (int i = 0; i < boxes.size(); ++i) {
			if (CheckCollisionCircleRec(center, radius, boxes.at(i))) {
				genseparatebox(i, 3.f);
			}
		}

	};

	//Now multithreading fxn to be used
	bool running = true; 
	std::unique_ptr<std::thread> thrptr;
	std::mutex mlock;
	std::mutex pauselock;

	class Timer {
	private:
		// Type aliases to make accessing nested type easier
		using Clock = std::chrono::steady_clock;
		using Second = std::chrono::duration<double, std::ratio<1> >;

		std::chrono::time_point<Clock> m_beg{ Clock::now() };

	public:
		void reset() {
			m_beg = Clock::now();
		}

		double elapsed() const {
			return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
		}
	};

	

	auto threadrun = [&](float steptime = 0.01f) {
		Timer timer{};
		Timer pausetime{};
		float duration = 0.f;
		while (running) {
			duration = timer.elapsed() + duration;
			timer.reset();
			float paused_dur = 0.f;

			while (duration >= steptime) {
				mlock.lock();

				pausetime.reset();
				pauselock.lock();
				paused_dur += pausetime.elapsed();

				updatestuff(steptime);
				pauselock.unlock();
				mlock.unlock();
				duration -= steptime;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(1000.f * (steptime - duration))));
			duration -= paused_dur;

		}

	};

	thrptr = std::make_unique<std::thread>(threadrun);



	while (!WindowShouldClose() ) {
		//draw(tex, game);

		if (IsKeyReleased(KEY_SPACE)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		}
		if (IsKeyPressed(KEY_ENTER)) {
			pauselock.lock();
		}
		if (IsKeyReleased(KEY_ENTER)) {
			pauselock.unlock();
		}
		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
			mlock.lock();
			vel = Vector2Scale(Vector2Normalize(Vector2Subtract(GetMousePosition(), center)), 220.f);
			mlock.unlock();
		}


		BeginDrawing();
		ClearBackground(RAYWHITE);

		for (auto& box : boxes) {
			DrawRectangleRec(box, boxcol);
		}
		DrawCircleV(center, radius, c_color);
		DrawFPS(10, 10);

		// NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom) 
		//DrawTextureRec(tex.texture, Rectangle{ 0, viewrec.height-(slide.value-slide.minValue) * scrollH / (slide.maxValue - slide.minValue), viewrec.width, viewrec.height}, Vector2{(float)width / 3, (float)height / 3}, WHITE);
		//
		//BeginScissorMode(slide.box.x, slide.box.y, slide.box.width * 2, slide.box.height / 2);
		//ClearBackground(GREEN);
		//slide.doStuff();
		//EndScissorMode();
		EndDrawing();

	}
	//UnloadRenderTexture(tex);
	
	running = false;
	thrptr->join();

	CloseWindow();
	return 0;
}