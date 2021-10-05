#pragma once

#include <concepts>
#include <algorithm>
#include <memory>
#include "matrix.hpp"
#include "image.hpp"

class AbstractInterpolatable {
public:
    virtual ~AbstractInterpolatable() {}
    virtual void linear_interpolate_with (
        double k2, const AbstractInterpolatable& a2, 
        double k3, const AbstractInterpolatable& a3,
        AbstractInterpolatable& dest
    ) const {  }
};

template <typename T>
concept Interpolatable = requires(T a, double k) {
    //{ a.linear_interpolate_with(k, a, k, a) } -> std::same_as<T>;
    // can i add a function for T????
    // 可以要求上面的功能，然后把虚接口交给 vertex
    std::derived_from<T, AbstractInterpolatable>;
};

class AbstractFragment {
    AbstractInterpolatable forrtn;
public:
    virtual AbstractInterpolatable& getProperties() { return forrtn; }
    virtual AbstractInterpolatable getProperties() const {
        return getProperties();
    }
    virtual ~AbstractFragment() {}
};

template <typename P>
requires Interpolatable<P>
class Fragment: public AbstractFragment {
public:
    P properties;
    virtual AbstractInterpolatable& getProperties() override { return properties; }
};

class AbstractVertex {
    AbstractInterpolatable forrtn;
public:
    Vec3 pos;
    
    AbstractVertex(): pos(Vec3()) {}
    AbstractVertex(const Vec3& pos): pos(pos) {}
    virtual AbstractInterpolatable& getProperties() {
        return forrtn;
    }
    virtual const AbstractInterpolatable& getProperties() const {
        return forrtn;
    }
    virtual ~AbstractVertex(){}
    virtual std::shared_ptr<AbstractFragment> initFragment() const {
        return std::make_shared<AbstractFragment>();
    }
};

template <typename P>
requires Interpolatable<P>
class Vertex: public AbstractVertex {
public:
    Vertex(const Vec3& pos, const P& properties): AbstractVertex(pos), properties(properties) {}
    P properties;
    virtual AbstractInterpolatable& getProperties() override { return properties; }
    virtual const AbstractInterpolatable& getProperties() const override { return properties; }
    virtual std::shared_ptr<AbstractFragment> initFragment() const {
        return std::make_shared<Fragment<P>>();
    }
};  



class AbstractUniform {
public:
    virtual ~AbstractUniform() {}
};

class AbstractShader {
public:
    virtual RGBColor shade(const AbstractFragment& fragment, const AbstractUniform& uniform) const { 
        return RGBColor{0, 0, 0};
    }
};

template <typename P, typename Uniform>
requires Interpolatable<P> && std::derived_from<Uniform, AbstractUniform>
class Shader: public AbstractShader {
public:
    virtual RGBColor shade(const Fragment<P>& fragment, const Uniform& uniform) const {
        return RGBColor{0, 0, 0};
    }
    RGBColor shade(const AbstractFragment& fragment, const AbstractUniform& uniform) const {
        auto n_frag = dynamic_cast<const Fragment<P>&>(fragment);
        auto n_uniform = dynamic_cast<const Uniform&>(uniform);
        return shade(n_frag, n_uniform);
    }
};
// how to use: subclassing a specific Shader<T, U> and than write the shade function

class AbstractObject {
    AbstractVertex forrtn1;
    AbstractShader forrtn2;
public:
    std::vector<std::tuple<int, int, int>> triangles;
    
    virtual const AbstractVertex& getVertex(int) const { return forrtn1; }
    virtual const AbstractShader& getShader() const { return forrtn2; }
    virtual ~AbstractObject() {}
};

template <typename P, typename Uniform, typename ShaderT>
requires Interpolatable<P> && 
        std::derived_from<Uniform, AbstractUniform> && 
        std::derived_from<ShaderT, Shader<P, Uniform>>
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




class NothingUniform: public AbstractUniform {};

class NothingProperty: public AbstractInterpolatable {};