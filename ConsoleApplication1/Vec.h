
#ifndef VEC_H
#define VEC_H
#include <math.h>

class Vec {
public:
	double x, y, z;                  // position, also color (r,g,b) 
	Vec(double x_ = 0, double y_ = 0, double z_ = 0){ x = x_; y = y_; z = z_; }
	Vec operator+(Vec b) { return Vec(x + b.x, y + b.y, z + b.z); }
	Vec operator-(Vec b) { return Vec(x - b.x, y - b.y, z - b.z); }
	Vec operator*(double b) { return Vec(x * b, y * b, z * b); }
	Vec mult(Vec b) { return Vec(x * b.x, y * b.y, z * b.z); }
	Vec& norm(){ 
		if (x == 0 && y == 0 && z == 0)
			return Vec(0, 0, 0);
		else
			return *this = *this * (1 / sqrt(x*x + y*y + z*z));
	}
	double dot(Vec b) { return x*b.x + y*b.y + z*b.z; }
	Vec operator%(Vec b){ return Vec(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x); }
};
#endif