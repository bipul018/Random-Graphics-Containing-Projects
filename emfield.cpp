#include <raylib-cpp.hpp>
#include <iostream>
#include <thread>
#include <array>
#include <vector>
#include <functional>
using Vec2 = raylib::Vector2;
using Vec4 = raylib::Vector4;
Vec4 operator*(Vec4 a, double d) {
	return Vec4(a.x * d, a.y * d, a.z * d, a.w * d);
}
Vec4 operator+(Vec4 a, Vec4 b) {
	return Vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
struct CollideExp {
	Vec2 val;
	CollideExp(Vec2 v):val(v){}
};
struct Charge {
	Vec2 pos = Vec2(0,0);
	float charge = 1;
	const float k = 10000.0f;
	const float unitR = 10;
	Vec2 vel = Vec2(0, 0);
	Vec2 getStrength(Vec2 point)const {
		Vec2 r = point - pos;
		if (r.Length() == 0)
			throw CollideExp(Vec2(0, 0));
		Vec2 dir = r.Normalize();
		if (r.Length() < unitR * abs(charge)) {
			throw CollideExp(dir * charge * r.Length() * k / pow((unitR * abs(charge)), 3));
		}
		else {
				return dir * charge * k / (r.Length() * r.Length());
		}
	}
	float getPot(Vec2 point)const {
		Vec2 r = point - pos;
		if (r.Length() == 0)
			return 0;
		if (r.Length() < unitR * abs(charge)) {
			return charge * k / (unitR * abs(charge));
		}
		else {
				return  charge * k / (r.Length());
		}
	}
};

int emfield() {

	InitWindow(900, 900, "EM Simulation");
	SetTargetFPS(60);

	const unsigned number = 5;
	const float timeFac = 0.7f;
	std::array<Charge, number> charges;

	for (int i = 0; i < number; i++) {
		charges[i].charge = GetRandomValue(-10, 10) / 10.0f;/*
		charges[i].charge = GetRandomValue(-1, 0);
		if (charges[i].charge == 0)
			charges[i].charge = 1;*/
		charges[i].pos.x = GetRandomValue(0, 900);
		charges[i].pos.y = GetRandomValue(0, 900);
		charges[i].vel.x = GetRandomValue(0, 900)/300.0f;
		charges[i].vel.y = GetRandomValue(0, 900)/300.0f;
	}



	std::array<Vec2, number> fields;
	bool collide = false;
	auto updateField = [&]() {
		
		for (int i = 0; i < number; i++) {
			fields.at(i) = Vec2(0, 0);
			for (int j = 0; j < number; j++) {
				if (j == i)
					continue;
				fields.at(i) += charges.at(j).getStrength(charges.at(i).pos);
			}
		}
	};
	auto updatePos = [&]() {
		for (int i = 0; i < number; i++) {
			float tempFac = timeFac * GetFrameTime();


			//charges.at(i).pos += charges.at(i).vel*timeFac;
			//charges.at(i).vel += fields.at(i) * charges.at(i).charge *  timeFac;

			charges.at(i).pos += charges.at(i).vel*GetFrameTime()*timeFac;
			charges.at(i).vel += fields.at(i) * charges.at(i).charge * GetFrameTime() * timeFac;
		}
	};
	float maxPot = -100;
	raylib::Color* board = new raylib::Color[900 * 900];
	auto getCol = [&](int i, int j)->raylib::Color& {return board[i * 900 + j]; };
	while (!WindowShouldClose()) {
		ClearBackground(WHITE);

		std::function<void(int, int)> threader = [&](int starty, int endy) {

			for (int x = 0; x < 900; x++) {
				for (int y = starty; y < endy; y++) {
					float pot = 0;
					for (auto& q : charges)
						pot += q.getPot(Vec2(x, y));
					auto r = Vec4(RED);
					auto b = Vec4(BLUE);
					float fac = pot / maxPot;
					if (fac > 1)
						fac = 1;
					if (fac < -1)
						fac = -1;
					fac = (fac + 1) / 2;
					auto avg = b * fac + r * (1 - fac);
					getCol(y,x) = raylib::Color(avg);
				}
			}
		};

		std::thread thr(threader, 0, 450);
		threader(450, 900);
		thr.join();


		BeginDrawing();

		for (int x = 0; x < 900; x++) {
			for (int y = 0; y < 900; y++) {
				getCol(y,x).DrawPixel(Vec2(x, y));
			}
		}

		for (int i = 0; i < number; i++) {
			auto& q = charges.at(i);
			auto& f = fields.at(i);
			DrawCircle(q.pos.x, q.pos.y, q.unitR * abs(q.charge), (signbit(q.charge) ? BLACK : YELLOW));

			//DrawLine(q.pos.x, q.pos.y, q.pos.x + q.vel.x * 10, q.pos.y + q.vel.y * 10, BLUE);
			DrawLine(q.pos.x, q.pos.y, q.pos.x + f.x * 1000, q.pos.y + f.y * 1000, GRAY);


		}
		EndDrawing();
		if (true) {
			try {
				updateField();
			}
			catch (CollideExp c) {
				collide = true;
			}
			updatePos();
		}
	}
	return 0;
}
