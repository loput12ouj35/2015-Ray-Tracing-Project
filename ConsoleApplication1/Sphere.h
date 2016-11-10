#ifndef SPHERE_H
#define SPHERE_H
#include "Vec.h"

//#define DIFFERENT_SPEED


class Sphere {
public:
	const double SPEED_SCALE = 0.25;

	double speed;
	double r;       // radius 
	Vec C, direction, color;      // position, moving direction, color 
	bool isTrans, isReflec, isTransparent, isPhysical;
	Sphere(double r_, Vec C_, Vec direction_, Vec color_, bool isTrans_ = true, bool isReflec_ = true, double speed_ = 50, bool isTransparent_ = false, bool isPhysical_ = false) :
		r(r_), C(C_), direction(direction_.norm()), color(color_), isTrans(isTrans_), isReflec(isReflec_), speed(speed_), isTransparent(isTransparent_), isPhysical(isPhysical_){}

	void move(){
		if (isPhysical){
			
			//gravity
			if (C.y > 50){		//in the air
				if (direction.y > 0){		//going up
					speed -= 10 * SPEED_SCALE;
					if (speed <= 10 * SPEED_SCALE){		//reach peak
						direction.y = -direction.y;
					}
				}
				else{		//going down
					speed += 10 * SPEED_SCALE;
					direction.y = -1;
					direction.norm();
				}

			}
			else if (direction.y != 0){		//hit the ground
				//speed -= 15;
				if (speed < 20 * SPEED_SCALE){	//hit not strongly
					C.y = 50;
					direction.y = 0;
					direction.norm();
					speed = 20 * SPEED_SCALE;
				}
				else{
					direction.y = -direction.y;
				}
			}
			else if (speed <= 0){
				speed = 0;
				direction = Vec(0, 0, 0);
			}

			//friction
			speed -= 5 * SPEED_SCALE;
		}

		if (speed > 0){
			C = C + direction * speed * SPEED_SCALE;

			C.y = max(C.y, 50);		//floor : 50
		}
	}

	bool checkConflict(Sphere *s, bool isInner){
		Vec tmp = s->C - C;
		double sqr_d = tmp.dot(tmp);
		Vec touch_point;
		Vec N;
		Vec R;

		if ((s->r - r) * (s->r - r) - sqr_d > -0.000001)		//s involves this or this involves s
			return false;
		else if ((s->r + r) * (s->r + r) - sqr_d < 0.000001)	//s is too far from this
			return false;

		else if (isInner){	//inner conflict  //&& (s->r - r) * (s->r - r) - sqr_d < 0.1
			touch_point = s->r > r ? C + ((s->C - C) * (r / (s->r - r))) : s->C + ((C - s->C) * (s->r / (r - s->r)));

			N = touch_point - s->C;
			N.norm();
			R = direction - (N * 2 * (direction.dot(N)));
			R = R.norm();
			
			direction = R;
			
			C = C - N * r;
			C.y = max(C.y, 50);
		}

		else{		//outer cinflict
			touch_point = C + ((s->C - C) * (r / (r + s->r)));

			N = touch_point - s->C;
			N.norm();
			if (direction.x == 0 && direction.y == 0 && direction.z == 0){		//if this has stopped
				R = N;
				speed = s->speed - 5 * SPEED_SCALE;
			}
			else{
				R = direction - (N * 2 * (direction.dot(N)));
				R = R.norm();
			}

#ifdef DIFFERENT_SPEED
			if (s->r < 100){		//except greatest ball
				double energy = speed + s->speed;
				speed = energy / 2;
				s->speed = energy / 2;
			}
#endif

			direction = R;

			C = C + N * r;
			if (!isPhysical){	//outer ball
				double r2_r1 = (300 - r) * (300 - r);
				Vec d = Vec(650, 350, 50) - C;
				double sqr_d = d.dot(d);

				if (sqr_d > r2_r1)
					C = C + d.norm() * r;
			}

			C.y = max(C.y, 50);
		}

		return true;
		
	}
};
#endif