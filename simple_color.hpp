#pragma once
#include "shader.hpp"

class ColorProperty: public AbstractInterpolatable {
public:
    RGBColor color;
    ColorProperty() = default; // 纯虚基类 vtable ?
    ColorProperty(const RGBColor& color): color(color) { } /* 这个可以 = default 吗 */
    ColorProperty inversed() const {
        auto out = *this;
        out.color.r = 1.0 / out.color.r;
        out.color.g = 1.0 / out.color.g;
        out.color.b = 1.0 / out.color.b;
        return out;
    }
    ColorProperty operator+(const ColorProperty& rhs) const { 
        return ColorProperty(this->color + rhs.color); 
    }
    ColorProperty operator*(double k) const { 
        return ColorProperty(this->color * k); 
    };
};

class ColorShader: public Shader<ColorProperty, NothingUniform> {
public:
    RGBColor shade(const Fragment<ColorProperty>& fragment, const NothingUniform& uniform) const override {
        return fragment.properties.color;
    }
};


using ColorObject = Object<ColorProperty, NothingUniform, ColorShader>;