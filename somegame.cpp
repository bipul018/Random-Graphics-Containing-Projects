#include <raylib-cpp.hpp>
#include <random>
#include <chrono>
#include <functional>


class HillGen {
public:

	unsigned full;
	unsigned view;
	long currptr = 0;
	Vector2* arr = nullptr;
	double* seed = nullptr;

	std::mt19937_64& rng;


	double top = 800;
	double bottom = 100;
	double rate = 1.5;

	double mid() {
		return 0.5 * (top + bottom);
	}
	double diff() {
		return top - bottom;
	}

	//double getnext() {
	//		y += y1;
	//	//y += tan(y1);
	//		y1 += y2;
	//		y2 += std::uniform_real_distribution<double>(0, 0.02)(rng) - 0.01;
	//		//y1 += std::uniform_real_distribution<double>(-PI/180, PI/180)(rng);

	//		if (y > mid()) {
	//			//y1 -= 0.1*rate / abs(top + diff() * 0 - y);

	//			double fac = 2 * (top - y) / diff() - 1;
	//			y1 += fac * y1;

	//		}
	//		else if(y<mid()) {
	//			//y1 += 0.1*rate / abs(bottom - diff() * 0 - y);
	//			double fac = 2 * (y - bottom) / diff() - 1;

	//			y1 += fac * y1;
	//		}
	//	
	//return y;
	//}

	//end exclusive
	void fill(unsigned start, unsigned end) {

		unsigned y = start;
		if (start > end) {
			start = end;
			end = y;
		}

		//fill seed
		//clear field
		for (int i = start; i < end; ++i) {
			seed[i] = std::uniform_real_distribution<double>(0, 1)(rng);	
			arr[i].x = i;
			arr[i].y = 0;
		}


		//now fill octaves
		int o = 900;
		//factor for each octave stages divided by rate
		double fac = 1.0;
		//sum of fac for mapping back
		double facsum = 0;
		//for each octave
		for (int octaves=0; octaves <o; ++octaves) {

			//goes from lowest freq / highest range to highest freq / lowest range
			//for each sement in each ocatave
			for (float xx = start; xx < end; xx += view * fac) {

				int x1 = xx;
				int x2 = (int)(xx + view * fac) % full;
				//delx = view/octaves


				//for each element in each octave
				for (int i = xx; i < (xx + view * fac) && i < end; ++i) {

					double y = seed[x1] + (i - x1) * (seed[x2] - seed[x1]) /( fac * view);
					arr[i].y += fac * y;

				}

			}
			facsum += fac;
			fac /= rate;
			
		}

		//Now mapping back to desired range
		for (int i = start; i < end; ++i) {
			//mapping to 0 1
			arr[i].y /= facsum;
			//mapping to top bottom
			arr[i].y = bottom * (1 - arr[i].y) + top * arr[i].y;
		}

		
	}

	HillGen(std::mt19937_64& rngfunc,unsigned maxSize, unsigned viewSize,
		double upmost=800,double downmost=100,double pushfac = 0.1 ):

		rng(rngfunc),top(upmost),bottom(downmost),rate(pushfac) {

		if (maxSize > viewSize) {
			full = maxSize;
			view = viewSize;
		}
		else {
			full = viewSize;
			view = maxSize;
		}

		arr = new Vector2[full];
		seed = new double[full];
		
		if (upmost < downmost) {
			top = downmost;
			bottom = upmost;
		}

		fill(0, full);


	}

	void shift(int n) {
		
		/*
		currptr += n;
		if (currptr < 0)
			currptr = 0;
		if (currptr > (full - view)) {
			if (currptr >= full) {

				for (int i = 0; i < full; ++i) {
					arr[i].y = getnext();
				}
				currptr = 0;
			}
			else {
				memmove(arr, arr + currptr, (full - currptr) * sizeof(Vector2));
				for (int i = currptr; i < full; ++i) {
					arr[i].y = getnext();
				}
				currptr = 0;
			}
		}*/
	}

	Vector2* getView() {
		for (int i = currptr; i < currptr + view; ++i) {
			arr[i].x = i-currptr;
		}
		return arr + currptr;
	}

	~HillGen() {
		delete[] arr;
		delete[] seed;
	}


};

int coaster() {
	std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
	
	unsigned height = 900;
	unsigned width = 1200;
	InitWindow(width, height, "up and down");
	SetTargetFPS(60);

	
	//HillGen *hills =new HillGen(rng, 900, 901);

	float accum = 0;


	float amps[20];
	Vector2 *vecs = new Vector2[width];
	auto sinehill = [&]() {

		float sumamp = 0;
		float topamp = 1.0;

		for (int i = 1; i <= sizeof(amps) / sizeof(amps[0]); i++) {
			topamp = std::uniform_real_distribution<double>(0.9 / exp(i*0.1), 1.0 / exp(i*0.1))(rng);
			amps[i - 1] = std::uniform_real_distribution<double>(0, topamp)(rng);
			//topamp = 1.0 / (i + 1);
			sumamp += amps[i - 1];
		}
		float offset = std::uniform_real_distribution<double>(0, 2 * PI)(rng);
		for (int i = 0; i < width; ++i) {
			vecs[i].x = i;
			vecs[i].y = 0;
			for (int j = 1; j <= sizeof(amps) / sizeof(amps[0]); ++j) {
				vecs[i].y += amps[j - 1] * cos(offset + i * j * PI / (2.0 * width))  / sumamp;
			}
		}
		for (int i = 0; i < width; ++i)
			vecs[i].y = height / 3 + height * (vecs[i].y+1) / 6;

	};

	sinehill();

	double radius = 30;
	
	//Now two balls , center reprenents center of gravity, separaed by radius distance between edges
	Vector2 center = { radius,radius };
	//angle of rotation in radians
	double angle = 0;
	double anglev = 0;

	//Returns system's forward direction
	auto getforward = [](double angle)->Vector2 {
		Vector2 vec = { 0 };
		vec.x = cos(angle);
		vec.y = sin(angle);
		return vec;
	};

	//returns ball center 1 input gives forward ball, 2 gives backward ball
	auto getball = [&radius, &center, &angle,&getforward](int n)->Vector2 {

		Vector2 cen = center;
		//first get line vector
		Vector2 vec = getforward(angle);
		//then get ball
		if (n == 1) {
			cen.x += vec.x * 1.5 * radius;
			cen.y += vec.y * 1.5 * radius;
		}
		else if (n == 2) {
			cen.x -= vec.x * 1.5 * radius;
			cen.y -= vec.y * 1.5 * radius;
			
		}
		else {
			return Vector2{ -1. - 1 };
		}
		return cen;
	};

	auto checktouch = [&vecs,&radius](Vector2 center,Vector2* pt1 = nullptr, Vector2* pt2 = nullptr)->bool {

		bool collide = false;

		for (double x = center.x - radius; x <= center.x + radius; x+=0.5) {

			if (CheckCollisionPointCircle(vecs[int(x)], center, radius)) {
				if (!collide) {
					collide = true;
					if (pt1 != nullptr) {
						*pt1 = vecs[int(x)];
						pt1->x = x;
					}
					
				}
				else {
					if (pt2 != nullptr) {
						*pt2 = vecs[int(x)];
						pt2->x = x;
					}
				}

			}

		}

		//case when ball is submerged inside
		if ((center.y - radius) > vecs[int(center.x)].y)
			collide = true;

		return collide;
	};

	float gravity = +20;
	Vector2 vel{ 0,0 };
	Vector2 acc{ 0,0 };

	//elasticity factor
	double elas = 0.1;

	//when ball has just collided
	bool collided = false;

	while (!WindowShouldClose()) {
		if (IsKeyDown(KEY_RIGHT))
			center.x += 2.5;
		if (IsKeyDown(KEY_LEFT))
			center.x -= 2.5;
		if (IsKeyDown(KEY_UP)) 
			center.y -= 2.5;
		if (IsKeyDown(KEY_DOWN))
			center.y += 2.5;

		if (IsKeyReleased(KEY_SPACE)) {
			//delete hills;
			//hills = new HillGen(rng, 1800, 900);
			sinehill();
		}

		if (IsKeyReleased(KEY_ENTER)) {
			vel.x *= 1.5;
			vel.y *= 1.5;
		}
		double tanforce = 0;
		if (IsKeyDown(KEY_D))
			tanforce = 1;
		if (IsKeyDown(KEY_A))
			tanforce = -1;
		
		//ball stuff

		//temporary centers and velocities
		Vector2 tmpc = center, tmpv = vel;

		center.x += vel.x* GetFrameTime();
		center.y += vel.y* GetFrameTime();

		vel.y += gravity * GetFrameTime();
		vel.y += acc.y * GetFrameTime();
		vel.x += acc.x * GetFrameTime();


		//check if collide and find if touch or not
		Vector2 pta{ -1,-1 };
		Vector2 ptb{ -1,-1 };
		bool justcollided = checktouch(&pta, &ptb);
		auto tmpcol = justcollided ? BLUE : YELLOW;
		if (justcollided && !collided) {

			//normal pointing towards upwards
			Vector2 normal;
			normal.y = (ptb.x - pta.x) * ((ptb.y > pta.y) ? 1 : -1);
			normal.x = -abs(ptb.y - pta.y);

			normal = Vector2Normalize(normal);

			//normal and tangential components of vel
			Vector2 tancom;
			tancom.x = vel.x - Vector2DotProduct(normal, vel) * normal.x;
			tancom.y = vel.y - Vector2DotProduct(normal, vel) * normal.y;
			//now add tanforce if available
			tancom.x += ((tancom.x > 0) ? 1 : ((tancom.x < 0) ? -1 : 0)) * GetFrameTime() * tanforce*30;
			tancom.y += ((tancom.y > 0) ? 1 : ((tancom.y < 0) ? -1 : 0)) * GetFrameTime() * tanforce*30;

			//normal component is adjusted to reflections and inelasticity
			Vector2 normcom;
			normcom.x = -Vector2DotProduct(normal, vel) * normal.x * sqrt(elas);
			normcom.y = -Vector2DotProduct(normal, vel) * normal.y * sqrt(elas);


			vel.y = normcom.y + tancom.y;
			vel.x = normcom.x + tancom.x;
			
			

			collided = true;
		}
		else {
			collided = false;
			acc.x = 0;
			acc.y = 0;
		}
		//collisions with walls need to account if goes inside the walls too
		if (center.x < radius) {
			center.x = radius;
			vel.x = -vel.x;
		}
		if (center.y < radius) {
			center.y = radius;
			vel.y = -vel.y;
		}
		if (center.x > width - radius) {
			center.x = width - radius;
			vel.x = -vel.x;
		}
		if (center.y > height - radius) {
			center.y = height - radius;
			vel.y = -vel.y;
		}


		ClearBackground(SKYBLUE);
		BeginDrawing();

		//DrawLineStrip(hills->getView(), 900, DARKBLUE);
		
		DrawLineStrip(vecs, width, RED);
		for (int i = 0; i < width; ++i) {
			DrawLine(i, height-1, i, vecs[i].y, GREEN);
		}
		DrawCircle(center.x, center.y, radius, tmpcol);
		EndDrawing();


	}
	delete[] vecs;
	return 0;
}