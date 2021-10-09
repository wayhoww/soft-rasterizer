#include <iostream>
#include "image.hpp"
#include "matrix.hpp"
#include "rasterizer.hpp"
#include "OBJ_Loader.h"
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

template <typename Uniform, typename Shader> /* 里面有限制 Shader 类型了 */
Object<NothingProperty, Uniform, Shader> // TODO color ->
create_object_from_obj_loader_mesh(const objl::Mesh& mesh) {
	Object<NothingProperty, Uniform, Shader> object;
	for(int i = 0; i < mesh.Indices.size(); i += 3) 
		object.triangles.push_back({mesh.Indices[i], mesh.Indices[i+1], mesh.Indices[i+2]});
	
	for(const auto& vertex: mesh.Vertices) {
		Vertex<NothingProperty> vert;
		vert.pos = { vertex.Position.X, vertex.Position.Y, vertex.Position.Y };
		object.vertices.push_back(vert);
	}
	return object;
}

int main() {
	objl::Loader loader;
	loader.LoadFile("Keqing.obj");
	std::cout << "loaded: " << loader.LoadedMeshes.size() << " meshes" << std::endl;

	Rasterizer<NothingUniform> rasterizer;
	for(auto mesh: loader.LoadedMeshes) {
		auto loadedObject = create_object_from_obj_loader_mesh<NothingUniform, GrayShader>(mesh);
		rasterizer.addObject(loadedObject, { {1, 0, 0}, {0, 1, 0 }, {0, 0, 1 } }, { 0, 0, 0 });
	}
	
	rasterizer.rasterize(
		{0, 5, 40},				// pos
		{0, 0, -1},				// dir
		{0, 1,  0},				// top
		0.1,
		200,
		deg_to_rad(90),
		1,
		800,
		800
	).save("image_loaded.bmp");
}


/*

	GrayObject obj1 = getGrayObj();
	ColorObject obj2 = getColorObj();

	Vec3 obj_pos1{ -2, 0.0, 0.0 };
	Vec3 obj_pos2{ 2, 0.0, 0.0 };
	Mat3 obj_dir{
		{1, 0, 0},
		{0, 1, 0},
		{0, 0, 1}
	};

	// cannot guarentee objects are NothingUniform acceptable
	Rasterizer<NothingUniform> rasterizer;
	rasterizer.addObject(loadedObject, obj_dir, obj_pos1);
	rasterizer.addObject(obj2, obj_dir, obj_pos2);

	Vec3 camera_pos{ 0, 3, 9 };
	Vec3 camera_dir{ 0, -1, -3 };
	Vec3 camera_top{ 0, 3, -1 };

	rasterizer.rasterize(
		camera_pos,
		camera_dir,
		camera_top,
		0.5,
		10,
		deg_to_rad(90),
		2,
		800 * 2,
		800
	).save("image_multi.bmp");

	Image image(400, 200);
	image[{20, 10}].b = 1.0;
	image.save("b.bmp");

	getchar();

*/