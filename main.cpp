#include "common_header.hpp"
#include <iostream>
#include <algorithm>
#include "image.hpp"
#include "matrix.hpp"
#include "rasterizer.hpp"
#include "OBJ_Loader.h"
#include "blinn_phong.hpp"
#include "ld_obj_loader.hpp"
#include "utils.hpp"

Vec3 correct(const Vec3& n, const Vec3& a) {
	return a - n * (dot_product(n, a) / dot_product(n, n));
}


int entrance(int argc, char** argv) {
	objl::Loader loader;
	Rasterizer<BlinnPhongUniform> rasterizer;
/*
	Vec3 camera_pos {0, 100, 0};
	Vec3 camera_dir {0, -1, 0};
	Vec3 camera_top {1, 0, 0};
	camera_top = correct(camera_dir, camera_top);

	float z_near = 0.1;
	float z_far = 200;
	float fovY = 90; // deg
	float aspect_ratio = 1.0;
	int width = 800;
	int height = 800;

	Vec3 light_pos{-4, 16, 30};
	RGBAColor light_color{600, 600, 600, 1.0};
	rasterizer.uniform = BlinnPhongUniform {{
		Light {light_pos, light_color}	
	}};

	std::string filename = "out.bmp";
	std::string modelpath = "../samples/normalmap/normalmap.obj";
*/
	// Keqing
	Vec3 camera_pos {2, 16, 13};
	Vec3 camera_dir {-2, -2, -10};
	Vec3 camera_top {0, 1, 0};
	camera_top = correct(camera_dir, camera_top);

	float z_near = 0.1;
	float z_far = 200;
	float fovY = 90; // deg
	float aspect_ratio = 0.6;
	int width = 600;
	int height = 1000;

	Vec3 light_pos{-4, 16, 30};
	RGBAColor light_color{600, 600, 600, 1.0};
	rasterizer.uniform = BlinnPhongUniform {{
		Light {light_pos, light_color}	
	}};

	std::string filename = "out.bmp";
	std::string modelpath = "../../../samples/Keqing/Keqing.obj";


	using namespace std;

	bool should_exit = false;
	
	while (!should_exit) {
		cout << "objv> ";

		string line;
		getline(cin, line);

		try {
			auto commands = split(line, ';');
			for(auto command: commands){
				auto args = split(trimmed(command), ' ');
		
				if(args.size() == 1 && args[0] == "exit") {
					should_exit = true;
					break;
				} else if ((args.size() == 1 || args.size() == 2) && args[0] == "load") {
					rasterizer.clearObjects();
					if (args.size() == 2) {
						modelpath = args[1];
					}
					loader.LoadFile(modelpath);

					for(auto mesh: loader.LoadedMeshes) {
						auto loadedObject = create_object_from_obj_loader_mesh<
							BlinnPhongProperty,
							BlinnPhongUniform,
							BlinnPhongVShader,
							BlinnPhongFShader
						>(mesh, modelpath);
						// TODO: 这不是常见的模型方向指定方式
						rasterizer.addObject(loadedObject, { {1, 0, 0}, {0, 1, 0 }, {0, 0, 1 } }, { 0, 0, 0 });
					}
					std::cerr << "loaded: " << loader.LoadedMeshes.size() << " meshes" << std::endl;
				} else if (args.size() == 4 && args[0] == "cdir") {
					camera_dir = {stof(args[1]), stof(args[2]), stof(args[3])};
					camera_top = correct(camera_dir, camera_top);
					std::cerr << "camera_top is corrected to [" << camera_top[0] << ", " << camera_top[1] << ", " << camera_top[2] << " ]" << endl;
				} else if (args.size() == 4 && args[0] == "cpos") {
					camera_pos = {stof(args[1]), stof(args[2]), stof(args[3])};
				} else if (args.size() == 4 && args[0] == "ctop") {
					camera_top = {stof(args[1]), stof(args[2]), stof(args[3])};
					camera_dir = correct(camera_top, camera_dir);
					std::cerr << "camera_dir is corrected to [" << camera_dir[0] << ", " << camera_dir[1] << ", " << camera_dir[2] << " ]" << endl;
				} else if (args.size() == 4 && args[0] == "lpos") {
					light_pos = {stof(args[1]), stof(args[2]), stof(args[3])};
					rasterizer.uniform = BlinnPhongUniform {{ Light {light_pos, light_color} }};
				} else if (args.size() == 4 && args[0] == "lcolor") {
					light_color = {stof(args[1]), stof(args[2]), stof(args[3]), 1.0};
					rasterizer.uniform = BlinnPhongUniform {{ Light {light_pos, light_color} }};
				} else if (args.size() == 2 && args[0] == "znear") {
					z_near = stof(args[1]);
				} else if (args.size() == 2 && args[0] == "zfar") {
					z_far = stof(args[1]);
				} else if (args.size() == 2 && args[0] == "fov") {
					fovY = stof(args[1]);
				} else if (args.size() == 2 && args[0] == "ar") {
					aspect_ratio = stof(args[1]);
					height = std::lround(width / aspect_ratio);
					std::cerr << "height is corrected to " << height << endl;
				} else if (args.size() == 2 && args[0] == "width") {
					width = stoi(args[1]);
					// width / height == ar
					aspect_ratio = 1.0 * width / height;
					std::cerr << "ar is corrected to " << aspect_ratio << endl;
				} else if (args.size() == 2 && args[0] == "height") {
					height = stoi(args[1]);
					aspect_ratio = 1.0 * width / height;
					std::cerr << "ar is corrected to " << aspect_ratio << endl;
				} else if ((args.size() == 1 || args.size() == 2) && args[0] == "w") {
					if(args.size() == 2) {
						filename = args[1];
					}
					auto image = rasterizer.rasterize(
						camera_pos,				// pos
						camera_dir,				// dir
						camera_top,				// top
						z_near,
						z_far,
						deg_to_rad(fovY),
						aspect_ratio,
						width,
						height
					);
					save_image(image, filename);
				} else if (args.size() == 1 && args[0] == "p") {
					printf(
						"[Camera]\n"
						"position(cpos)     %.2f %.2f %.2f\n"
						"direction(cdir)    %.2f %.2f %.2f\n"
						"top(ctop)          %.2f %.2f %.2f\n"
						"[Light]\n"
						"position(lpos)     %.2f %.2f %.2f\n"
						"color(lcolor)      %.2f %.2f %.2f\n"
						"[Frustum]\n"  
						"z_near(znear)      %.2f\n"
						"z_far(far)         %.2f\n" 
						"fovY(fov)          %.2f deg\n" 
						"aspect ratio(ar)   %.2f\n"
						"[Screen]\n"  
						"width              %d\n"
						"height             %d\n"
						"[I/O]\n"  
						"model(load)        %s\n"
						"output(w)          %s\n",

						camera_pos[0], camera_pos[1], camera_pos[2], 
						camera_dir[0], camera_dir[1], camera_dir[2], 
						camera_top[0], camera_top[1], camera_top[2], 
						light_pos[0], light_pos[1], light_pos[2], 
						light_color.r, light_color.g, light_color.b,
						z_near, z_far, fovY, aspect_ratio,
						width, height,
						modelpath.c_str(), filename.c_str()
					);
				} else if(args.size() == 0) {
					// do nothing
				} else {
					cout << "unsupported command" << endl;
				}
			}
		} catch(...) {
			cout << "exception occurred" << endl;
		}
	}	
	return 0;
}

int main(int argc, char** argv) {
#ifdef _DEBUG
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif 

	// load ../../../samples/simple/simple.obj; width 5; height 5; w; exit;

	return entrance(argc, argv);
}