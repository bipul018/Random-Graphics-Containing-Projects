

typedef struct {

	float x;
	float y;
} Point; 
typedef struct {
	float q;
	Point pos;
	Point vel;
} Charge;

typedef struct {

	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
}MyColor;

float my_abs(float num) {
	return (num > 0) ? num : -num;
}

float get_len(Point vec){
	return sqrt(vec.x*vec.x+vec.y*vec.y);
}

Point get_unit(Point vec){
	Point p;
	p.x=0;
	p.y=0;
	float r = get_len(vec);
	if(r!=0){
		p.x=vec.x/r;
		p.y=vec.y/r;
	}
	return p;
}

kernel void calc_pot(global Point* points, global Charge* charges, global float* results, global float* constants, int nCharges, float delTime) {
	
		size_t index = get_global_id(0);
	results[index] = 0;
	
		for (size_t i = 0; i < nCharges; ++i) {
			
			Point p2 = points[index];
			
			Point delr;
			delr.x = charges[i].vel.x * delTime;
			delr.y = charges[i].vel.y * delTime;

			Point p1;
			p1.x = p2.x - delr.x;
			p1.y = p2.y - delr.y;

			Point p = p1;

			Point dr;
			dr.x = p.x - p1.x;
			dr.y = p.y - p1.y;
			
			Point unit = get_unit(delr);
			float alpha = charges[i].q*2/ (get_len(delr) * get_len(delr));
			while(get_len(dr)<=get_len(delr)){

				float q = alpha * get_len(dr) * 0.01;

				p.x = p1.x + dr.x; 
				p.y = p1.y + dr.y; 
				Point tmp;
				tmp.x = points[index].x- p.x;
				tmp.y = points[index].y-p.y;
				float r = get_len( tmp );
				
				if (r < constants[1] * my_abs(q)) {
					results[index] += q * constants[0] / (constants[1] * my_abs(q));
				}
				
				else {
					results[index] += q * constants[0] / r;
				}

				dr.x += unit.x * 0.01;
				dr.y += unit.y * 0.01;
			}

			
			
		}
}

kernel void calc_elec_field(global Point* points, global Charge* charges, global Point* elec_field, global float* constants, int nCharges) {

	size_t index = get_global_id(0);
	elec_field[index].x = 0;
	elec_field[index].y = 0;

	for (size_t i = 0; i < nCharges; ++i) {

		Point vr;
		vr.x = points[index].x - charges[i].pos.x;
		vr.y = points[index].y - charges[i].pos.y;

		float r = sqrt(vr.x * vr.x + vr.y * vr.y);

		if (r == 0) {
		}

		else if (r < constants[1] * my_abs(charges[i].q)) {
			float rr = constants[1] * my_abs(charges[i].q);
			float mag = charges[i].q * constants[0] / (rr * rr * rr);
			elec_field[index].x += mag * vr.x * rr / r;
			elec_field[index].y += mag * vr.y * rr / r;

		}

		else {
			float mag = charges[i].q * constants[0] / (r * r * r);
			elec_field[index].x += mag * vr.x;
			elec_field[index].y += mag * vr.y;
		}

	}
}

kernel void update_pos_vel(global Charge* charges, global Point* elec_field, float delTime) {
	size_t index = get_global_id(0);
	charges[index].pos.x += charges[index].vel.x * delTime;
	charges[index].pos.y += charges[index].vel.y * delTime;
	charges[index].vel.x += charges[index].q * elec_field[index].x * delTime;
	charges[index].vel.y += charges[index].q * elec_field[index].y * delTime;
}

kernel void calc_charge_field(global Charge* charges, global Point* elec_field, global float* constants, int nCharges) {
	size_t index = get_global_id(0);
	elec_field[index].x = 0;
	elec_field[index].y = 0;


	for (int i = 0; i < nCharges; ++i) {
		Point vr;
		vr.x = charges[index].pos.x - charges[i].pos.x;
		vr.y = charges[index].pos.y - charges[i].pos.y;

		float r = sqrt(vr.x * vr.x + vr.y * vr.y);

		if (r == 0) {
		}

		else if (r <= constants[1] * my_abs(charges[i].q)) {
			float rr = constants[1] * my_abs(charges[i].q);
			float mag = charges[i].q * constants[0] / (rr * rr * rr);
			elec_field[index].x += mag * vr.x * rr / r;
			elec_field[index].y += mag * vr.y * rr / r;

		}

		else {
			float mag = charges[i].q * constants[0] / (r * r * r);
			elec_field[index].x += mag * vr.x;
			elec_field[index].y += mag * vr.y;
		}


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
}

