#pragma once

#include "blinn_phong.hpp"
#include "shader.hpp"

template <typename P, typename Uniform, typename VShaderT, typename FShaderT> /* 里面有限制 Shader 类型了 */
Object<BlinnPhongAttribute, P, Uniform, VShaderT, FShaderT> // TODO color ->
create_object_from_obj_loader_mesh(const objl::Mesh& mesh, const std::string& obj_path) {
	Object<BlinnPhongAttribute, P, Uniform, VShaderT, FShaderT> object;
	for(int i = 0; i < mesh.Indices.size(); i += 3) 
		object.triangles.push_back({mesh.Indices[i], mesh.Indices[i+1], mesh.Indices[i+2]});
	
	auto basepath = obj_path.substr(0, obj_path.find_last_of('/'));

	std::shared_ptr<Image> map_Kd = nullptr, map_Ka = nullptr;
	
	if(!mesh.MeshMaterial.map_Kd.empty()) 
		map_Kd = std::make_shared<Image>(basepath + "/" + mesh.MeshMaterial.map_Kd);
	
	if(!mesh.MeshMaterial.map_Ka.empty()) 
		map_Ka = std::make_shared<Image>(basepath + "/" + mesh.MeshMaterial.map_Ka);

	for(auto vert: mesh.Vertices) {
		BlinnPhongAttribute attr;
		attr.vertex = vert;
		attr.map_Kd = map_Kd;
		attr.map_Ka = map_Ka;
		attr.material = mesh.MeshMaterial;
		object.vertices.push_back(attr);
	}
	return object;
}
