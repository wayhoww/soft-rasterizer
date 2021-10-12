#pragma once

#include "shader.hpp"
#include "matrix.hpp"
#include "transforms.hpp"
#include <vector>
#include <iostream>
#include <cmath>
#include <list>

template <typename T> 
T min3(const T& t1, const T& t2, const T& t3) {
    return std::min(std::min(t1, t2), t3);
}

template <typename T> 
T max3(const T& t1, const T& t2, const T& t3) {
    return std::max(std::max(t1, t2), t3);
}

class ObjectDescriptor {
public:
    std::shared_ptr<AbstractObject> object;
    Mat3 dir; // 3 x 3, [inWorld(objX) inWorld(objY) inWorld(objZ)]
    Vec3 pos; // 3 x 1
};

bool to_left(const Vec2& a, const Vec2& b) {
    return a[0] * b[1] - b[0] * a[1] > 0;
}

bool in_triangle(const Vec2& pt, const Vec2& v1, const Vec2& v2, const Vec2& v3) {
    auto e1 = v1 - v2;
    auto p1 = pt - v2;

    auto e2 = v2 - v3;
    auto p2 = pt - v3;

    auto e3 = v3 - v1;
    auto p3 = pt - v1;

    auto r1 = to_left(e1, p1);
    auto r2 = to_left(e2, p2);
    auto r3 = to_left(e3, p3);

    return r1 == r2 && r2 == r3;
}

std::tuple<double, double, double> bary_centric(
    const Vec3& center, 
    const Vec3& v1, 
    const Vec3& v2, 
    const Vec3& v3
    ) {
    auto area1 = cross_product(center - v2, center - v3).norm2();
    auto area2 = cross_product(center - v3, center - v1).norm2();
    auto area3 = cross_product(center - v1, center - v2).norm2();

    auto area_sum = area1 + area2 + area3;
    double k1 = area1 / area_sum;
    double k2 = area2 / area_sum;
    double k3 = area3 / area_sum;

    return {k1, k2, k3};
}

template <typename T> 
std::vector<std::vector<T>> matrix_of_size(int n_rows, int n_cols, const T& default_value) {
    return std::vector<std::vector<T>>(n_rows, std::vector<T>(n_cols, default_value));
}

double deg_to_rad(double deg) {
    return deg / 180 * acos(-1.0);
}


// Object<Uniform, Attribute, Property, Shader>
// VertexData<Attribute> --- VertexShader<Attribute, Uniform> --> Vertex<Property> --- 
// --- Fragment<Vertex> --> RGBAColor

template<typename Uniform>
class Rasterizer {
    static constexpr int POOL_SIZE = 1024 * 1024 * 4; // 4MB per pool
    std::list<void*> fragment_pools;    // 这里，没有重复利用！
    decltype(fragment_pools.begin()) pool_it = fragment_pools.begin(); // TODO: 顺序
    size_t mem_index = 0;

    void reset_mem() {
        pool_it = fragment_pools.begin();
        mem_index = 0;
    }

    void* alloc_mem(size_t size) {
        if(size > POOL_SIZE) {
            throw "fragment size is too large.";
        }

        if(pool_it != fragment_pools.end() && mem_index + size > POOL_SIZE) {
            pool_it++;
            mem_index = 0;
        }
        
        // 如果当前有 pool，那么空间一定足够
        
        if(pool_it == fragment_pools.end()) {
            pool_it = fragment_pools.insert(pool_it, malloc(POOL_SIZE));
            mem_index = 0;
        }
        
        // 那么当前一定有 pool

        void* out = (void*)((char*)*pool_it + mem_index);
        mem_index += size;
        return out;
    }

    std::vector<ObjectDescriptor> objects;

public:
    Uniform uniform;

    Rasterizer() = default;
    Rasterizer(const Rasterizer& r): objects(r.objects), uniform(r.uniform) {}
    Rasterizer(Rasterizer&& r): objects(std::move(r.objects)), uniform(std::move(r.uniform)) {}
    Rasterizer& operator=(const Rasterizer& r) {
        objects = r.objects;
        uniform = r.uniform;
        return *this;
    }
    ~Rasterizer() {
        for(auto ptr: fragment_pools) {
            free(ptr);
            ptr = nullptr;
        }
    }

    template<typename T, typename P, typename VShaderT, typename FShaderT>
    Rasterizer& addObject(
        const Object<T, P, Uniform, VShaderT, FShaderT>& obj, 
        const Mat3& dir, 
        const Vec3& pos
    ) {
        objects.push_back(ObjectDescriptor{std::make_shared<Object<T, P, Uniform, VShaderT, FShaderT>>(obj), dir, pos});
        return *this;
    }

    Rasterizer& clearObjects() {
        objects.clear();
        return *this;
    }

    Image rasterize(
        const Vec3& camera_pos,
        const Vec3& camera_dir,
        const Vec3& camera_top,
        double near, // should be positive
        double far,  // should be positive
        double field_of_view_y, // 和渲染结果里面的范围有关
        double aspect_ratio,
        int width,           // 和实际的图片大小有关
        int height           // width / height == aspect ratio should hold
    ) /* const cast here */ {

        auto h = 2 * tan(field_of_view_y / 2) * near;
        auto w = aspect_ratio * h;

        Mat4 S {
            { 2.0/w, 0, 0, 0 },
            { 0, 2.0/h, 0, 0 },
            { 0, 0,     1, 0 },
            { 0, 0,     0, 1 }
        };

        auto V = view_transform(camera_pos, camera_dir, camera_top);
        auto P = projection_transform(near, far);
        auto SPV = S * P * V;

        auto f_buffer = matrix_of_size<std::pair<const AbstractFragment*, const AbstractFShader*>>(width, height, std::make_pair(nullptr, nullptr));
        auto d_buffer = matrix_of_size<double>(width, height, far + 1); // TODO: -inf

        RasterizerInfo info;
        info.V = V;
        info.P = S * P;
        info.camera_dir = camera_dir;
        info.camera_top = camera_top;
        info.camera_pos = camera_pos;

        for(auto desp: objects) {
            auto& pObj = desp.object;
            auto& vShader = pObj->getVShader();
            auto& fShader = pObj->getFShader();

            auto M = model_transform(desp.pos, desp.dir);
            info.M = M;

            for(auto [i1, i2, i3]: pObj->triangles) {
                const int VertexSize = pObj->getVShader().vertexSize();
                char* mem = (char*) alloc_mem (VertexSize * 3);
                auto& v1 = vShader.shade(pObj->getVertexData(i1), uniform, info, mem + 0 * VertexSize);
                auto& v2 = vShader.shade(pObj->getVertexData(i2), uniform, info, mem + 1 * VertexSize);
                auto& v3 = vShader.shade(pObj->getVertexData(i3), uniform, info, mem + 2 * VertexSize);

                auto pos1_world_vec4 = M * to_vec4_as_pos(v1.pos_model);
                auto pos2_world_vec4 = M * to_vec4_as_pos(v2.pos_model);
                auto pos3_world_vec4 = M * to_vec4_as_pos(v3.pos_model);

                auto pos1_screen_vec4 = SPV * pos1_world_vec4;
                auto pos2_screen_vec4 = SPV * pos2_world_vec4;
                auto pos3_screen_vec4 = SPV * pos3_world_vec4;

                auto pos1_screen_vec3 = to_vec3_as_pos(pos1_screen_vec4);
                auto pos2_screen_vec3 = to_vec3_as_pos(pos2_screen_vec4);
                auto pos3_screen_vec3 = to_vec3_as_pos(pos3_screen_vec4);

                double fragment_width = 2.0 / width;
                double fragment_height = 2.0 / height;

                int x_min = (min3(pos1_screen_vec3[0], pos2_screen_vec3[0], pos3_screen_vec3[0]) + 1) / fragment_width - 0.5 - 1;
                int x_max = (max3(pos1_screen_vec3[0], pos2_screen_vec3[0], pos3_screen_vec3[0]) + 1) / fragment_width - 0.5 + 2;

                int y_min = (min3(pos1_screen_vec3[1], pos2_screen_vec3[1], pos3_screen_vec3[1]) + 1) / fragment_height - 0.5 - 1;
                int y_max = (max3(pos1_screen_vec3[1], pos2_screen_vec3[1], pos3_screen_vec3[1]) + 1) / fragment_height - 0.5 + 2;
                
                for(int x_index = std::max(0, x_min); x_index < std::min(width, x_max); x_index++) {
                    for(int y_index = std::max(0, y_min); y_index < std::min(height, y_max); y_index++) {
                        double x = x_index * fragment_width + 0.5 * fragment_width - 1;
                        double y = y_index * fragment_height + 0.5 * fragment_height - 1;
                        Vec2 pt {x, y};

                        Vec2 pos1_screen_vec2 = { pos1_screen_vec3[0], pos1_screen_vec3[1] }; 
                        Vec2 pos2_screen_vec2 = { pos2_screen_vec3[0], pos2_screen_vec3[1] }; 
                        Vec2 pos3_screen_vec2 = { pos3_screen_vec3[0], pos3_screen_vec3[1] }; 

                        if(in_triangle(pt, pos1_screen_vec2, pos2_screen_vec2, pos3_screen_vec2)) {
                            Vec3 center {x, y, 0};

                            // 如果我不强制令z=0呢？那就会四点不共面
                            auto [k1, k2, k3] = bary_centric(
                                center, 
                                { pos1_screen_vec2[0], pos1_screen_vec2[1], 0 }, 
                                { pos2_screen_vec2[0], pos2_screen_vec2[1], 0 }, 
                                { pos3_screen_vec2[0], pos3_screen_vec2[1], 0 }
                            );

                            double z1 = pos1_screen_vec3[2];
                            double z2 = pos2_screen_vec3[2];
                            double z3 = pos3_screen_vec3[2];

                            double z = k1 * z1 + k2 * z2 + k3 * z3;;
                            
                            if(z <= far && z >= near && z < d_buffer[x_index][y_index]) {
                                d_buffer[x_index][y_index] = z;

                                auto mem = alloc_mem(v1.fragment_size());
                                
                                auto nk1 = k1 / pos1_screen_vec4[3];
                                auto nk2 = k2 / pos2_screen_vec4[3];
                                auto nk3 = k3 / pos3_screen_vec4[3];
                                auto nksum = nk1 + nk2 + nk3;
                                nk1 /= nksum;
                                nk2 /= nksum;
                                nk3 /= nksum;

                                auto& fragment = v1.linear_interpolation(
                                    nk2, v2,
                                    nk3, v3,
                                    mem
                                );

                                fragment.pos_world =  to_vec3_as_pos(pos1_world_vec4) * nk1 + 
                                                      to_vec3_as_pos(pos2_world_vec4) * nk2 + 
                                                      to_vec3_as_pos(pos3_world_vec4) * nk3 ;

                                f_buffer[x_index][y_index].first = &fragment;
                                f_buffer[x_index][y_index].second = &pObj->getFShader();
                            }   
                        }
                    }
                }
            }
        }

        Image image(width, height);

        for(int x_index = 0; x_index < width; x_index++) {
            for(int y_index = 0; y_index < height; y_index++) {
                auto [fragment, shader] =  f_buffer[x_index][y_index];
                if(fragment != nullptr) {
                    image[{x_index, y_index}] = shader->shade(*fragment, uniform);
                }
            }
        }

        return image;
    }

};