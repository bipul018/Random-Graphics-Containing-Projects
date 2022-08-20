#include <raylib-cpp.hpp>
#include <iostream>
#include <thread>
#include <array>
#include <vector>
#include <algorithm>
#include <functional>

//Parallel Computation tryout stuff
#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/functional/math.hpp>
#include <boost/compute/core.hpp>
#include <boost/compute/functional/bind.hpp>
namespace compute = boost::compute;


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
constexpr float k = 10000.0f;
constexpr float unitR = 10;
struct Charge {
	Vec2 pos = Vec2(0,0);
	float charge = 1;
	const float k = ::k;
	const float unitR = ::unitR;
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
//Parallel stuff

struct PointPot {
	float x;
	float y;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

};
BOOST_COMPUTE_ADAPT_STRUCT(PointPot, PointPot, (x, y, r, g, b, a));

int emfield() {

	InitWindow(900, 900, "EM Simulation");
	SetTargetFPS(60);

	const unsigned number = 3;
	const float timeFac = 2.0f;
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
	charges[0].charge = 1;
	charges[1].charge = -1;


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

	//parallel stuff tryout

	// get default device and setup context
	compute::device device = compute::system::default_device();
	compute::context context(device);
	compute::command_queue queue(context, device);

	//Device data
	PointPot* points = new PointPot[900 * 900];
	auto getPoint = [&](int x, int y)->PointPot& {return points[y * 900 + x]; };

	for (int x = 0; x < 900; x++) {
		for (int y = 0; y < 900; y++) {
			getPoint(x, y).x = x;
			getPoint(x, y).y = y;
			getPoint(x, y).r = 0;
			getPoint(x, y).g = 0;
			getPoint(x, y).b = 0;
			getPoint(x, y).a = 0;
		}
	}

	//Ends with assigning value of q
	std::string part1 = "PointPot calculate_pot(PointPot pp) {float q = ";
	//Ends with assigning value of qx
	std::string part2 = " ; float qx = ";
	//Ends with assigning value of qy
	std::string part3 = " ; float qy = ";
	//Ends the function
	std::string part4 = " ; float k = "+std::to_string(k)+";float unitR = "+std::to_string(unitR) + "; PointPot p = pp;\
double r = sqrt((p.x - qx) * (p.x - qx) + (p.y - qy) * (p.y - qy));\
																   \
	const float maxPot = -100;								   \
	float pot = 0;												   \
	if (r < unitR * ((q>0)?q:-q)) {								   \
		pot = q * k / (unitR * ((q>0)?q:-q));				   \
	}															   \
	else {														   \
		pot = q * k / (r);									   \
	}															   \
	float fac = pot / maxPot;									   \
	if (fac > 1)												   \
		fac = 1;												   \
	if (fac < -1)												   \
		fac = -1;												   \
	fac = (fac + 1) / 2;										   \
	p.r = 0 * fac + 230 * (1 - fac);							   \
	p.g = 121 * fac + 41 * (1 - fac);							   \
	p.b = 241 * fac + 55 * (1 - fac);							   \
	p.a = 255 * fac + 255 * (1 - fac);							   \
	return p;													   \
}";

	// create a vector on the device
	compute::vector<PointPot> device_vector(900*900, context);


	// transfer data from the host to the device
	compute::copy(
		points, points + 900 * 900, device_vector.begin(), queue
	);

	while (!WindowShouldClose()) {
		ClearBackground(WHITE);

		//std::function<void(int, int)> threader = [&](int starty, int endy) {

		//	for (int x = 0; x < 900; x++) {
		//		for (int y = starty; y < endy; y++) {
		//			float pot = 0;
		//			for (auto& q : charges)
		//				pot += q.getPot(Vec2(x, y));
		//			auto r = Vec4(RED);
		//			auto b = Vec4(BLUE);
		//			float fac = pot / maxPot;
		//			if (fac > 1)
		//				fac = 1;
		//			if (fac < -1)
		//				fac = -1;
		//			fac = (fac + 1) / 2;
		//			auto avg = b * fac + r * (1 - fac);
		//			getCol(y,x) = raylib::Color(avg);
		//		}
		//	}
		//};

		////std::thread thr(threader, 0, 450);
		//threader(0, 900);
		////thr.join();

	//parllel stuff
	// calculate the pot of each element in-place
		for (auto& qz : charges) {
			
			float q = qz.charge;
			float qx = qz.pos.x;
			float qy = qz.pos.y;
			std::string tmp = part1 + std::to_string(q) + part2 + std::to_string(qx) + part3 + std::to_string(qy) + part4;
			boost::compute::function<PointPot(PointPot)> calculate_pot =
				boost::compute::make_function_from_source<PointPot(PointPot)>(
					"calculate_pot",
					tmp
					);

			
			compute::transform(
				device_vector.begin(),
				device_vector.end(),
				device_vector.begin(),
				calculate_pot,
				queue
			);
		}
		// copy values back to the host
		compute::copy(
			device_vector.begin(), device_vector.end(), points, queue
		);



		BeginDrawing();
		//parallel stuff
		for (int x = 0; x < 900; x++) {
			for (int y = 0; y < 900; y++) {
				auto pot = getPoint(x, y);
				raylib::Color c;
				c.r = pot.r;
				c.g = pot.g;
				c.b = pot.b;
				c.a = pot.a;
				c.DrawPixel(x, y);
			}
		}
		

	/*	for (int x = 0; x < 900; x++) {
			for (int y = 0; y < 900; y++) {
				getCol(y,x).DrawPixel(x, y);
			}
		}*/

		for (int i = 0; i < number; i++) {
			auto& q = charges.at(i);
			auto& f = fields.at(i);
			DrawCircle(q.pos.x, q.pos.y, q.unitR * abs(q.charge), (signbit(q.charge) ? BLACK : YELLOW));

			//DrawLine(q.pos.x, q.pos.y, q.pos.x + q.vel.x * 10, q.pos.y + q.vel.y * 10, BLUE);
			DrawLine(q.pos.x, q.pos.y, q.pos.x + f.x * 1000, q.pos.y + f.y * 1000, GRAY);


		}
		DrawFPS(10, 10);
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

		if (number >= 2) {
			if ((charges[0].pos.x < 0 || charges[0].pos.x>900 ||
				charges[0].pos.y < 0 || charges[0].pos.y>900) &&
				(charges[1].pos.x < 0 || charges[1].pos.x>900 ||
				charges[1].pos.y < 0 || charges[1].pos.y>900)) {

				charges[0].pos.x = GetRandomValue(0, 900);
				charges[0].pos.y = GetRandomValue(0, 900);
				charges[0].vel.x = GetRandomValue(0, 900) / 300.0f;
				charges[0].vel.y = GetRandomValue(0, 900) / 300.0f;

				charges[1].pos.x = GetRandomValue(0, 900);
				charges[1].pos.y = GetRandomValue(0, 900);
				charges[1].vel.x = GetRandomValue(0, 900) / 300.0f;
				charges[1].vel.y = GetRandomValue(0, 900) / 300.0f;
			}
		}

	}
	return 0;
}
