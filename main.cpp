#include <iostream>
#include "image.hpp"
#include "matrix.hpp"
#include "rasterizer.hpp"
#include "simple_gray.hpp"
#include "simple_color.hpp"

GrayObject getGrayObj() {
	GrayObject obj;

	Vec3 pt0{+1, -1, -1};
	Vec3 pt1{+1, +1, -1};
	Vec3 pt2{-1, +1, -1};
	Vec3 pt3{-1, -1, -1};
	Vec3 pt4{+1, -1, +1};
	Vec3 pt5{+1, +1, +1};
	Vec3 pt6{-1, +1, +1};
	Vec3 pt7{-1, -1, +1};

	obj.vertices.push_back(Vertex<NothingProperty>(pt0, NothingProperty()));
	obj.vertices.push_back(Vertex<NothingProperty>(pt1, NothingProperty()));
	obj.vertices.push_back(Vertex<NothingProperty>(pt2, NothingProperty()));
	obj.vertices.push_back(Vertex<NothingProperty>(pt3, NothingProperty()));
	obj.vertices.push_back(Vertex<NothingProperty>(pt4, NothingProperty()));
	obj.vertices.push_back(Vertex<NothingProperty>(pt5, NothingProperty()));
	obj.vertices.push_back(Vertex<NothingProperty>(pt6, NothingProperty()));
	obj.vertices.push_back(Vertex<NothingProperty>(pt7, NothingProperty()));

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
	return obj;
}

RGBColor rand_color() {
	return {
		(rand() % 255) / 255.0,
		(rand() % 255) / 255.0,
		(rand() % 255) / 255.0
	};
}

ColorObject getColorObj() {
	ColorObject obj;

	Vec3 pt0{+1, -1, -1};
	Vec3 pt1{+1, +1, -1};
	Vec3 pt2{-1, +1, -1};
	Vec3 pt3{-1, -1, -1};
	Vec3 pt4{+1, -1, +1};
	Vec3 pt5{+1, +1, +1};
	Vec3 pt6{-1, +1, +1};
	Vec3 pt7{-1, -1, +1};

	obj.vertices.push_back(Vertex<ColorProperty>(pt0, ColorProperty(rand_color())));
	obj.vertices.push_back(Vertex<ColorProperty>(pt1, ColorProperty(rand_color())));
	obj.vertices.push_back(Vertex<ColorProperty>(pt2, ColorProperty(rand_color())));
	obj.vertices.push_back(Vertex<ColorProperty>(pt3, ColorProperty(rand_color())));
	obj.vertices.push_back(Vertex<ColorProperty>(pt4, ColorProperty(rand_color())));
	obj.vertices.push_back(Vertex<ColorProperty>(pt5, ColorProperty(rand_color())));
	obj.vertices.push_back(Vertex<ColorProperty>(pt6, ColorProperty(rand_color())));
	obj.vertices.push_back(Vertex<ColorProperty>(pt7, ColorProperty(rand_color())));

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

	obj.shader = ColorShader();

	return obj;
}

#include <list>

int main() {
	{
		std::list<int> list;
		auto it = list.begin();
		auto temp1 = it == list.end();
		list.push_back(1);
		auto temp2 = it == list.end();
		//
	}


	GrayObject obj1 = getGrayObj();
	ColorObject obj2 = getColorObj();

	Vec3 obj_pos1 {-2, 0.0, 0.0};
	Vec3 obj_pos2 {2, 0.0, 0.0};
	Mat3 obj_dir{
		{1, 0, 0},
		{0, 1, 0},
		{0, 0, 1}
	};


	ObjectDescriptor desp1{std::make_shared<GrayObject>(obj1), obj_dir, obj_pos1};
	ObjectDescriptor desp2{std::make_shared<ColorObject>(obj2), obj_dir, obj_pos2};
	

	// cannot guarentee objects are NothingUniform acceptable
	Rasterizer<NothingUniform> rasterizer;
	rasterizer.objects.push_back(desp1);
	rasterizer.objects.push_back(desp2);

	Vec3 camera_pos {0, 3, 4};
	Vec3 camera_dir {0, -0.7, -1}; 
	Vec3 camera_top {0, 1.0, -0.7};

	rasterizer.rasterize(
		camera_pos,
		camera_dir,
		camera_top,
		0.5,
		10,
		deg_to_rad(90),
		1,
		800,
		800
	).save("image_multi.bmp");
}