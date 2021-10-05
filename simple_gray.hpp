#pragma once
#include "shader.hpp"

class GrayShader: public Shader<NothingProperty, NothingUniform> {
public:
    RGBColor shade(const Fragment<NothingProperty>& fragment, const NothingUniform& uniform) const override {
        return RGBColor{0.5, 0.5, 0.5};
    }
};


using GrayObject = Object<NothingProperty, NothingUniform, GrayShader>;