#ifndef RAY_H
#define RAY_H
#include "Vec.h"
#include "Sphere.h"

class Ray {
public:
	Vec p0, v;
	Ray(Vec p0_, Vec v_) : p0(p0_), v(v_) {}

	double intersect(const Sphere s) {	//return (t) > 0 : intersect
		double b = 2 * v.dot(p0 - s.C);
		double c = (p0 - s.C).dot(p0 - s.C) - s.r * s.r;

		double check = b * b - 4 * c;

		if (check <= 0)
			return 0;
		else
			return (-b - sqrt(check)) / 2;
	}
};
#endif