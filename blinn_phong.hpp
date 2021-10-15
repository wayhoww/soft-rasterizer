#pragma once

#include "shader.hpp"
#include "OBJ_Loader.h"
#include <iostream>

struct BlinnPhongAttribute {
    objl::Vertex vertex;
    objl::Material material;
    Mat3 TBN;
    std::shared_ptr<const Image> map_Kd = nullptr;
    std::shared_ptr<const Image> map_Ka = nullptr;
    std::shared_ptr<const Image> map_Ks = nullptr;
    std::shared_ptr<const Image> map_bump = nullptr;
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
                       const Mat3& TBN,
                       const objl::Material& material,
                       const Mat4& M,
                       const Vec3& camera_pos,
                       std::shared_ptr<const Image> map_Kd,
                       std::shared_ptr<const Image> map_Ka,
                       std::shared_ptr<const Image> map_Ks,
                       std::shared_ptr<const Image> map_bump): 
                    normal_world_n(normal), uv(uv), map_Kd(map_Kd), 
                    map_Ka(map_Ka), map_Ks(map_Ks), material(material), 
                    M(M), camera_pos(camera_pos), map_bump(map_bump), TBN(TBN) {}

    Vec3 normal_world_n;
    Vec2 uv;
    Mat3 TBN;
    std::shared_ptr<const Image> map_Kd = nullptr;
    std::shared_ptr<const Image> map_Ka = nullptr;
    std::shared_ptr<const Image> map_Ks = nullptr;
    std::shared_ptr<const Image> map_bump = nullptr;
    Mat4 M;
    Vec3 camera_pos;
    
    objl::Material material;
    // TODO 这其实很不合理。。。要求插值顺序了。。
    // TODO: assert map_Kd is the same
    BlinnPhongProperty operator+(const BlinnPhongProperty& other) const {
        return BlinnPhongProperty{ normal_world_n + other.normal_world_n, uv + other.uv, TBN, material, M, camera_pos, map_Kd, map_Ka, map_Ks, map_bump };
    }
    
    BlinnPhongProperty operator*(float k) const {
        return BlinnPhongProperty{ normal_world_n * k, uv * k, TBN, material, M, camera_pos, map_Kd, map_Ka, map_Ks, map_bump };
    }
};


class BlinnPhongVShader: public VShader<BlinnPhongAttribute, BlinnPhongProperty, BlinnPhongUniform> {
public:
    virtual Vertex<BlinnPhongProperty> shade(
        const BlinnPhongAttribute& attr, 
        const BlinnPhongUniform& _,
        const RasterizerInfo& info
    ) const {
        auto& data = attr.vertex;

        Vertex<BlinnPhongProperty> vert;
        
        // vert
        vert.pos_model = { data.Position.X, data.Position.Y, data.Position.Z };
        vert.properties.normal_world_n = to_vec3_as_dir(info.M * to_vec4_as_dir({ data.Normal.X, data.Normal.Y, data.Normal.Z })).normalized();
        vert.properties.TBN = attr.TBN;

        // material
        vert.properties.map_Kd = attr.map_Kd;
        vert.properties.map_Ka = attr.map_Ka;
        vert.properties.map_Ks = attr.map_Ks;
        vert.properties.map_bump = attr.map_bump;
        vert.properties.uv = { attr.vertex.TextureCoordinate.X, attr.vertex.TextureCoordinate.Y };
        vert.properties.material = attr.material;

        // rasterizer
        vert.properties.M = info.M;
        vert.properties.camera_pos = info.camera_pos;

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
    auto texture_color = texture->getPixel(
        std::min<int>(width - 1, lround(uv[0] * (width - 1))), 
        std::min<int>(height - 1, lround(uv[1] * (height - 1)))
    );
    return texture_color;
}

class BlinnPhongFShader: public FShader<BlinnPhongProperty, BlinnPhongUniform> {
public:
    virtual RGBAColor shade(
        const Fragment<BlinnPhongProperty>& fragment, 
        const BlinnPhongUniform& uniform
    ) const {
        
        RGBAColor out = {0, 0, 0, 1};
        auto normal_world = fragment.properties.normal_world_n.normalized();

        // bump mapping (normal)
        // 改变的：表面法向量
        if(fragment.properties.map_bump != nullptr) {
            
            // change normal_world
            auto normal_c = get_texture(fragment.properties.map_bump, fragment.properties.uv);
            Vec3 normal_tangent {
                normal_c.r * 2 - 1,
                normal_c.g * 2 - 1,
                normal_c.b * 2 - 1
            };
            Vec3 normal_model = fragment.properties.TBN * normal_tangent;
            normal_world = to_vec3_as_dir(fragment.properties.M * to_vec4_as_dir(normal_model)).normalized();
        } 
        
        // what if no map ?
        {
            for(auto light: uniform.lights) {
                auto light_pos_world = light.pos;
                
                //// kd
                // texture
                auto texture_color = get_texture(fragment.properties.map_Kd, fragment.properties.uv);
            
                // coefficient
                auto k = std::max<float>(0, dot_product(
                    normal_world, 
                    (light_pos_world - fragment.pos_world).normalized()
                ));
                auto r_squared = (light.pos - fragment.pos_world).norm2_squared();
                auto eff = k / r_squared;

                auto c = objl_vec3_to_color(fragment.properties.material.Kd) * texture_color * light.intensity * eff;
                out += c;

                //// ks
                // texture
                auto highlight_texture_color = get_texture(fragment.properties.map_Ks, fragment.properties.uv);
                
                auto h = ((fragment.properties.camera_pos - fragment.pos_world).normalized() + (light_pos_world - fragment.pos_world).normalized()).normalized();
                auto s = objl_vec3_to_color(fragment.properties.material.Ks) * 
                       highlight_texture_color * 
                       pow( (float)std::max<float>(0, dot_product(normal_world, h)), (float) fragment.properties.material.Ns);
                out += s;
            }
        }

        {
            // Ka
            auto texture_color = get_texture(fragment.properties.map_Ka, fragment.properties.uv);

            auto a = objl_vec3_to_color(fragment.properties.material.Ka) * texture_color;
            out += a;
        }
       
        out.a = 1.0;

        return out.clip();
    }
};