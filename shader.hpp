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
    Vec3 pos;
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
    Vertex() = default;
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

        auto frag = new (mem) Fragment(p); // TODO operator new 要多大空间？(对的)
       // frag->pos = this->pos * k1 + v2.pos * k2 + v3.pos * k3;

        return *frag;
    }
};  

// TODO：Object<DerivedUniform> < Object<BaseUniform>
class AbstractFShader {
public:
    virtual ~AbstractFShader() {}
    virtual RGBAColor shade(const AbstractFragment& fragment, const std::any& uniform) const = 0;
};

template <typename P, typename Uniform>
requires Interpolatable<P>
class FShader: public AbstractFShader {
public:
    // Shader 因此是一个抽象类，无法实例化。TODO：可以看一下如何用 Module 阻止用户直接继承 AbstractShader
    virtual RGBAColor shade(const Fragment<P>& fragment, const Uniform& uniform) const = 0;
    virtual RGBAColor shade(const AbstractFragment& fragment, const std::any& uniform) const override {
        auto& n_frag = dynamic_cast<const Fragment<P>&>(fragment);
        auto n_uniform = std::any_cast<Uniform>(uniform);
        return shade(n_frag, n_uniform);
    }
};
// how to use: subclassing a specific Shader<T, U> and than write the shade function


// TODO：Object<DerivedUniform> < Object<BaseUniform>
class AbstractVShader {
public:
    virtual ~AbstractVShader() {}
    virtual AbstractVertex& shade(const std::any& data, const std::any& uniform, const Mat4& M, const Vec3& camera_pos, void* mem) const = 0;
    virtual size_t vertexSize() const = 0; 
};

template <typename VertexDataT, typename P, typename Uniform>
requires Interpolatable<P>
class VShader: public AbstractVShader {
public:
    virtual Vertex<P> shade(const VertexDataT& data, const Uniform& uniform, const Mat4& M, const Vec3& camera_pos) const = 0;
    virtual size_t vertexSize() const override {
        return sizeof(Vertex<P>);
    }
    // TODO 这个 M 应该怎么处理比较好
    virtual AbstractVertex& shade(
        const std::any& data, 
        const std::any& uniform, 
        const Mat4& M, 
        const Vec3& camera_pos,
        void* mem
    ) const {
        auto n_data = std::any_cast<VertexDataT>(data);
        auto n_uniform = std::any_cast<Uniform>(uniform);
        Vertex<P> vertex = shade(n_data, n_uniform, M, camera_pos);
        memcpy(mem, &vertex, sizeof(vertex));
        return *static_cast<AbstractVertex*>(mem);
    }
};

class AbstractObject {
public:
    std::vector<std::tuple<int, int, int>> triangles;
    
    virtual const std::any getVertexData(int) const = 0;
    virtual const AbstractVShader& getVShader() const = 0;
    virtual const AbstractFShader& getFShader() const = 0;
    virtual ~AbstractObject() {}
};


// TODO: 没必要和 ShaderT 绑定了
template <typename A, typename P, typename Uniform, typename VShaderT, typename FShaderT>
requires Interpolatable<P> && std::derived_from<FShaderT, FShader<P, Uniform>>
                           && std::derived_from<VShaderT, VShader<A, P, Uniform>>
class Object: public AbstractObject {
public:
    VShaderT vshader;
    FShaderT fshader;
    std::vector<A> vertices;
    
    Object() {}

    virtual const std::any getVertexData(int index) const override {
        A vert = vertices[index];
        std::any val = vert;
        return val;
    }

    virtual const AbstractVShader& getVShader() const override {
        return vshader;
    }
    virtual const AbstractFShader& getFShader() const override {
        return fshader;
    }
};


class NothingUniform {};

class NothingProperty: public AbstractInterpolatable {
public:
    NothingProperty inversed() const { return *this; }
    NothingProperty operator*(double) const { return *this; }
    NothingProperty operator+(NothingProperty) const { return *this; }
};