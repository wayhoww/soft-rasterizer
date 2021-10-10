#pragma once

#include "shader.hpp"
#include "OBJ_Loader.h"
#include <iostream>

struct BlinnPhongAttribute {
    objl::Vertex vertex;
    std::shared_ptr<const Image> map_Kd = nullptr;
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
                       std::shared_ptr<const Image> map_Kd): 
                    normal(normal), uv(uv), map_Kd(map_Kd) {}

    Vec3 normal;
    Vec2 uv;
    std::shared_ptr<const Image> map_Kd = nullptr;

    // TODO: assert map_Kd is the same
    BlinnPhongProperty operator+(const BlinnPhongProperty& other) const {
        return BlinnPhongProperty{ normal + other.normal, uv + other.uv, map_Kd };
    }
    
    BlinnPhongProperty operator*(double k) const {
        return BlinnPhongProperty{ normal * k, uv * k, map_Kd };
    }
};


class BlinnPhongVShader: public VShader<BlinnPhongAttribute, BlinnPhongProperty, BlinnPhongUniform> {
public:
    virtual Vertex<BlinnPhongProperty> shade(
        const BlinnPhongAttribute& attr, 
        const BlinnPhongUniform& _,
        const Mat4& M
    ) const {
        auto& data = attr.vertex;

        Vertex<BlinnPhongProperty> vert;
        vert.pos = { data.Position.X, data.Position.Y, data.Position.Z };
        vert.properties.normal = { data.Normal.X, data.Normal.Y, data.Normal.Z };
        vert.properties.normal = to_vec3_as_dir(M * to_vec4_as_dir(vert.properties.normal));
        vert.properties.map_Kd = attr.map_Kd;
        vert.properties.uv = { attr.vertex.TextureCoordinate.X, attr.vertex.TextureCoordinate.Y };
        return vert;
    }
};

class BlinnPhongFShader: public FShader<BlinnPhongProperty, BlinnPhongUniform> {
public:
    virtual RGBAColor shade(
        const Fragment<BlinnPhongProperty>& fragment, 
        const BlinnPhongUniform& uniform
    ) const {
        auto normal = fragment.properties.normal.normalized();
        
        RGBAColor out = {0, 0, 0, 1};

        // for(auto light: uniform.lights) {
        //     auto r_squared = (light.pos - fragment.pos).norm2_squared();
        //     out.r += light.intensity.r / r_squared * 
        //         std::max<double>(0, dot_product(fragment.properties.normal, light.pos - fragment.pos));
        //     out.g += light.intensity.g / r_squared * 
        //         std::max<double>(0, dot_product(fragment.properties.normal, light.pos - fragment.pos));
        //     out.b += light.intensity.b / r_squared * 
        //         std::max<double>(0, dot_product(fragment.properties.normal, light.pos - fragment.pos));
        // }

        // std::cout << fragment.properties.uv[0] << " " << fragment.properties.uv[1] << std::endl;
        
        auto uv = fragment.properties.uv;
        auto& map_Kd = *fragment.properties.map_Kd;
        auto [width, height] = map_Kd.size();
        out = map_Kd[{ int(uv[0] * width), int(uv[1] * height) }];

        return out.clip();
    }
};