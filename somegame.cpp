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
	Vector2 front = { 1,0 };

	//Returns system's forward direction
	auto getforward = [&front](double angle)->Vector2 {

		//this is a new approach 
		return front;

		//this is old approach
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

	//These are for each tyre, rather than including pesky angular velocities and accln , I decided to do this instead
	Vector2 vel1{ 0,0 };
	Vector2 acc1{ 0,0 };
	Vector2 vel2{ 0,0 };
	Vector2 acc2{ 0,0 };

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
			vel1.x *= 1.5;
			vel1.x *= 1.5;
			vel2.y *= 1.5;
			vel2.y *= 1.5;
		}
		double tanforce = 0;
		if (IsKeyDown(KEY_D))
			tanforce = 1;
		if (IsKeyDown(KEY_A))
			tanforce = -1;
		
		//ball stuff

		//temporary centers and velocities
		Vector2 tmpc = center, tmpv1 = vel1, tmpv2 = vel2;double tmpang = angle;

		//ball centers
		Vector2 cen1 = getball(1);
		Vector2 cen2 = getball(2);


		//Skip rotation calculations , lets do all y linear velocities
		//Vector2 transvel, rotvel;
		//transvel.x = 0.5 * (vel1.x + vel2.x);
		//transvel.y = 0.5 * (vel1.y + vel2.y);
		//
		//rotvel.x = 0.5 * (vel1.x - vel2.x);
		//rotvel.y = 0.5 * (vel1.y - vel2.y);

		//rotation = rvec x velvec
		/*
		float halfdist = Vector2Length({ cen1.x - cen2.x,cen1.y - cen2.y });
		Vector2 halfvec = getforward(angle);
		halfvec.x *= halfdist;
		halfvec.y *= halfdist;

		Vector2 paravel = { 0 };
		paravel.x = (Vector2DotProduct(vel1, halfvec) + Vector2DotProduct(vel2, halfvec)) * getforward(angle).x / halfdist;
		paravel.y = (Vector2DotProduct(vel1, halfvec) + Vector2DotProduct(vel2, halfvec)) * getforward(angle).y / halfdist;

		Vector3 perpvel = { 0 };
		perpvel = Vector3CrossProduct(Vector3{ halfvec.x / halfdist,halfvec.y / halfdist,0 }, Vector3{ vel1.x - vel2.x,vel1.y - vel2.y,0 });
		perpvel = Vector3Scale(perpvel, 1.0f / (2 * halfdist));

		angle += GetFrameTime() * (perpvel.z);

		center.x += paravel.x* GetFrameTime();
		center.y += paravel.y* GetFrameTime();*/

		//trying another approach yet again
		cen1 = getball(1);
		cen2 = getball(2);

		cen1.x += vel1.x * GetFrameTime();
		cen1.y += vel1.y * GetFrameTime();
		
		cen2.x += vel2.x * GetFrameTime();
		cen2.y += vel2.y * GetFrameTime();

		center.x = 0.5 * (cen1.x + cen2.x);
		center.y = 0.5 * (cen1.y + cen2.y);

		front = Vector2Normalize(Vector2{ cen1.x - cen2.x,cen1.y - cen2.y });

		vel1.y += gravity * GetFrameTime();
		vel1.y += acc1.y * GetFrameTime();
		vel1.x += acc1.x * GetFrameTime();
		
		vel2.y += gravity * GetFrameTime();
		vel2.y += acc2.y * GetFrameTime();
		vel2.x += acc2.x * GetFrameTime();


		//check if collide and find if touch or not
		Vector2 pt1a{ -1,-1 };
		Vector2 pt1b{ -1,-1 };
		Vector2 pt2a{ -1,-1 };
		Vector2 pt2b{ -1,-1 };

		bool just1collided = checktouch(cen1,&pt1a, &pt1b);
		bool just2collided = checktouch(cen2,&pt2a, &pt2b);
		if (( just2collided || just1collided) && !collided) {

			//If ballno = 1 then 1 ball else 2 ball
			auto reflectball = [&](int ballno) {
				if ((ballno != 1) && (ballno != 2))
					throw - 1;

				//decide by ballno
				Vector2& pta = ((ballno == 1) ? pt1a : pt2a);
				Vector2& ptb = ((ballno == 1) ? pt1b : pt2b);
				Vector2& vel = ((ballno == 1) ? vel1 : vel2);
				
				//normal pointing towards upwards
				Vector2 normal;
				normal.y = (ptb.x - pta.x) * ((ptb.y > pta.y) ? 1 : -1);
				normal.x = -abs(ptb.y - pta.y);

				normal = Vector2Normalize(normal);

				//normal and tangential components of vel
				Vector2 tancom;
				tancom.x = vel.x - Vector2DotProduct(normal, vel) * normal.x;
				tancom.y = vel.y - Vector2DotProduct(normal, vel) * normal.y;
				//normal component is adjusted to reflections and inelasticity
				Vector2 normcom;
				normcom.x = -Vector2DotProduct(normal, vel) * normal.x * sqrt(elas);
				normcom.y = -Vector2DotProduct(normal, vel) * normal.y * sqrt(elas);

				//now rotate the damn tyre 
				vel.y = normcom.y + tancom.y;
				vel.x = normcom.x + tancom.x;

			}; 
			if (just1collided)
				reflectball(1);
			if (just2collided)
				reflectball(2);
			collided = true;
		}
		else {
			collided = false;
		}
		if (just2collided) {

			//now add tanforce if available but along the forward direction this time , yay
			/*tancom.x += ((tancom.x > 0) ? 1 : ((tancom.x < 0) ? -1 : 0)) * GetFrameTime() * tanforce * 30;
			tancom.y += ((tancom.y > 0) ? 1 : ((tancom.y < 0) ? -1 : 0)) * GetFrameTime() * tanforce * 30;*/

			vel1.x += tanforce * GetFrameTime() * getforward(angle).x;
			vel1.y += tanforce * GetFrameTime() * getforward(angle).y;
			vel2.x += tanforce * GetFrameTime() * getforward(angle).x;
			vel2.y += tanforce * GetFrameTime() * getforward(angle).y;
		}

		//collisions with walls need to account if goes inside the walls too
		if (center.x < radius) {
			center.x = radius;
			vel1.x = -vel1.x;
			vel2.x = -vel2.x;
		}
		if (center.y < radius) {
			center.y = radius;
			vel1.y = -vel1.y;
			vel2.y = -vel2.y;
		}
		if (center.x > width - radius) {
			center.x = width - radius;
			vel1.x = -vel1.x;
			vel2.x = -vel2.x;
		}
		if (center.y > height - radius) {
			center.y = height - radius;
			vel1.y = -vel1.y;
			vel2.y = -vel2.y;
		}


		ClearBackground(SKYBLUE);
		BeginDrawing();

		//DrawLineStrip(hills->getView(), 900, DARKBLUE);
		
		DrawLineStrip(vecs, width, RED);
		for (int i = 0; i < width; ++i) {
			DrawLine(i, height-1, i, vecs[i].y, GREEN);
		}

		Vector2 perp;
		perp.y = -front.x;
		perp.x = front.y;
		if (perp.y > 0) {
			perp.x = -perp.x;
			perp.y = -perp.y;
		}
		DrawTriangle(cen1,
			Vector2{ cen1.x + perp.x * 1.5f * (float)radius,cen1.y + perp.y * 1.5f * (float)radius },
			Vector2{ cen2.x + perp.x * 1.5f * (float)radius,cen2.y + perp.y * 1.5f * (float)radius }, DARKBLUE);
		DrawTriangle(cen2,
			cen1,
			Vector2{ cen2.x + perp.x * 1.5f * (float)radius,cen2.y + perp.y * 1.5f * (float)radius }, DARKBLUE);

		auto getcol = [](bool justcollide)->Color {
			if (justcollide)
				return WHITE;
			else
				return BLACK;
		};

		DrawCircle(getball(1).x, getball(1).y, radius, getcol(just1collided));
		DrawCircle(getball(2).x, getball(2).y, radius * 1.05, getcol(just2collided));
		EndDrawing();


	}
	delete[] vecs;
	return 0;
}