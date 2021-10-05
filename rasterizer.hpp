#pragma once

#include "shader.hpp"
#include "matrix.hpp"
#include "transforms.hpp"
#include <vector>
#include <iostream>

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

bool to_left(double xa, double ya, double xb, double yb) {
    return xa * yb - xb * ya > 0;
}

bool in_triangle(const Vec2& pt, const Vec2& v1, const Vec2& v2, const Vec2& v3) {
    auto e1 = v1 - v2;
    auto p1 = pt - v2;

    auto e2 = v2 - v3;
    auto p2 = pt - v3;

    auto e3 = v3 - v1;
    auto p3 = pt - v1;

    auto r1 = to_left(e1[{0, 0}], e1[{1, 0}], p1[{0, 0}], p1[{1, 0}]);
    auto r2 = to_left(e2[{0, 0}], e2[{1, 0}], p2[{0, 0}], p2[{1, 0}]);
    auto r3 = to_left(e3[{0, 0}], e3[{1, 0}], p3[{0, 0}], p3[{1, 0}]);

    return r1 == r2 && r2 == r3;
}

template <typename T> 
std::vector<std::vector<T>> matrix_of_size(int n_rows, int n_cols, const T& default_value) {
    return std::vector<std::vector<T>>(n_rows, std::vector<T>(n_cols, default_value));
}

template<typename Uniform>
class Rasterizer {
public:
    std::vector<ObjectDescriptor> objects;
    Uniform uniform;

    Image rasterize(
        const Vec3& camera_pos,
        const Vec3& camera_dir,
        const Vec3& camera_top,
        double near, // should be negative
        double far,  // should be negative
        double field_of_view_y, // 和渲染结果里面的范围有关
        double aspect_ratio,
        int width,           // 和实际的图片大小有关
        int height           // width / height == aspect ratio should hold
    ) const {
        // inspect the feature of projection transform

        auto h = 2 * tan(field_of_view_y / 2) * near;
        auto w = aspect_ratio * h;

        Mat4 Screen;
        Screen[{0, 0}] = 2.0 / w;
        Screen[{1, 1}] = 2.0 / h;
        Screen[{2, 2}] = 1;
        Screen[{3, 3}] = 1; 

        auto V = view_transform(camera_pos, camera_dir, camera_top);
        auto P = projection_transform(near, far);
        auto GlobalTransform = Screen * P * V;

        auto f_buffer = matrix_of_size<std::pair<std::shared_ptr<AbstractFragment>, const AbstractShader*>>(width, height, std::make_pair(nullptr, nullptr));
        auto d_buffer = matrix_of_size<double>(width, height, -1e9);

        for(auto desp: objects) {
            auto& pObj = desp.object;
            auto shader = pObj->getShader();

            auto M = model_transform(desp.pos, desp.dir);
            auto Transform = GlobalTransform * M;

            for(auto [i1, i2, i3]: pObj->triangles) {
                auto& v1 = pObj->getVertex(i1);
                auto& v2 = pObj->getVertex(i2);
                auto& v3 = pObj->getVertex(i3);

                // printf("[%6.2lf %6.2lf %6.2lf]\n", v1.pos[{0, 0}], v1.pos[{1, 0}], v1.pos[{2, 0}]);
                // printf("[%6.2lf %6.2lf %6.2lf]\n", x1[{0, 0}], x1[{1, 0}], x1[{2, 0}]);
                // printf("[%6.2lf %6.2lf %6.2lf]\n", x2[{0, 0}], x2[{1, 0}], x2[{2, 0}]);
                // printf("[%6.2lf %6.2lf %6.2lf]\n", x3[{0, 0}], x3[{1, 0}], x3[{2, 0}]);
                // printf("[%6.2lf %6.2lf %6.2lf]\n", x4[{0, 0}], x4[{1, 0}], x4[{2, 0}]);
                // printf("\n\n");

                auto pos1 = to_vec3_as_pos(Transform * to_vec4_as_pos(v1.pos));
                auto pos2 = to_vec3_as_pos(Transform * to_vec4_as_pos(v2.pos));
                auto pos3 = to_vec3_as_pos(Transform * to_vec4_as_pos(v3.pos));

                // - 1.0 - + 1.0
                double fragment_width = 2.0 / width;
                double fragment_height = 2.0 / height;

                // x_index  = (x + 1) / fragment_width - 0.5
                int x_min = (min3(pos1[{0, 0}], pos2[{0, 0}], pos3[{0, 0}]) + 1) / fragment_width - 0.5 - 1;
                int x_max = (max3(pos1[{0, 0}], pos2[{0, 0}], pos3[{0, 0}]) + 1) / fragment_width - 0.5 + 2;

                int y_min = (min3(pos1[{1, 0}], pos2[{1, 0}], pos3[{1, 0}]) + 1) / fragment_height - 0.5 - 1;
                int y_max = (max3(pos1[{1, 0}], pos2[{1, 0}], pos3[{1, 0}]) + 1) / fragment_height - 0.5 + 2;
                

                // todo: bound to minimal of triangle and the screen
                for(int x_index = std::max(0, x_min); x_index < std::min(width, x_max); x_index++) {
                    for(int y_index = std::max(0, y_min); y_index < std::min(height, y_max); y_index++) {
                        double x = x_index * fragment_width + 0.5 * fragment_width - 1;
                        double y = y_index * fragment_height + 0.5 * fragment_height - 1;
                        Vec2 pt;
                        pt[{0, 0}] = x;
                        pt[{1, 0}] = y;

                        Vec2 vs2dim1; vs2dim1[{0, 0}] = pos1[{0, 0}]; vs2dim1[{1, 0}] = pos1[{1, 0}]; 
                        Vec2 vs2dim2; vs2dim2[{0, 0}] = pos2[{0, 0}]; vs2dim2[{1, 0}] = pos2[{1, 0}]; 
                        Vec2 vs2dim3; vs2dim3[{0, 0}] = pos3[{0, 0}]; vs2dim3[{1, 0}] = pos3[{1, 0}]; 
                        if(in_triangle(pt, vs2dim1, vs2dim2, vs2dim3)) {
                            Vec3 center;
                            center[{0, 0}] = x;
                            center[{1, 0}] = y;
                            center[{2, 0}] = 0;
                            Matrix vs1 = pos1;
                            Matrix vs2 = pos2;
                            Matrix vs3 = pos3;
                            vs1[{2, 0}] = 0;
                            vs2[{2, 0}] = 0;
                            vs3[{2, 0}] = 0;

                            auto area1 = cross_product(center - vs2, center - vs3).norm2();
                            auto area2 = cross_product(center - vs3, center - vs1).norm2();
                            auto area3 = cross_product(center - vs1, center - vs2).norm2();

                            auto area_sum = area1 + area2 + area3;
                            double k1 = area1 / area_sum;
                            double k2 = area2 / area_sum;
                            double k3 = area3 / area_sum;
                            // TODO 透视修正插值

                            double z1 = pos1[{2, 0}];
                            double z2 = pos2[{2, 0}];
                            double z3 = pos3[{2, 0}];

                            double z = k1 * z1 + k2 * z2 + k3 * z3;;
                            
                            if(/*z >= far && z <= near && */z > d_buffer[x_index][y_index]) {
                                d_buffer[x_index][y_index] = z;
                                
                                auto fragment = v1.initFragment();
                                fragment->getProperties() = v1
                                    .getProperties()
                                    .linear_interpolate_with(
                                        k2, v2.getProperties(),
                                        k3, v3.getProperties()
                                    );

                                f_buffer[x_index][y_index].first =  fragment;
                                f_buffer[x_index][y_index].second = &pObj->getShader();
                                //= std::make_pair(, pObj->getShader());
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