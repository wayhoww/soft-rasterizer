#pragma once

#include <vector>
#include <cmath>
#include <initializer_list>
#include <array>

template<int M, int N>
class Matrix {
    double data[M][N];
public:
    static constexpr int ShapeM = M;
    static constexpr int ShapeN = N;

    Matrix(){
        memset(data, 0, sizeof(data));
    }

    Matrix(const Matrix& other){
        memcpy(data, other.data, sizeof(data));
    }

    // 破坏了静态检查。不过 API 会比较易用。
    Matrix(const std::initializer_list<double>& init) requires (N == 1) {
        if(init.size() != M) throw "unmatch size";
        
        int i = 0;
        for(auto val: init) {
            data[i++][0] = val;
        }
    }

    // 破坏了静态检查。不过 API 会比较易用。
    Matrix(const std::initializer_list<std::initializer_list<double>>& init) {
        if(init.size() != M) throw "unmatch size";
        int i = 0, j = 0;
        for(auto row: init) {
            if(row.size() != N) throw "unmatch size";
            for(auto val: row) {
                data[i][j++] = val;
            }
            i++;
            j = 0;
        }
    }

    template <typename ...Cols>
    static Matrix hcat(const Cols&... cols) {
        Matrix target;
        Matrix::make_hcat<0, Cols...>(target, cols...);
        return target;
    }

    template <int StartAt, typename FirstCol, typename ...Cols> 
    static void make_hcat(Matrix& target, const FirstCol& firstcol, const Cols&... cols) 
    requires (StartAt + FirstCol::ShapeN <= N && FirstCol::ShapeM == M) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < FirstCol::ShapeN; j++) {
                target[{i, StartAt + j}] = firstcol[{i, j}];
            }
        }
        make_hcat<StartAt + FirstCol::ShapeN, Cols...>(target, cols...);
    }

    template <int StartAt, typename FirstCol> 
    static void make_hcat(Matrix& target, const FirstCol& firstcol) 
    requires (StartAt + FirstCol::ShapeN == N && FirstCol::ShapeM == M) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < FirstCol::ShapeN; j++) {
                target[{i, StartAt + j}] = firstcol[{i, j}];
            }
        }
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

    Matrix<N, M> transposed() { 
        Matrix<N, M> out;
        for(int i = 0; i < M; i++)
            for(int j = 0; j < N; j++)
                out[{j, i}] = data[i][j];
        return out;
    }

    template<int O>
    Matrix<N, O> operator*(const Matrix<N, O>& rhs) const {
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

    Matrix& operator/=(double x) {
        return (*this) *= 1/x;
    }

    Matrix& normalize() requires ( N == 1 ) {
        return (*this) /= norm2();
    }

    Matrix normalized() requires ( N == 1 ) {
        auto out = *this;
        return out /= norm2();
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
    mat[0] = vec31[1] * vec32[2] - vec32[1] * vec31[2];
    mat[1] = vec31[2] * vec32[0] - vec32[2] * vec31[0];
    mat[2] = vec31[0] * vec32[1] - vec32[0] * vec31[1];
    return mat;
}

Vec4 to_vec4_as_pos(const Vec3& vec) {
    Vec4 mat;
    for(int i = 0; i < 3; i++) mat[i] = vec[i];
    mat[3] = 1;
    return mat;
}

Vec4 to_vec4_as_dir(const Vec3& vec) {
    Vec4 mat;
    for(int i = 0; i < 3; i++) mat[i] = vec[i];
    return mat;
}

Vec3 to_vec3_as_dir(const Vec4& vec) {
    Vec3 mat;
    for(int i = 0; i < 3; i++) mat[i] = vec[i];
    return mat;
}

Vec3 to_vec3_as_pos(const Vec4& vec) {
    Vec3 mat;
    for(int i = 0; i < 3; i++) mat[i] = vec[i] / vec[3];
    return mat;
}