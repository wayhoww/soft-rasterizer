#pragma once

#include "shader.hpp"
#include "OBJ_Loader.h"
#include <iostream>

struct BlinnPhongAttribute {
    objl::Vertex vertex;
    objl::Material material;
    std::shared_ptr<const Image> map_Kd = nullptr;
    std::shared_ptr<const Image> map_Ka = nullptr;
    std::shared_ptr<const Image> map_Ks = nullptr;
};

struct Light {
    Vec3 pos;
    RGBAColor intensity; 
};

class BlinnPhongUniform {
public:
    std::vector<Light> lights;
};

class BlinnPhongProperty: public AbstractInterpolatable {
public:
    BlinnPhongProperty() = default;
    BlinnPhongProperty(const Vec3& normal, 
                       const Vec2& uv, 
                       const objl::Material& material,
                       const Mat4& M,
                       const Vec3& camera_pos,
                       std::shared_ptr<const Image> map_Kd,
                       std::shared_ptr<const Image> map_Ka,
                       std::shared_ptr<const Image> map_Ks): 
                    normal(normal), uv(uv), map_Kd(map_Kd), map_Ka(map_Ka), map_Ks(map_Ks), material(material), M(M), camera_pos(camera_pos) {}

    Vec3 normal;
    Vec2 uv;
    std::shared_ptr<const Image> map_Kd = nullptr;
    std::shared_ptr<const Image> map_Ka = nullptr;
    std::shared_ptr<const Image> map_Ks = nullptr;
    Mat4 M;
    Vec3 camera_pos;
    
    objl::Material material;

    // TODO: assert map_Kd is the same
    BlinnPhongProperty operator+(const BlinnPhongProperty& other) const {
        return BlinnPhongProperty{ normal + other.normal, uv + other.uv, material, M, camera_pos, map_Kd, map_Ka, map_Ks };
    }
    
    BlinnPhongProperty operator*(double k) const {
        return BlinnPhongProperty{ normal * k, uv * k, material, M, camera_pos, map_Kd, map_Ka, map_Ks };
    }
};


class BlinnPhongVShader: public VShader<BlinnPhongAttribute, BlinnPhongProperty, BlinnPhongUniform> {
public:
    virtual Vertex<BlinnPhongProperty> shade(
        const BlinnPhongAttribute& attr, 
        const BlinnPhongUniform& _,
        const Mat4& M,
        const Vec3& camera_pos
    ) const {
        auto& data = attr.vertex;

        Vertex<BlinnPhongProperty> vert;
        vert.pos = { data.Position.X, data.Position.Y, data.Position.Z };
        vert.properties.normal = { data.Normal.X, data.Normal.Y, data.Normal.Z };
        vert.properties.normal = to_vec3_as_dir(M * to_vec4_as_dir(vert.properties.normal));
        vert.properties.map_Kd = attr.map_Kd;
        vert.properties.map_Ka = attr.map_Ka;
        vert.properties.map_Ks = attr.map_Ks;
        vert.properties.uv = { attr.vertex.TextureCoordinate.X, attr.vertex.TextureCoordinate.Y };
        vert.properties.material = attr.material;
        vert.properties.M = M;
        vert.properties.camera_pos = camera_pos;
        return vert;
    }
};

RGBAColor objl_vec3_to_color(const objl::Vector3& vec3) {
    return RGBAColor{ vec3.X, vec3.Y, vec3.Z, 1.0 };
}

RGBAColor get_texture(std::shared_ptr<const Image> texture, const Vec2& uv) {
    if(!texture) {
        return RGBAColor{1.0, 1.0, 1.0, 1.0};
    }

    auto [width, height] = texture->size();
    auto texture_color = (*texture)[{
        std::min<int>(width - 1, lround(uv[0] * (width - 1))), 
        std::min<int>(height - 1, lround(uv[1] * (height - 1)))
    }];
    return texture_color;
}

class BlinnPhongFShader: public FShader<BlinnPhongProperty, BlinnPhongUniform> {
public:
    virtual RGBAColor shade(
        const Fragment<BlinnPhongProperty>& fragment, 
        const BlinnPhongUniform& uniform
    ) const {
        
        RGBAColor out = {0, 0, 0, 1};
        auto normal_world = to_vec3_as_dir(fragment.properties.M * to_vec4_as_dir(fragment.properties.normal)).normalized();
        
        // what if no map ?
        {
            for(auto light: uniform.lights) {
                auto light_pos_world = light.pos;
                
                //// kd
                // texture
                auto texture_color = get_texture(fragment.properties.map_Kd, fragment.properties.uv);
            
                // coefficient
                auto k = std::max<double>(0, dot_product(
                    normal_world, 
                    (light_pos_world - fragment.pos).normalized()
                ));
                auto r_squared = (light.pos - fragment.pos).norm2_squared();
                auto eff = k / r_squared;

                auto c = objl_vec3_to_color(fragment.properties.material.Kd) * texture_color * light.intensity * eff;
                out += c;

                //// ks
                // texture
                auto highlight_texture_color = get_texture(fragment.properties.map_Ks, fragment.properties.uv);
                
                auto h = ((fragment.properties.camera_pos - fragment.pos).normalized() + (light_pos_world - fragment.pos).normalized()).normalized();
                out += objl_vec3_to_color(fragment.properties.material.Ks) * 
                       highlight_texture_color * 
                       pow( (long double)std::max<double>(0, dot_product(normal_world, h)), (long double) fragment.properties.material.Ni);
            }
        }

        {
            // Ka
            auto texture_color = get_texture(fragment.properties.map_Ka, fragment.properties.uv);

            out += objl_vec3_to_color(fragment.properties.material.Ka) * texture_color;
        }
       

        return out.clip();
    }
};