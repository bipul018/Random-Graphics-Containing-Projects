#include <raylib-cpp.hpp>
#include <random>
#include <chrono>
#include <functional>



int coaster() {
	std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
	
	unsigned height = 900;
	unsigned width = 1200;
	InitWindow(width, height, "up and down");
	SetTargetFPS(60);

	
	float accum = 0;

	//Array for amplitudes
	float amps[20];
	//Array for those height points
	Vector2 *vecs = new Vector2[width];

	//Hill generator simple lambda fxn
	auto sinehill = [&]() {

		float sumamp = 0;
		float topamp = 1.0;

		//generates random decreasing amplitudes
		for (int i = 1; i <= sizeof(amps) / sizeof(amps[0]); i++) {
			//maximum amplitude for current iteration , by adjusting factor in exp high frequencies can be made more/less prominent
			topamp = std::uniform_real_distribution<double>(0.9 / exp(i*0.1), 1.0 / exp(i*0.1))(rng);
			amps[i - 1] = std::uniform_real_distribution<double>(0, topamp)(rng);
			//topamp = 1.0 / (i + 1);
			sumamp += amps[i - 1];
		}
		//Initial random phase difference
		float offset = std::uniform_real_distribution<double>(0, 2 * PI)(rng);
		//Summing up of half range fourier series for all availabe amplitudes
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

	//calling the function
	sinehill();

	double radius = 30;
	
	//Now two balls , center reprenents center of gravity, separaed by radius distance between edges
	Vector2 center = { radius,radius };
	//angle of rotation in radians
	double angle = 0;
	double anglev = 0;
	Vector2 front = { 1,0 };

	float bonusTime = 0.0f;
	bool justaddedTime = false;
	auto getdeltime = [&bonusTime,&justaddedTime]()->float {
		return GetFrameTime();

	};

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
			tanforce = 10;
		if (IsKeyDown(KEY_A))
			tanforce = -10;
		
		//ball stuff

		//temporary centers and velocities
		Vector2 tmpc = center, tmpv1 = vel1, tmpv2 = vel2;double tmpang = angle;

		//ball centers
		Vector2 cen1 = getball(1);
		Vector2 cen2 = getball(2);

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