#include <iostream>
#include "image.hpp"
#include "matrix.hpp"
#include "rasterizer.hpp"
#include "simple_gray.hpp"

int main() {
	GrayObject obj;

	Vec3 pt0; pt0[{0, 0}] = +1; pt0[{1, 0}] = -1; pt0[{2, 0}] = -1;
	Vec3 pt1; pt1[{0, 0}] = +1; pt1[{1, 0}] = +1; pt1[{2, 0}] = -1;
	Vec3 pt2; pt2[{0, 0}] = -1; pt2[{1, 0}] = +1; pt2[{2, 0}] = -1;
	Vec3 pt3; pt3[{0, 0}] = -1; pt3[{1, 0}] = -1; pt3[{2, 0}] = -1;
	Vec3 pt4; pt4[{0, 0}] = +1; pt4[{1, 0}] = -1; pt4[{2, 0}] = +1;
	Vec3 pt5; pt5[{0, 0}] = +1; pt5[{1, 0}] = +1; pt5[{2, 0}] = +1;
	Vec3 pt6; pt6[{0, 0}] = -1; pt6[{1, 0}] = +1; pt6[{2, 0}] = +1;
	Vec3 pt7; pt7[{0, 0}] = -1; pt7[{1, 0}] = -1; pt7[{2, 0}] = +1;

	obj.vertices.push_back(Vertex<Nothing>(pt0, Nothing()));
	obj.vertices.push_back(Vertex<Nothing>(pt1, Nothing()));
	obj.vertices.push_back(Vertex<Nothing>(pt2, Nothing()));
	obj.vertices.push_back(Vertex<Nothing>(pt3, Nothing()));
	obj.vertices.push_back(Vertex<Nothing>(pt4, Nothing()));
	obj.vertices.push_back(Vertex<Nothing>(pt5, Nothing()));
	obj.vertices.push_back(Vertex<Nothing>(pt6, Nothing()));
	obj.vertices.push_back(Vertex<Nothing>(pt7, Nothing()));

	obj.triangles.push_back({0, 1, 2});
	obj.triangles.push_back({0, 3, 2});

	obj.triangles.push_back({1, 3, 0});
	obj.triangles.push_back({1, 4, 0});

	obj.triangles.push_back({2, 3, 7});
	obj.triangles.push_back({2, 6, 7});

	obj.triangles.push_back({1, 2, 6});
	obj.triangles.push_back({1, 5, 6});

	obj.triangles.push_back({1, 0, 4});
	obj.triangles.push_back({1, 5, 4});

	obj.triangles.push_back({4, 5, 6});
	obj.triangles.push_back({4, 7, 6});

	obj.shader = GrayShader();

	Vec3 obj_pos;
	Mat3 obj_dir;
	obj_dir[{0, 0}] = obj_dir[{1, 1}] = obj_dir[{2, 2}] = 1.0;
//	obj_pos[{2, 0}] = -1.0;

	ObjectDescriptor desp{std::make_shared<GrayObject>(obj), obj_dir, obj_pos};
	
	Rasterizer<NoUniform> rasterizer;
	rasterizer.objects.push_back(desp);

	Vec3 camera_pos;
	camera_pos[{0, 0}] = 3;
	camera_pos[{1, 0}] = 3;
	camera_pos[{2, 0}] = 3;
	Vec3 camera_dir;
	camera_dir[{0, 0}] = -1 / sqrt(3);
	camera_dir[{1, 0}] = -1 / sqrt(3);
	camera_dir[{2, 0}] = -1 / sqrt(3);
//	camera_dir[{2, 0}] = -1;
//  camera_dir[{1, 0}] = -1 / sqrt(2);
	Vec3 camera_top;
	//camera_top[{0, 0}] = 1 / sqrt(2);
//	camera_top[{1, 0}] = 1 / sqrt(2);
	camera_top[{0, 0}] = -1 / sqrt(2);
	camera_top[{1, 0}] = 1 / sqrt(2);

	rasterizer.rasterize(
		camera_pos,
		camera_dir,
		camera_top,
		-0.5,
		-10,
		90.0 / 180 * acos(-1.0),
		1,
		800,
		800
	).save("image.bmp");

	return 0;
}