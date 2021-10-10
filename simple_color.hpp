// #pragma once
// #include "shader.hpp"

// class ColorProperty: public AbstractInterpolatable {
// public:
//     RGBAColor color;
//     ColorProperty() = default; 
//     ColorProperty(const RGBAColor& color): color(color) { }
//     ColorProperty inversed() const {
//         auto out = *this;
//         out.color.r = 1.0 / out.color.r;
//         out.color.g = 1.0 / out.color.g;
//         out.color.b = 1.0 / out.color.b;
//         return out;
//     }
//     ColorProperty operator+(const ColorProperty& rhs) const { 
//         return ColorProperty(this->color + rhs.color); 
//     }
//     ColorProperty operator*(double k) const { 
//         return ColorProperty(this->color * k); 
//     };
// };

// class ColorShader: public Shader<ColorProperty, NothingUniform> {
// public:
//     RGBAColor shade(const Fragment<ColorProperty>& fragment, const NothingUniform& uniform) const override {
//         return fragment.properties.color;
//     }
// };


// using ColorObject = Object<ColorProperty, NothingUniform, ColorShader>;