#pragma once

#include <concepts>
#include <algorithm>
#include <memory>
#include <any>
#include "matrix.hpp"
#include "image.hpp"

class AbstractInterpolatable {
public:
    virtual ~AbstractInterpolatable() {}
};

template <typename T>
concept Interpolatable = requires(const T& a, double k) {
    std::derived_from<T, AbstractInterpolatable>;

    { a * k        } -> std::same_as<T>;
    { a + a        } -> std::same_as<T>;
};

class AbstractFragment {
public:
    virtual AbstractInterpolatable& getProperties() = 0;
    virtual const AbstractInterpolatable& getProperties() const = 0;
    virtual ~AbstractFragment() {}
};

template <typename P>
requires Interpolatable<P>
class Fragment: public AbstractFragment {
public:
    P properties;
    Fragment() = default;
    Fragment(const P& properties): properties(properties) {}
    // 1. 对吗？ 2. 能用模板简化吗？
    Fragment(P&& properties): properties(properties) {} 
    
    virtual AbstractInterpolatable& getProperties() override { return properties; }
    virtual const AbstractInterpolatable& getProperties() const override { return properties; }
};

class AbstractVertex {
public:
    Vec3 pos;
    
    AbstractVertex(): pos(Vec3()) {}
    AbstractVertex(const Vec3& pos): pos(pos) {}
    virtual ~AbstractVertex(){}

    virtual AbstractInterpolatable& getProperties() = 0;
    virtual const AbstractInterpolatable& getProperties() const = 0;

    virtual size_t fragment_size() const = 0;
    AbstractFragment& perspective_correct_interpolation(
        double z1, double z2, double z3,
        double k2, const AbstractVertex& v2, 
        double k3, const AbstractVertex& v3,
        void* mem
    ) const {
        auto k1 = 1 - k2 - k3;
        auto zt_inv = k1 / z1 + k2 / z2 + k3 / z3;
        return linear_interpolation(k2/zt_inv, v2, k3/zt_inv, v3, mem);        
    }
    virtual AbstractFragment& linear_interpolation(
        double k2, const AbstractVertex& v2, 
        double k3, const AbstractVertex& v3,
        void* mem
    ) const = 0;
};

template <typename P>
requires Interpolatable<P>
class Vertex: public AbstractVertex {
public:
    Vertex(const Vec3& pos, const P& properties): AbstractVertex(pos), properties(properties) {}
    P properties;
    virtual AbstractInterpolatable& getProperties() override { return properties; }
    virtual const AbstractInterpolatable& getProperties() const override { return properties; }


    virtual size_t fragment_size() const { return sizeof(Fragment<P>); }

    virtual AbstractFragment& linear_interpolation(
        double k2, const AbstractVertex& v2, 
        double k3, const AbstractVertex& v3,
        void* mem
    ) const {
        
        double k1 = 1 - k2 - k3;
        const auto& v1 = *this;
        
        auto& p1 = dynamic_cast<const Vertex&>(v1).properties;
        auto& p2 = dynamic_cast<const Vertex&>(v2).properties;
        auto& p3 = dynamic_cast<const Vertex&>(v3).properties;
        
        auto p = p1 * k1 + p2 * k2 + p3 * k3;

        return *new (mem) Fragment(p); // TODO operator new 要多大空间？
    }
};  

// TODO：Object<DerivedUniform> < Object<BaseUniform>
class AbstractShader {
public:
    virtual ~AbstractShader() {}
    virtual RGBColor shade(const AbstractFragment& fragment, const std::any& uniform) const = 0;
};

template <typename P, typename Uniform>
requires Interpolatable<P>
class Shader: public AbstractShader {
public:
    // Shader 因此是一个抽象类，无法实例化。TODO：可以看一下如何用 Module 阻止用户直接继承 AbstractShader
    virtual RGBColor shade(const Fragment<P>& fragment, const Uniform& uniform) const = 0;
    RGBColor shade(const AbstractFragment& fragment, const std::any& uniform) const {
        auto& n_frag = dynamic_cast<const Fragment<P>&>(fragment);
        auto n_uniform = std::any_cast<Uniform>(uniform);
        return shade(n_frag, n_uniform);
    }
};
// how to use: subclassing a specific Shader<T, U> and than write the shade function

class AbstractObject {
public:
    std::vector<std::tuple<int, int, int>> triangles;
    
    virtual const AbstractVertex& getVertex(int) const = 0;
    virtual const AbstractShader& getShader() const = 0;
    virtual ~AbstractObject() {}
};

// TODO: 没必要和 ShaderT 绑定了
template <typename P, typename Uniform, typename ShaderT>
requires Interpolatable<P> && std::derived_from<ShaderT, Shader<P, Uniform>>
class Object: public AbstractObject {
public:

    Object() {}

    std::vector<Vertex<P>> vertices;
    
    const AbstractVertex& getVertex(int index) const {
        return vertices[index];
    }

    ShaderT shader;
    const AbstractShader& getShader() const {
        return shader;
    }
};


class NothingUniform {};

class NothingProperty: public AbstractInterpolatable {
public:
    NothingProperty inversed() const { return *this; }
    NothingProperty operator*(double) const { return *this; }
    NothingProperty operator+(NothingProperty) const { return *this; }
};