#pragma once
#include "shader.hpp"

class ColorProperty: public AbstractInterpolatable {
public:
    ColorProperty() {} // 纯虚基类 vtable ?
    ColorProperty(const RGBColor& color): color(color){}
    RGBColor color;
    virtual void linear_interpolate_with (
        double k2, const AbstractInterpolatable& a2, 
        double k3, const AbstractInterpolatable& a3,
        AbstractInterpolatable& dest
    ) const {
        auto r2 = dynamic_cast<const ColorProperty&>(a2);
        auto r3 = dynamic_cast<const ColorProperty&>(a3);
        auto rd = dynamic_cast<ColorProperty*>(&dest); // why cannot &
        rd->color = this->color*(1-k2-k3) + k2*r2.color + k3*r3.color;
    }
};

class ColorShader: public Shader<ColorProperty, NothingUniform> {
public:
    RGBColor shade(const Fragment<ColorProperty>& fragment, const NothingUniform& uniform) const override {
        return fragment.properties.color;
    }
};


using ColorObject = Object<ColorProperty, NothingUniform, ColorShader>;