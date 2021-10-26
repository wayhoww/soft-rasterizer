#pragma once

#include "common_header.hpp"
#include "blinn_phong.hpp"
#include "shader.hpp"
#include "matrix.hpp"
#include "OBJ_Loader.h"
#include <cmath>

Vec3 obj_ld_vec_to_vec3(const objl::Vector3& vec) {
	return { vec.X, vec.Y, vec.Z };
};

Vec3 sqrt(const Vec3& vec3) {
	return {
		sqrt(vec3[0]),
		sqrt(vec3[1]),
		sqrt(vec3[2])
	};
}

template <typename P, typename Uniform, typename VShaderT, typename FShaderT> /* 里面有限制 Shader 类型了 */
Object<BlinnPhongAttribute, P, Uniform, VShaderT, FShaderT> // TODO color ->
create_object_from_obj_loader_mesh(const objl::Mesh& mesh, const std::string& obj_path) {
	Object<BlinnPhongAttribute, P, Uniform, VShaderT, FShaderT> object;
	
	auto basepath = obj_path.substr(0, obj_path.find_last_of('/'));

	std::shared_ptr<Image> map_Kd = nullptr, map_Ka = nullptr, map_Ks = nullptr, map_bump = nullptr;
	
	if(!mesh.MeshMaterial.map_Kd.empty()) 
		map_Kd = std::make_shared<Image>(basepath + "/" + mesh.MeshMaterial.map_Kd);
	
	if(!mesh.MeshMaterial.map_Ka.empty()) 
		map_Ka = std::make_shared<Image>(basepath + "/" + mesh.MeshMaterial.map_Ka);

	if(!mesh.MeshMaterial.map_Ks.empty()) 
		map_Ks = std::make_shared<Image>(basepath + "/" + mesh.MeshMaterial.map_Ka);

	if(!mesh.MeshMaterial.map_bump.empty()) 
		map_bump = std::make_shared<Image>(basepath + "/" + mesh.MeshMaterial.map_bump);

	Image::reset_cache();

	// 本质问题是 TBN 其实是一个面属性，不是一个点属性。一般应该如何解决这种问题？
	// 这里的处理方式：
	// 每个三角形的属性由第一个点记录
	// 每个顶点最多记录一个三角形对应的信息。如果不能满足，那就重复顶点。
	for(auto vert: mesh.Vertices) {
		BlinnPhongAttribute attr;
		attr.vertex = vert;
		attr.map_Kd = map_Kd;
		attr.map_Ka = map_Ka;
		attr.map_Ks = map_Ks;
		attr.material = mesh.MeshMaterial;
		attr.map_bump = map_bump;
		object.vertices.push_back(attr);
	}
	
	std::vector<bool> TBN_set(object.vertices.size(), false);

	for(int i = 0; i < mesh.Indices.size(); i += 3) {
		// 1. 找到三个点中 TBN 没有设置的那个
		int v1 = mesh.Indices[i + 0];
		int v2 = mesh.Indices[i + 1];
		int v3 = mesh.Indices[i + 2];

		if(!TBN_set[v1]) {
			// do nothing
		}else if(!TBN_set[v2]){
			std::swap(v1, v2);
		}else if(!TBN_set[v3]){
			std::swap(v1, v3);
		}else{
			object.vertices.push_back(object.vertices[v1]);
			v1 = object.vertices.size() - 1;
		}
		TBN_set[v1] = true;
		object.triangles.push_back({v1, v2, v3});

		// calculate TBN matrix
		auto A = obj_ld_vec_to_vec3(object.vertices[v1].vertex.Position);
		auto B = obj_ld_vec_to_vec3(object.vertices[v2].vertex.Position);
		auto C = obj_ld_vec_to_vec3(object.vertices[v3].vertex.Position);

		auto y1 = B - A;
		auto y2 = C - A;
		auto val_u1 = object.vertices[v2].vertex.TextureCoordinate.X - object.vertices[v1].vertex.TextureCoordinate.X;
		auto val_u2 = object.vertices[v3].vertex.TextureCoordinate.X - object.vertices[v1].vertex.TextureCoordinate.X;
		auto val_v1 = object.vertices[v2].vertex.TextureCoordinate.Y - object.vertices[v1].vertex.TextureCoordinate.Y;
		auto val_v2 = object.vertices[v3].vertex.TextureCoordinate.Y - object.vertices[v1].vertex.TextureCoordinate.Y;

		auto u = (y2 * val_v1 - y1 * val_v2) * (1.0 / (val_v1 * val_u2 - val_u1 * val_v2));
		auto v = (y2 * val_u1 - y1 * val_u2) * (1.0 / (val_u1 * val_v2 - val_v1 * val_u2));
		auto n = cross_product(u, v).normalized() * sqrt(u.norm2() * v.norm2());

		object.vertices[v1].TBN = Mat3::hcat(u, v, n);
	}


	return object;
}
