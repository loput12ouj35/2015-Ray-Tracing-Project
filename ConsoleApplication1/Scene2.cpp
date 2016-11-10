#include <iostream>
#include <Windows.h>
#include <gl/glut.h>
#include <math.h>
#include <omp.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include "Vec.h"
#include "Ray.h"
#include "Sphere.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MODE_CAPTURE

#define CAPTURE_W 1300
#define CAPTURE_H 700
#define BALL_SIZE 25
#define BALL_SPACE 80

using namespace std;

int W = CAPTURE_W;
int H = CAPTURE_H;
int DEPTH = 3;
int time = 0;
int num_innerBall = 0;

const double effect_r = 0.5;
const double effect_t = 0.6;

const Sphere s_floor = Sphere(1e5, Vec(W / 2, -1e5, 300), Vec(0, 0, 0), Vec(.6, .6, .6), false, true, 0);

static int submenu_depth;
static int submenu_resolution;
static int submenu_viewer;
static int mainmenu;


vector<Vec> Vertices;
vector<Sphere> sphere;


class Face {
public:
	int f1;
	int f2;
	int f3;
	Vec color;
	bool isTrans, isReflec;
	Face(int f1_ = 0, int f2_ = 0, int f3_ = 0, Vec color_ = Vec(), bool isTrans_ = true, bool isReflec_ = true){
		f1 = f1_; f2 = f2_; f3 = f3_; color = color_; isTrans = isTrans_; isReflec = isReflec_;
	}

	Vec getNormalV(){
		Vec v12 = Vertices[f2] - Vertices[f1];
		Vec v13 = Vertices[f3] - Vertices[f1];
		return (v13 % v12).norm();
	}

	double intersect(Ray ray){
		Vec u = Vertices[f2] - Vertices[f1];
		Vec v = Vertices[f3] - Vertices[f1];
		Vec N = (u % v).norm();
		double d = -Vertices[f1].dot(N);
		double t = -(ray.p0.dot(N) + d) / (ray.v.dot(N));
		Vec w = ray.p0 + (ray.v * t) - Vertices[f1];

		double gamma = ((u % w).dot(N)) / N.dot(N);
		double beta = ((w % v).dot(N)) / N.dot(N);
		double alpha = 1 - gamma - beta;

		if (alpha < 0 || alpha > 1 || beta < 0 || beta > 1 || gamma < 0 || gamma >1)
			return 0;
		else
			return t;
	}
};

vector<Face> Faces;

bool lightAll_on = true;
bool light1_on = false && lightAll_on;
bool light2_on = true && lightAll_on;
bool light3_on = true && lightAll_on;

Vec light1_position = Vec(W + 100, H, 300);
Vec light1_direction = Vec(0, 0, 500) - light1_position;	//Light1 : p0 : light1_position v : light1_direction (Spot Light)
Vec light2_position = Vec(-2 * W + 1200, 2 * H, 100);		//Light2 : from light2_position to Origin (Directional Light)
Vec light3_position = Vec(W + 100, H, 300);	//Light3 : p0 : light3_position v : light1_direction (Point Light)
Vec viewer = Vec(CAPTURE_W / 2, CAPTURE_H / 2, -1000);


unsigned char color_capture[CAPTURE_H][CAPTURE_W][3];
Vec color_result[CAPTURE_W][CAPTURE_H];

Vec color_back = Vec(0.15, 0.15, 0.15);

//bool pattern1 = true;
//bool pattern2 = true;
bool pattern3 = true;

class RayTracing{
public:
	Vec color;
	Vec touchPoint;
	bool transparent;

	RayTracing *transmission, *reflection;

	RayTracing(Vec touchPoint_){
		color = color_back;
		touchPoint = touchPoint_;
		bool transparent = false;
	}

	RayTracing(Vec color_, Vec touchPoint_){
		color = color_;
		touchPoint = touchPoint_;
		bool transparent = false;
	}

	RayTracing(RayTracing const &) = delete;
	~RayTracing(){
		delete transmission;
		delete reflection;
	}

};

void makeSphere(){
	const int INIT_W = CAPTURE_W / 2 - 160;
	const int INIT_H = CAPTURE_H / 2 - 160;

	//greatest ball
//	sphere.push_back(Sphere(300, Vec(CAPTURE_W / 2, CAPTURE_H / 2, 50), Vec(0, 0, 0), Vec(.14, .16, .14), true, true, 0, true));

	//50 inner balls
#if 0
	for (int i = 0; i < 5; ++i){
		for (int j = 0; j < 5; ++j){
			for (int k = 0; k < 2; ++k){
				Vec color = i % 2 ? (j % 2 ? Vec(1, 1, 0) : Vec(1, 0, 0)) : (j % 2 ? Vec(0, 1, 0) : Vec(0, 0, 1));
				sphere.push_back(Sphere(BALL_SIZE, Vec(INIT_W + i * BALL_SPACE, INIT_H + j * BALL_SPACE, -40 + k * BALL_SPACE), Vec(0, -1, i % 2), color));
				++num_innerBall;
			}
		}
	}
#endif
#if 0
	//20 outer balls
	for (int k = 0; k < 500; k += 150){
		for (int j = 300; j <= 400; j += 100){
			sphere.push_back(Sphere(BALL_SIZE, Vec(j, 50, k), Vec(1, 0, 0), pattern3 ? Vec(.7, .8, .7) : Vec(.1, .1, .1), true, true, 20, false, true));
			sphere.push_back(Sphere(BALL_SIZE, Vec(j + 600, 50, k), Vec(1, 0, 0), pattern3 ? Vec(.7, .8, .7) : Vec(.1, .1, .1), true, true, 20, false, true));
			pattern3 = !pattern3;
		}
		pattern3 = !pattern3;
	}
#endif
#if 0
	//conflict test balls
	sphere.push_back(Sphere(25, Vec(100, 50, 50), Vec(0, 0, 0), Vec(.7, .8, .7), true, true, 0, false, true));
	sphere.push_back(Sphere(25, Vec(200, 50, 50), Vec(-1, 0, 0), Vec(.7, .8, .7), true, true, 50, false, true));
#endif

	//floor
	sphere.push_back(s_floor);

}
void makePolygon(){
	Vertices.push_back(Vec(50, 20, 200));
	Vertices.push_back(Vec(100, 20, 200));
	Vertices.push_back(Vec(75, 20, 150));
	Vertices.push_back(Vec(75, 70, 150));

	Faces.push_back(Face(0, 1, 3, Vec(.5, 0, 0), true, true));	//back
	Faces.push_back(Face(0, 2, 1, Vec(.5, .5, 0), true, true));	//floor
	Faces.push_back(Face(2, 3, 0, Vec(0, 0, .5), true, true));	//left
	Faces.push_back(Face(3, 2, 1, Vec(0, .5, .5), true, true));	//right

}
bool meetLight1(Vec touchPoint, double cosine_){
	Vec direction = (light1_position - touchPoint).norm();
	touchPoint = touchPoint - (Vec(0.01, 0.01, 0.01).mult(direction));
	Ray ray = Ray(touchPoint, direction);
	for (int k = 0; k < sphere.size(); ++k){
		if (ray.intersect(sphere[k]) > 0){
			return false;
		}
	}
	double cosine = light1_direction.norm().dot(direction * -1);

	return cosine >= cosine_;
}
bool meetLight2(Vec touchPoint){
	Vec direction = light2_position.norm();
	touchPoint = touchPoint - (Vec(0.01, 0.01, 0.01).mult(direction));
	Ray ray = Ray(touchPoint, direction);
	for (int k = 0; k < sphere.size(); ++k){
		if (ray.intersect(sphere[k]) > 0){
			return false;
		}
	}

	return true;
}
bool meetLight3(Vec touchPoint){
	Vec direction = (light3_position - touchPoint).norm();
	touchPoint = touchPoint - (Vec(0.01, 0.01, 0.01).mult(direction));
	Ray ray = Ray(touchPoint, direction);
	for (int k = 0; k < sphere.size(); ++k){
		double t = ray.intersect(sphere[k]);
		if (t > 0 && t < 1.2){
			return false;
		}
	}
	return true;
}

void postorder(RayTracing *root) {
	if (root != NULL) {
		postorder(root->transmission);
		postorder(root->reflection);

		double effect_t_ = root->transparent ? 0.9 : effect_t;

		if (root->transmission != NULL)
			root->color = root->color + root->transmission->color * effect_t_;
		else
			root->color = root->color + color_back * effect_t_;
		if (root->reflection != NULL)
			root->color = root->color + root->reflection->color * effect_r;
		else
			root->color = root->color + color_back * effect_r;

		//Light1 : spot light
		if (light1_on){
			if (meetLight1(root->touchPoint, 0.5)){		//120 degree
				Vec between1 = light1_position - root->touchPoint;
				double sqr_d = between1.dot(between1) / 922500;
				double r = 1 / sqr_d;

				root->color = root->color * max(0.3, r);
			}
			else{
				root->color = root->color * 0.3;
			}
		}
		//Light2 : directional light
		if (light2_on){
			if (!meetLight2(root->touchPoint))
				root->color = root->color * 0.7;
		}
		//Light3 : point light
		if (light3_on){
			Vec between3 = light3_position - root->touchPoint;
			double sqr_d = between3.dot(between3) / 1000000;
			double r = min(1 / sqrt(sqr_d), 1.1);

			root->color = root->color * max(0.1, r);
			if (!meetLight3(root->touchPoint)){
				root->color = root->color * 0.4;
			}
		}

	}
}

void makingTree(RayTracing *root, Ray ray, int depth, int transException){
	Vec touch_point;
	Vec N;
	Vec R;
	bool isReflec = false;
	bool isTrans = false;
	int current_Exception;	//index of exception (-1:  no one)

	if (root == NULL)
		root = new RayTracing(ray.p0);
	if (depth < DEPTH){
		double t;
#if 0
		double min_t = 1e5;
		Vec min_R;
		for (int p = 0; p < Faces.size(); ++p){
			if (p == transException)
				continue;

			t = Faces[p].intersect(ray);
			if (t > 0){		//if intersect && is not the exception
				if (t < min_t){
					min_t = t;
					min_R = R;
				}
				else
					continue;

				root->color = Faces[p].color;


				//reflection
				touch_point = ray.p0 + ray.v * t;
				root->touchPoint = touch_point;
				N = Faces[p].getNormalV();
				R = ray.v - (N * 2 * (ray.v.dot(N)));
				R = R.norm();

				root->reflection = new RayTracing(touch_point);
				isReflec = Faces[p].isReflec;

				//transmission
				root->transmission = new RayTracing(touch_point);
				isTrans = Faces[p].isTrans;
				current_Exception = p;
			}
		}
		if (isReflec){
			Vec min_touch_point = ray.p0 + ray.v * min_t;

			makingTree(root->reflection, Ray(min_touch_point + min_R * 0.01, min_R), depth + 1, -1);
		}
		if (isTrans)
			makingTree(root->transmission, touch_point - N * 0.5, depth + 1, current_Exception);

#else
		int min_k = -1;
		double min_t = 1e7;

		for (int k = 0; k < sphere.size(); ++k){
			if (transException == k)
				continue;

			t = ray.intersect(sphere[k]);

			if (t > 0 && min_t > t){
				min_t = t;
				min_k = k;
			}
		}

		if (min_k != -1){		//if intersect && is not the exception
			root->color = sphere[min_k].color;
			root->transparent = sphere[min_k].isTransparent;

			//reflection
			touch_point = ray.p0 + ray.v * min_t;
			root->touchPoint = touch_point;
			N = touch_point - sphere[min_k].C;
			N = N.norm();
			R = ray.v - (N * 2 * (ray.v.dot(N)));
			R = R.norm();

			root->reflection = new RayTracing(touch_point);
			isReflec = sphere[min_k].isReflec;

			//transmission
			root->transmission = new RayTracing(touch_point);
			isTrans = sphere[min_k].isTrans;
			current_Exception = min_k;
		}
		if (isReflec)
			makingTree(root->reflection, Ray(touch_point, R), depth + 1, -1);
		if (isTrans)
			makingTree(root->transmission, Ray(touch_point - N * 0.7, ray.v), depth + 1, current_Exception);
#endif
	}
}

void cal_ray(){
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < W; ++i){
		for (int j = 0; j < H; ++j){
			RayTracing* raytracing_start = new RayTracing(viewer);

			makingTree(raytracing_start, Ray(viewer, (Vec(i, j, 0) - viewer).norm()), 0, -1);
			postorder(raytracing_start);			//set color
			color_result[i][j] = raytracing_start->color;
			delete raytracing_start;

#ifdef MODE_CAPTURE
			double r = min(max(0, color_result[i][j].x), 1);
			double g = min(max(0, color_result[i][j].y), 1);
			double b = min(max(0, color_result[i][j].z), 1);

			color_capture[CAPTURE_H - 1 - j][i][0] = (unsigned char)(r * (double)255);
			color_capture[CAPTURE_H - 1 - j][i][1] = (unsigned char)(g * (double)255);
			color_capture[CAPTURE_H - 1 - j][i][2] = (unsigned char)(b * (double)255);
#endif
		}
	}

#ifdef MODE_CAPTURE
	string filename = "l" + to_string(time) + ".png";
	stbi_write_png(filename.c_str(), W, H, 3, color_capture, 0);
	cout << time << " : " << filename << endl;
#endif
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	glPointSize(3);
	glBegin(GL_POINTS);
	for (int i = 0; i < W; ++i){
		for (int j = 0; j < H; ++j){
			glColor3f(color_result[i][j].x, color_result[i][j].y, color_result[i][j].z);
			glVertex2i(i, j);
		}
	}
	glEnd();

	glutSwapBuffers();
}

void reshape(int w, int h){
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluOrtho2D(0, W, 0, H);
}

void menu(int val){
	switch (val){
	case 1:
		cout << "Depth : 5" << endl;
		DEPTH = 5;
		cal_ray();
		cout << "redisplayed" << endl;
		glutPostRedisplay();
		break;
	case 2:
		cout << "Depth : 4" << endl;
		DEPTH = 4;
		cal_ray();
		cout << "redisplayed" << endl;
		glutPostRedisplay();
		break;
	case 3:
		cout << "Depth : 3" << endl;
		DEPTH = 3;
		cal_ray();
		cout << "redisplayed" << endl;
		glutPostRedisplay();
		break;
	case 4:
		cout << "Depth : 2" << endl;
		DEPTH = 2;
		cal_ray();
		cout << "redisplayed" << endl;
		glutPostRedisplay();
		break;
	case 5:
		cout << "Depth : 1" << endl;
		DEPTH = 1;
		cal_ray();
		cout << "redisplayed" << endl;
		glutPostRedisplay();
		break;
	case 6:
		cout << "Resolution : 500 X 500" << endl;
		W = 500;
		H = 500;
		cal_ray();
		cout << "redisplayed" << endl;
		glutReshapeWindow(W, H);
		glutPostRedisplay();
		break;
	case 7:
		cout << "Resolution : 700 X 700" << endl;
		W = 700;
		H = 700;
		cal_ray();
		cout << "redisplayed" << endl;
		glutReshapeWindow(W, H);
		glutPostRedisplay();
		break;
	case 10:
		cout << "View Point : 250, 250, -1000" << endl;
		viewer = Vec(250, 250, -1000);
		cal_ray();
		cout << "redisplayed" << endl;
		glutPostRedisplay();
		break;
	case 11:
		cout << "View Point : 0, 250, -1000" << endl;
		viewer = Vec(0, 250, -1000);
		cal_ray();
		cout << "redisplayed" << endl;
		glutPostRedisplay();
		break;
	case 12:
		cout << "View Point : 250, 500, -1000" << endl;
		viewer = Vec(250, 500, -1000);
		cal_ray();
		cout << "redisplayed" << endl;
		glutPostRedisplay();
		break;

	}
}

void setupMenus()
{
	submenu_depth = glutCreateMenu(menu);
	glutAddMenuEntry("5", 1);
	glutAddMenuEntry("4", 2);
	glutAddMenuEntry("3", 3);
	glutAddMenuEntry("2", 4);
	glutAddMenuEntry("1", 5);

	submenu_resolution = glutCreateMenu(menu);
	glutAddMenuEntry("500 X 500", 6);
	glutAddMenuEntry("700 X 700", 7);

	submenu_viewer = glutCreateMenu(menu);
	glutAddMenuEntry("250, 250, -1000", 10);
	glutAddMenuEntry("0, 250, - 1000", 11);
	glutAddMenuEntry("250, 500, - 1000", 12);

	mainmenu = glutCreateMenu(menu);
	glutAddSubMenu("Depth", submenu_depth);
	glutAddSubMenu("Resolution", submenu_resolution);
	glutAddSubMenu("View Point", submenu_viewer);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void timePass(){
#if 1
	//adding
	if (time % 40 == 0){
		sphere.pop_back();
		int r = rand() % 9 - 4;
		int r2 = rand() % 9 - 4;
		sphere.push_back(Sphere(50, Vec(CAPTURE_W / 2 + r, 1200, 300 + r2), Vec(0, -1, 0), Vec(.1, .1, .1), true, true, 15, false, true));

		sphere.push_back(s_floor);	//floor
	}
#endif

	//moving
	for (int i = 0; i < sphere.size() - 1; ++i){		//except greatest ball, floor
		sphere[i].move();
	}

	//innerBall part
	for (int i = 0; i < num_innerBall + 1; ++i){		//except greatest ball
		if (sphere[i].speed == 0)
			continue;
		for (int j = 0; j < num_innerBall + 1; ++j){	//except floor and outer ball
			if (j == i)
				continue;

			sphere[i].checkConflict(&sphere[j], j == 0);
		}
	}

	//outerBall part
	for (int k = num_innerBall; k < sphere.size() - 1; ++k){
		for (int j = num_innerBall + 1; j < sphere.size() - 1; ++j){	//except floor and inner ball
			if (j == k)
				continue;

			sphere[k].checkConflict(&sphere[j], false);
		}
	}


	//calculation ray
	cal_ray();
}

void keyInput(unsigned char key, int x, int y){
	switch (key){
	case 't':
		cout << time << endl;
		++time;

		timePass();
		glutPostRedisplay();
		break;
	case 'i':
		sphere.pop_back();

		sphere.push_back(Sphere(25, Vec(CAPTURE_W - 100, 800, 50), Vec(-1, 0, 0), Vec(.7, .8, .7), true, true, 10, false, true));
		sphere.push_back(Sphere(25, Vec(100, 800, 50), Vec(1, 0, 0), Vec(.01, .01, .01), true, true, 10, false, true));
		sphere.push_back(Sphere(25, Vec(CAPTURE_W / 2, 1200, 50), Vec(-0.1, -1, -0.1), Vec(.1, .1, .1), true, true, 15, false, true));

		sphere.push_back(s_floor);	//floor
		break;
	}
}

void main() {
	int END = 960 * 2;
	string input;
	//	SYSTEMTIME time1, time2;
	//	WORD millis;
	omp_set_num_threads(8);

	makeSphere();
	//	makePolygon();

#ifdef MODE_CAPTURE
	while (time < END){
		timePass();
		++time;
		//		if (time >= END){
		//			cout << "More?" << endl;
		//			getline(cin, input);
		//			if(input == "y")
		//				END += 240;
		//			else
		//				break;
		//		}
	}

#else
	GetSystemTime(&time1);
	cal_ray();
	GetSystemTime(&time2);
	millis = ((time2.wSecond * 1000) + time2.wMilliseconds) - ((time1.wSecond * 1000) + time1.wMilliseconds);
	cout << "time:" << millis << endl;

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(W, H);
	glutCreateWindow("2012210019_4");
	setupMenus();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyInput);
	glutMainLoop();
#endif
}