#pragma once

#include <vector>
#include <cmath>

template<int N, int M>
class Matrix {
    double data[N][M];
public:
    Matrix(){
        memset(data, 0, sizeof(data));
    }

    Matrix(const Matrix& other){
        memcpy(data, other.data, sizeof(data));
    }

    double& operator[](const std::pair<int, int>& index) {
        return data[index.first][index.second];
    }

    double operator[](const std::pair<int, int>& index) const {
        return data[index.first][index.second];   
    }

    //template<>
    double& operator[](int index) requires (N == 1) {
        return data[index][0];
    }

    // /template<>
    double operator[](int index) const requires (N == 1) {
        return data[index][0];
    }

    Matrix<M, N> transposed() { 
        Matrix<M, N> out;
        for(int i = 0; i < N; i++)
            for(int j = 0; j < M; j++)
                out.data[j][i] = data[i][j];
    }

    template<int O>
    Matrix<M, O> operator*(const Matrix<N, O>& rhs) const {
        Matrix<M, O> mat;

        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                for(int k = 0; k < O; k++) {
                    mat[{i, k}] += data[i][j] * rhs[{j,k}];
                }
            }
        }
        return mat;
    }

    Matrix& operator*=(double k) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                data[i][j] *= k;
            }
        }
        return *this;
    }

    Matrix operator*(double k) const {
        Matrix rst = *this;
        rst *= k;
        return rst;
    }

    Matrix& operator+=(const Matrix& rhs) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                data[i][j] += rhs.data[i][j];
            }
        }
        return *this;
    }

    Matrix operator+(const Matrix& rhs) const {
        auto lhs = *this;
        return lhs += rhs;
    }

    Matrix operator-(const Matrix& rhs) const {
        Matrix out = *this;
        return out -= rhs;
    }

    Matrix& operator-=(const Matrix& rhs) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                data[i][j] -= rhs.data[i][j];
            }
        }
        return *this;
    }

    double norm2() const {
        double sum = 0;
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                sum += data[i][j] * data[i][j];
            }
        }
        return sqrt(sum);
    }
};

// 如何只对某个成员函数进行特化？

using Vec2 = Matrix<2, 1>;
using Vec3 = Matrix<3, 1>;
using Vec4 = Matrix<4, 1>;
using Mat3 = Matrix<3, 3>;
using Mat4 = Matrix<4, 4>;

Vec3 cross_product(const Vec3& vec31, const Vec3& vec32) {
    Vec3 mat;
    mat[{0, 0}] = vec31[{1, 0}] * vec32[{2, 0}] - vec32[{1, 0}] * vec31[{2, 0}];
    mat[{1, 0}] = vec31[{2, 0}] * vec32[{0, 0}] - vec32[{2, 0}] * vec31[{0, 0}];
    mat[{2, 0}] = vec31[{0, 0}] * vec32[{1, 0}] - vec32[{0, 0}] * vec31[{1, 0}];
    return mat;
}

Vec4 to_vec4_as_pos(const Vec3& vec) {
    Vec4 mat;
    for(int i = 0; i < 3; i++) mat[{i, 0}] = vec[{i, 0}];
    mat[{3, 0}] = 1;
    return mat;
}

Vec4 to_vec4_as_dir(const Vec3& vec) {
    Vec4 mat;
    for(int i = 0; i < 3; i++) mat[{i, 0}] = vec[{i, 0}];
    return mat;
}

Vec3 to_vec3_as_dir(const Vec4& vec) {
    Vec3 mat;
    for(int i = 0; i < 3; i++) mat[{i, 0}] = vec[{i, 0}];
    return mat;
}

Vec3 to_vec3_as_pos(const Vec4& vec) {
    Vec3 mat;
    for(int i = 0; i < 3; i++) mat[{i, 0}] = vec[{i, 0}] / vec[{3, 0}];
    return mat;
}