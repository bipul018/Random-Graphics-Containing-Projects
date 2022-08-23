#include <raylib-cpp.hpp>
#include <iostream>
#include <thread>
#include <array>
#include <vector>
#include <algorithm>
#include <functional>
#include <exception>

//Parallel Computation tryout stuff
#ifndef N_DEBUG
#define BOOST_COMPUTE_DEBUG_KERNEL_COMPILATION
#endif

#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/functional/math.hpp>
#include <boost/compute/core.hpp>
#include <boost/compute/functional/bind.hpp>
#include <boost/compute/utility/source.hpp>
#include <boost/compute/command_queue.hpp>
namespace compute = boost::compute;


using Vec2 = raylib::Vector2;
using Vec4 = raylib::Vector4;
Vec4 operator*(Vec4 a, double d) {
	return Vec4(a.x * d, a.y * d, a.z * d, a.w * d);
}
Vec4 operator+(Vec4 a, Vec4 b) {
	return Vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}


constexpr float k = 10000.0f;
constexpr float unitR = 10;



//Parallel stuff



struct Point {
	
	cl_float x;
	cl_float y;
};

struct Charge {
	cl_float q;
	Point pos;
	Point vel;
};

struct MyColor {
	
	cl_uchar r;
	cl_uchar g;
	cl_uchar b;
	cl_uchar a;
};
BOOST_COMPUTE_ADAPT_STRUCT(Point, Point, (x, y));
BOOST_COMPUTE_ADAPT_STRUCT(Charge, Charge, (q, pos, vel));
BOOST_COMPUTE_ADAPT_STRUCT(MyColor, MyColor, (r,g,b,a));


int emfield() {

	if ((sizeof(Point) != (sizeof(Point::x) + sizeof(Point::y))) ||
		(sizeof(MyColor) != (sizeof(MyColor::r) + sizeof(MyColor::g) + sizeof(MyColor::b) + sizeof(MyColor::a))) ||
		(sizeof(Charge) != (sizeof(Charge::q) + sizeof(Charge::pos) + sizeof(Charge::vel))))
		return -1;

	constexpr size_t width = 900;
	constexpr size_t height = 900;
	const cl_int clWid = width;
	const cl_int clHei = height;

	InitWindow(width, height, "EM Simulation");
	SetTargetFPS(60);

	constexpr size_t NumofCharges = 50;
	const cl_int nCharges = NumofCharges;
	std::array<Charge, NumofCharges> charges;

	for (auto& chr : charges) {
		chr.q = GetRandomValue(-10, 10)/10.0;
		chr.pos.x = GetRandomValue(0, width);
		chr.pos.y = GetRandomValue(0, height);
		chr.vel.x = GetRandomValue(-100, 100) / 50.0;
		chr.vel.y = GetRandomValue(-100, 100) / 50.0;
	}

	constexpr float k = 10000;
	constexpr float unitRadius = 10;
	constexpr float maxPot = -500;

	float constants[3];
	constants[0] = k;
	constants[1] = unitRadius;
	constants[2] = maxPot;

	std::vector<MyColor> board;
	std::vector<Point> points;
	

	board.resize(width * height);
	points.resize(width * height);

	for (size_t y = 0; y < height; ++y) {
		for (size_t x = 0; x < width; ++x) {
			board[y * width + x].r = 255;
			board[y * width + x].g = 0;
			board[y * width + x].b = 0;
			board[y * width + x].a = 255;

			points[y * width + x].x = x;
			points[y * width + x].y = y;
		}
	}


	//Initialize boost::compute
	compute::device device = compute::system::default_device();
	compute::context context(device);
	compute::command_queue queue(context, device);


	const char device_codes[] = BOOST_COMPUTE_STRINGIZE_SOURCE(

	typedef struct {

		float x;
		float y;
	} Point;\n

	typedef struct {
		float q;
		Point pos;
		Point vel;
	} Charge;\n

	typedef struct  {

		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	}MyColor;\n

	float my_abs(float num) {
		return (num > 0) ? num : -num;
	}\n

	kernel void calc_pot(global Point* points, global Charge* charges,global float* results, global float* constants,int nCharges) {
		\n
		size_t index = get_global_id(0);
		results[index] = 0;
		\n
			for (size_t i = 0; i < nCharges; ++i) {
			\n
			float r = sqrt((points[index].x - charges[i].pos.x) * (points[index].x - charges[i].pos.x) +
				(points[index].y - charges[i].pos.y) * (points[index].y - charges[i].pos.y));
			\n
			if (r < constants[1] * my_abs(charges[i].q)) {
				results[index] += charges[i].q * constants[0] / (constants[1] * my_abs(charges[i].q));
			}
			\n
			else {
				results[index] += charges[i].q * constants[0] / r;
			}
			\n
		}
	}\n
	kernel void calc_elec_field(global Point* points, global Charge* charges,global Point* elec_field, global float* constants,int nCharges) {
		\n
		size_t index = get_global_id(0);
		elec_field[index].x = 0;
		elec_field[index].y = 0;
		\n
			for (size_t i = 0; i < nCharges; ++i) {
			\n
			Point vr ;
			vr.x = points[index].x - charges[i].pos.x;
			vr.y = points[index].y - charges[i].pos.y;

			float r = sqrt(vr.x*vr.x+vr.y*vr.y);
			\n
			if(r == 0){
			}
			if (r < constants[1] * my_abs(charges[i].q)) {
				float mag = charges[i].q * constants[0] / (r * r * constants[1] * my_abs(charges[i].q));
				elec_field[index].x += mag * vr.x;
				elec_field[index].y += mag * vr.y;
			}
			\n
			else {
				float mag = charges[i].q * constants[0] / (r * r * r);
				elec_field[index].x += mag * vr.x;
				elec_field[index].y += mag * vr.y;
			}
			\n
		}
	}\n

	kernel void update_pos_vel(global Charge* charges, global Point* elec_field,int maxWidth, int maxHeight){
		size_t index = get_global_id(0);
		int x = (int)(charges[index].pos.x);
		int y = (int)(charges[index].pos.y);
		if (x >= 0 && x < maxWidth && y >= 0 && y < maxHeight) {
			charges[index].pos.x += charges[index].vel.x;
			charges[index].pos.y += charges[index].vel.y;
			charges[index].vel.x += elec_field[y * maxWidth + x].x;
			charges[index].vel.y += elec_field[y * maxWidth + x].y;
		}
	}

	kernel void calc_col(global float* pots, global MyColor* results, global float* constants) {
		size_t index = get_global_id(0);
		float fac = pots[index] / constants[2];
		if (fac > 1)
			fac = 1;
		if (fac < -1)
			fac = -1;
		fac = (fac + 1) / 2;
		MyColor col;
		col.r = 0 * fac + 255 * (1 - fac);
		col.g = 0 * fac + 255 * (1 - fac);
		col.b = 255 * fac + 0 * (1 - fac);
		col.a = 255;
		results[index] = col;
	}\n



	);


	compute::program device_prog = compute::program::build_with_source(device_codes, context);

	compute::kernel pot_kernel(device_prog, "calc_pot");
	compute::kernel col_kernel(device_prog, "calc_col");
	compute::kernel elec_field_kernel(device_prog, "calc_elec_field");
	compute::kernel pos_kernel(device_prog, "update_pos_vel");

	compute::vector<Point> device_points(width* height, context);
	compute::vector<MyColor> device_board(width* height, context);
	compute::vector<Charge> device_charges(nCharges, context);
	compute::vector<float> device_pots(width* height, context);
	compute::vector<float> device_consts(sizeof(constants) / sizeof(constants[0]), context);
	compute::vector<Point> device_elec_field(width* height, context);

	auto time_s = GetTime();

	compute::copy(charges.begin(), charges.end(), device_charges.begin(), queue);
	compute::copy(points.begin(), points.end(), device_points.begin(), queue);
	compute::copy(board.begin(), board.end(), device_board.begin(), queue);
	compute::copy(constants, constants + sizeof(constants) / sizeof(constants[0]), device_consts.begin(), queue);

	pot_kernel.set_arg(0, device_points.get_buffer());
	pot_kernel.set_arg(1, device_charges.get_buffer());
	pot_kernel.set_arg(2, device_pots.get_buffer());
	pot_kernel.set_arg(3, device_consts.get_buffer());
	pot_kernel.set_arg(4, sizeof(nCharges), &nCharges);


	col_kernel.set_arg(0, device_pots.get_buffer());
	col_kernel.set_arg(1, device_board.get_buffer());
	col_kernel.set_arg(2, device_consts.get_buffer());

	elec_field_kernel.set_arg(0, device_points.get_buffer());
	elec_field_kernel.set_arg(1, device_charges.get_buffer());
	elec_field_kernel.set_arg(2, device_elec_field.get_buffer());
	elec_field_kernel.set_arg(3, device_consts.get_buffer());
	elec_field_kernel.set_arg(4, sizeof(nCharges), &nCharges);

	pos_kernel.set_arg(0, device_charges.get_buffer());
	pos_kernel.set_arg(1, device_elec_field.get_buffer());
	pos_kernel.set_arg(2, clWid);
	pos_kernel.set_arg(3, clHei);

	std::cout << "Time spent = " << GetTime() - time_s << std::endl;

	std::cout << std::endl << "Fucking 11 pot : " << static_cast<float>(device_pots.at(11)) << std::endl;
	std::cout<<std::endl << "Fucking green value : "<<static_cast<int>(static_cast<MyColor>(device_board.at(2)).b) <<" got it ?" << std::endl;

	RenderTexture2D tex = LoadRenderTexture(width, height);
	while (!WindowShouldClose()) {

		for (auto& x : charges) {
			if (x.pos.x < 0 || x.pos.x >= width ) {

				x.vel.x = -x.vel.x;

				//x.q = GetRandomValue(-10, 10)/10.0;
				//x.pos.x = static_cast<cl_float>(GetRandomValue(0, width));
				//x.pos.y = static_cast<cl_float>(GetRandomValue(0, height));
				//x.vel.x = static_cast<cl_float>(GetRandomValue(-1000, 1000) / 50.0);
				//x.vel.y = static_cast<cl_float>(GetRandomValue(-1000, 1000) / 50.0);
				
			}
			if (x.pos.y < 0 ||  x.pos.y >= height) {

				x.vel.y = -x.vel.y;

				//x.q = GetRandomValue(-10, 10)/10.0;
				//x.pos.x = static_cast<cl_float>(GetRandomValue(0, width));
				//x.pos.y = static_cast<cl_float>(GetRandomValue(0, height));
				//x.vel.x = static_cast<cl_float>(GetRandomValue(-1000, 1000) / 50.0);
				//x.vel.y = static_cast<cl_float>(GetRandomValue(-1000, 1000) / 50.0);
				
			}

			x.pos.x += x.vel.x * GetFrameTime();
			x.pos.y += x.vel.y * GetFrameTime();


			Point field;
			field.x = 0;
			field.y = 0;
			for (auto& y : charges) {

				Point vr;
				vr.x = x.pos.x - y.pos.x;
				vr.y = x.pos.y - y.pos.y;

				float r = sqrt(vr.x * vr.x + vr.y * vr.y);

				if (r == 0) {
				}
				else if (r < constants[1] * abs(y.q)) {
					float mag = y.q * constants[0] / (r * r * constants[1] * abs(y.q));
					field.x += mag * vr.x;
					field.y += mag * vr.y;
				}

				else {
					float mag = y.q * constants[0] / (r * r * r);
					field.x += mag * vr.x;
					field.y += mag * vr.y;
				}

			}
			x.vel.x += field.x * GetFrameTime();
			x.vel.y += field.y * GetFrameTime();

		}

	
		compute::copy(charges.begin(), charges.end(), device_charges.begin(), queue);
		queue.enqueue_1d_range_kernel(pot_kernel, 0, width* height, 0);
		queue.enqueue_1d_range_kernel(col_kernel, 0, width* height, 0);
		/*
		queue.enqueue_1d_range_kernel(elec_field_kernel, 0, width* height, 0);
		queue.enqueue_1d_range_kernel(pos_kernel, 0, nCharges, 0);*/


		compute::copy(device_board.begin(), device_board.end(), board.begin(), queue);





		BeginTextureMode(tex);
		UpdateTexture(tex.texture, static_cast<void*>(board.data()));
		EndTextureMode();
		BeginDrawing();

		// NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom) BUT Since we're drawing maually , no problem
		DrawTextureRec(tex.texture, Rectangle{ 0, 0, (float)tex.texture.width, (float)tex.texture.height }, Vec2{ 0, 0 }, WHITE);
		//DrawTextureRec(tex.texture, Rectangle{ 0, 0, (float)tex.texture.width, (float)-tex.texture.height }, Vec2{ 0, 0 }, WHITE);


		for (auto& c : charges) {
			DrawCircle(c.pos.x, c.pos.y, abs(c.q * unitR), signbit(c.q) ? BLACK : WHITE);
		}
		
		DrawFPS(30, 30);
		EndDrawing();

	}
	UnloadRenderTexture(tex);



	return 0;
}
