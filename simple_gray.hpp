#pragma once
#include "shader.hpp"
#include "matrix.hpp"


// class GrayVShader: public VShader<Vec3, NothingProperty, NothingUniform> {

//     virtual Vertex<NothingProperty> shade(const Vec3& data, const NothingUniform& uniform) const override {
//         Vertex<NothingProperty> vec;
//         vec.pos = data;
//         return vec;
//     }
// };

// class GrayFShader: public FShader<NothingProperty, NothingUniform> {
// public:
//     RGBColor shade(const Fragment<NothingProperty>& fragment, const NothingUniform& uniform) const override {
//         return RGBColor{0.5, 0.5, 0.5};
//     }
// };


// using GrayObject = Object<Vec3, NothingProperty, NothingUniform, GrayVShader, GrayFShader>;