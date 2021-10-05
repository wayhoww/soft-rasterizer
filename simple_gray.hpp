#pragma once
#include "shader.hpp"

class NoUniform: public AbstractUniform {};

class Nothing: public AbstractInterpolatable {
    virtual AbstractInterpolatable linear_interpolate_with (
        double k2, const AbstractInterpolatable& a2, 
        double k3, const AbstractInterpolatable& a3
    ) const {
        return *this;
    }
};

class GrayShader: public Shader<Nothing, NoUniform> {
public:
    RGBColor shade(const Fragment<Nothing>& fragment, const NoUniform& uniform) const override {
        return RGBColor{0.5, 0.5, 0.5};
    }
};


using GrayObject = Object<Nothing, NoUniform, GrayShader>;