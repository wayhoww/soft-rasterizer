#include <iostream>
#include "image.hpp"
#include "matrix.hpp"
#include "rasterizer.hpp"
#include "OBJ_Loader.h"
#include "blinn_phong.hpp"

template <typename P, typename Uniform, typename VShaderT, typename FShaderT> /* 里面有限制 Shader 类型了 */
Object<objl::Vertex, P, Uniform, VShaderT, FShaderT> // TODO color ->
create_object_from_obj_loader_mesh(const objl::Mesh& mesh) {
	Object<objl::Vertex, P, Uniform, VShaderT, FShaderT> object;
	for(int i = 0; i < mesh.Indices.size(); i += 3) 
		object.triangles.push_back({mesh.Indices[i], mesh.Indices[i+1], mesh.Indices[i+2]});
	
	object.vertices = mesh.Vertices;
	return object;
}

std::vector<std::string> split(const std::string& line, char c) {
	std::vector<std::string> components;
	size_t pos = 0;
	do {
		size_t npos = line.find(c, pos);
		if(npos != std::string::npos) {
			components.push_back(line.substr(pos, npos - pos));
		} else {
			components.push_back(line.substr(pos));
			break;
		}
		pos = npos + 1;
	} while (true);
	return components;
}

std::string trimmed(const std::string& str) {
	std::string out;
	bool first_non_empty_occurred = false;
	for(int i = 0; i < str.size(); i++) {
		if(str[i] != ' ') first_non_empty_occurred = true;
		if(first_non_empty_occurred) {
			out.push_back(str[i]);
		}
	}
	first_non_empty_occurred = false;
	int i;
	for(i = out.size() - 1; i >= 0; i--) {
		if(out[i] != ' ') {
			break;
		}
	}
	out.reserve(i + 1);
	return out;
}

int main() {
	objl::Loader loader;
	Rasterizer<BlinnPhongUniform> rasterizer;
	Vec3 camera_pos {0, 0, 10};
	Vec3 camera_dir {0, 0, -1};
	Vec3 camera_top {0, 1, 0};	// TODO: 这两个量必须垂直
	double z_near = 0.1;
	double z_far = 200;
	double fovY = 90; // deg
	double aspect_ratio = 1;
	int width = 1000;
	int height = 1000; // 这两个量也是关联的
	std::string filename = "out.bmp";
	std::string modelpath = "Keqing/Keqing.obj";

	BlinnPhongUniform uniform;
	uniform.lights.push_back(Light{
		{-0.4 * 7, 2.0 * 7, 1.2 * 7},
		RGBColor{3, 3, 3}
	});

	using namespace std;
	
	while(true) {
		cout << "objv > ";

		string line;
		getline(cin, line);

		try {
			auto commands = split(line, ';');
			for(auto command: commands){
				auto args = split(trimmed(command), ' ');
		
				if(args.size() == 1 && args[0] == "exit") {
					return 0;
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
						>(mesh);
						// TODO: 这不是常见的模型方向指定方式
						rasterizer.addObject(loadedObject, { {1, 0, 0}, {0, 1, 0 }, {0, 0, 1 } }, { 0, 0, 0 });
					}

					std::cout << "loaded: " << loader.LoadedMeshes.size() << " meshes" << std::endl;
				} else if (args.size() == 4 && args[0] == "cdir") {
					camera_dir = {stod(args[1]), stod(args[2]), stod(args[3])};
				} else if (args.size() == 4 && args[0] == "cpos") {
					camera_pos = {stod(args[1]), stod(args[2]), stod(args[3])};
				} else if (args.size() == 4 && args[0] == "ctop") {
					camera_top = {stod(args[1]), stod(args[2]), stod(args[3])};
				} else if (args.size() == 2 && args[0] == "znear") {
					z_near = stod(args[1]);
				} else if (args.size() == 2 && args[0] == "zfar") {
					z_far = stod(args[1]);
				} else if (args.size() == 2 && args[0] == "fov") {
					fovY = stod(args[1]);
				} else if (args.size() == 2 && args[0] == "ar") {
					aspect_ratio = stod(args[1]);
				} else if (args.size() == 2 && args[0] == "width") {
					width = stoi(args[1]);
				} else if (args.size() == 2 && args[0] == "height") {
					height = stoi(args[1]);
				} else if ((args.size() == 1 || args.size() == 2) && args[0] == "w") {
					if(args.size() == 2) {
						filename = args[1];
					}

					rasterizer.uniform = uniform;
					rasterizer.rasterize(
						camera_pos,				// pos
						camera_dir,				// dir
						camera_top,				// top
						z_near,
						z_far,
						deg_to_rad(fovY),
						aspect_ratio,
						width,
						height
					).save(filename);
				} else if (args.size() == 1 && args[0] == "p") {
					printf(
						"[Camera]\n"
						"position(cpos)     %.2f %.2f %.2f\n"
						"direction(cdir)    %.2f %.2f %.2f\n"
						"top(ctop)          %.2f %.2f %.2f\n"
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
}