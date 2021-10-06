#pragma once

#include "matrix.hpp"

// dir: [inWorld(objX) inWorld(objY) inWorld(objZ)]
Mat4 model_transform(Vec3 pos, Mat3 dir) {
    Mat4 mat;
    for(int i = 0; i < 3; i++) 
        for(int j = 0; j < 3; j++) 
            mat[{i, j}] = dir[{i, j}];

    // pos 
    for(int i = 0; i < 3; i++)
        mat[{i, 3}] = pos[{i, 0}];

    mat[{3, 3}] = 1;

    return mat;
}

Mat4 view_transform(Vec3 camera_pos, Vec3 camera_dir, Vec3 camera_top) {
    camera_dir.normalize();
    camera_top.normalize();
    
    auto mat = Mat3::hcat(
        cross_product(camera_dir, camera_top),     //  x -> dir x top
        camera_top,                                //  y -> camera_top
        camera_dir                                 //  z -> camera_dir
    ).transposed(); // already transposed

    Mat4 mat1;
    Mat4 mat2;

    for(int i = 0; i < 3; i++) 
        for(int j = 0; j < 3; j++) 
            mat1[{i, j}] = mat[{i, j}];

    mat1[{3, 3}] = 1;
        
    for(int i = 0; i < 3; i++) 
        mat2[{i, 3}] = -camera_pos[{i, 0}];    
    
    for(int i = 0; i < 4; i++) 
        mat2[{i, i}] = 1.0;

    return mat1 * mat2;
}

Mat4 projection_transform(double near, double far) {
    Mat4 mat;
    mat[{0, 0}] = near;
    mat[{1, 1}] = near;
    
    mat[{2, 2}] = near + far;
    mat[{2, 3}] = - near * far;
    
    mat[{3, 2}] = 1;
    return mat;
}
