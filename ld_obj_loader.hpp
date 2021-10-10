#pragma once

#include "blinn_phong.hpp"
#include "shader.hpp"

template <typename P, typename Uniform, typename VShaderT, typename FShaderT> /* 里面有限制 Shader 类型了 */
Object<BlinnPhongAttribute, P, Uniform, VShaderT, FShaderT> // TODO color ->
create_object_from_obj_loader_mesh(const objl::Loader& loader, size_t index, const std::string& obj_path) {
	auto& mesh = loader.LoadedMeshes[index];
	Object<BlinnPhongAttribute, P, Uniform, VShaderT, FShaderT> object;
	for(int i = 0; i < mesh.Indices.size(); i += 3) 
		object.triangles.push_back({mesh.Indices[i], mesh.Indices[i+1], mesh.Indices[i+2]});
	
	auto basepath = obj_path.substr(0, obj_path.find_last_of('/'));
	auto map_Kd = std::make_shared<Image>(basepath + "/" + mesh.MeshMaterial.map_Kd);
	for(auto vert: mesh.Vertices) {
		BlinnPhongAttribute attr;
		attr.vertex = vert;
		attr.map_Kd = map_Kd;
		object.vertices.push_back(attr);
	}
	return object;
}
