#pragma once

#include "shader.hpp"
#include "OBJ_Loader.h"

struct Light {
    Vec3 pos;
    RGBColor intensity; 
};

class BlinnPhongUniform {
public:
    std::vector<Light> lights;
};

class BlinnPhongProperty: public AbstractInterpolatable {
public:
    BlinnPhongProperty() = default;
    BlinnPhongProperty(const Vec3& normal): normal(normal) {}

    Vec3 normal;

    BlinnPhongProperty operator+(const BlinnPhongProperty& other) const {
        return BlinnPhongProperty{ normal + other.normal };
    }
    
    BlinnPhongProperty operator*(double k) const {
        return BlinnPhongProperty{ normal * k };
    }
};


class BlinnPhongVShader: public VShader<objl::Vertex, BlinnPhongProperty, BlinnPhongUniform> {
public:
    virtual Vertex<BlinnPhongProperty> shade(
        const objl::Vertex& data, 
        const BlinnPhongUniform& _,
        const Mat4& M
    ) const {
        Vertex<BlinnPhongProperty> vert;
        vert.pos = { data.Position.X, data.Position.Y, data.Position.Z };
        vert.properties.normal = { data.Normal.X, data.Normal.Y, data.Normal.Z };
        vert.properties.normal = to_vec3_as_dir(M * to_vec4_as_dir(vert.properties.normal));
        return vert;
    }
};

class BlinnPhongFShader: public FShader<BlinnPhongProperty, BlinnPhongUniform> {
public:
    virtual RGBColor shade(
        const Fragment<BlinnPhongProperty>& fragment, 
        const BlinnPhongUniform& uniform
    ) const {
        auto normal = fragment.properties.normal.normalized();
        
        RGBColor out = {0.25, 0.25, 0.25};

        for(auto light: uniform.lights) {
            auto r_squared = (light.pos - fragment.pos).norm2_squared();
            out.r += light.intensity.r / r_squared * 
                std::max<double>(0, dot_product(fragment.properties.normal, light.pos - fragment.pos));
            out.g += light.intensity.g / r_squared * 
                std::max<double>(0, dot_product(fragment.properties.normal, light.pos - fragment.pos));
            out.b += light.intensity.b / r_squared * 
                std::max<double>(0, dot_product(fragment.properties.normal, light.pos - fragment.pos));
        }
        
        return out.clip();
    }
};