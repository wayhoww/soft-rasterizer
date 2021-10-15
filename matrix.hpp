#pragma once

#include <vector>
#include <cmath>
#include <initializer_list>
#include <array>

template<int M, int N, typename ConstT>
requires ( std::same_as<ConstT, float> || std::same_as<ConstT, const float> )
class MatrixSlice {
public:
    ConstT* data[M][N];
    static constexpr bool NOT_CONST = std::same_as<ConstT, float>;

    float& operator[](const std::pair<int, int>& index)
    requires (NOT_CONST) {
        return *data[index.first][index.second];
    }

    float operator[](const std::pair<int, int>& index) const {
        return *data[index.first][index.second];   
    }

    MatrixSlice& operator=(const MatrixSlice& slice) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                *data[i][j] = *slice.data[i][j];
            }
        }
    }

    // 语义上限制一下对const slice对象的内容的访问
    // 怎么整？
    // MatrixSlice& operator-=(const Matrix<M, N>& mat);

    template <typename ConstT2>
    MatrixSlice& operator-=(const MatrixSlice<M, N, ConstT2>& mat) {
        for(int i = 0; i < M; i++)
            for(int j = 0; j < N; j++)
                *data[i][j] -= *mat.data[i][j];
        return *this;
    }

    
    MatrixSlice& operator/=(float k) {
        for(int i = 0; i < M; i++)
            for(int j = 0; j < N; j++)
                *data[i][j] /= k;
        return *this;
    }

    MatrixSlice() = default;
    MatrixSlice(const MatrixSlice&) = default;
    MatrixSlice(MatrixSlice&&) = delete; // 如何直接 delete 一个外部的？？
    MatrixSlice& operator=(MatrixSlice&&) = delete; 
};

namespace std {
    template<int M, int N>
    void swap(const MatrixSlice<M, N, float> slice1, const MatrixSlice<M, N, float>& slice2) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                swap(*slice1.data[i][j], *slice1.data[i][j]);
            }
        }
    }
}

template<int M, int N>
class Matrix {
    float data[M][N];
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
    Matrix(const std::initializer_list<float>& init) requires (N == 1) {
        if(init.size() != M) throw "unmatch size";
        
        int i = 0;
        for(auto val: init) {
            data[i++][0] = val;
        }
    }

    // 破坏了静态检查。不过 API 会比较易用。
    Matrix(const std::initializer_list<std::initializer_list<float>>& init) {
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

    Matrix(const MatrixSlice<M, N, float>& slice) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                data[i][j] = slice[{i, j}];
            }
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

    float& operator[](const std::pair<int, int>& index) {
        return data[index.first][index.second];
    }

    float operator[](const std::pair<int, int>& index) const {
        return data[index.first][index.second];   
    }

    //template<>
    float& operator[](int index) requires (N == 1) {
        return data[index][0];
    }

    // /template<>
    float operator[](int index) const requires (N == 1) {
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

    Matrix& operator*=(float k) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                data[i][j] *= k;
            }
        }
        return *this;
    }

    Matrix operator*(float k) const {
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

    template <typename ConstT>
    Matrix& operator-=(const MatrixSlice<M, N, ConstT>& rhs) {
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                data[i][j] -= rhs[{i, j}];
            }
        }
        return *this;
    }

    static Matrix Identity() 
    requires (M == N){
        Matrix out;
        for(int i = 0; i < M; i++) {
            out[{i, i}] = 1;
        }
        return out;
    };

    float norm2_squared() const {
        float sum = 0;
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                sum += data[i][j] * data[i][j];
            }
        }
        return sum;
    }

    float norm2() const {
        return sqrt(norm2_squared());
    }

    Matrix& operator/=(float x) {
        return (*this) *= 1/x;
    }

    Matrix& normalize() requires ( N == 1 ) {
        return (*this) /= norm2();
    }

    Matrix normalized() const requires ( N == 1 ) {
        auto out = *this;
        return out /= norm2();
    }

    Matrix inversed() const requires (M == N) {
        Matrix L = *this;
        Matrix R = Matrix::Identity();
        for(int i = 0; i < N - 1; i++) {
            // make sure R[i][i] != 0
            int max_abs_index = i;
            float max_abs_val = fabs(R[{i, i}]);

            for(int j = i + 1; j < N; j++) {
                // 只要别带来数值精度问题，都没啥关系。float：1e-38
                if(max_abs_val > 1e-10) break;
                double abs_val = fabs(R[{j, i}]);
                if(abs_val > max_abs_val) {
                    max_abs_val = abs_val;
                    max_abs_index = j;
                }
            }

            if(max_abs_index != i) {
                std::swap(L.row(i), L.row(max_abs_index));
                std::swap(R.row(i), R.row(max_abs_index));
            }

            float head = L[{i, i}];
            for(int j = i + 1; j < N; j++) {
                float div = L[{j, i}] / head;
                // TODO
                auto temp1 = (L.row(i) * div);
                auto temp2 = (R.row(i) * div);
                L.row(j) -= temp1.row(0);
                R.row(j) -= temp2.row(0);
            }
        }

        for(int i = N - 1; i > 0; i--) {
            float head = L[{i, i}];
            for(int j = i - 1; j >= 0; j--) {
                float div = L[{j, i}] / head;
           //     auto temp1 = (L.row(i) * div);
                auto temp2 = (R.row(i) * div);
           //     L.row(j) -= temp1.row(0);
                R.row(j) -= temp2.row(0);
            }
        }

        for(int i = 0; i < N; i++) {
            float div = L[{i, i}];
      //      L.row(i) /= div;
            R.row(i) /= div;
        }

        return R;
    }

    MatrixSlice<1, N, float> row(int i) {
        MatrixSlice<1, N, float> out;
        for(int j = 0; j < N; j++) {
            out.data[0][j] = &data[i][j]; // TODO ????
        }
        return out;
    }

    MatrixSlice<M, 1, float> col(int j) {
        MatrixSlice<M, 1, float> out;
        for(int i = 0; i < M; i++) {
            out[{i, 0}] = &(*this)[{i, j}];
        }
        return out;
    }
};

template<int M, int N>
Matrix<M, N> operator*(const MatrixSlice<M, N, float>& slice, float k) {
    return Matrix<M, N>(slice) * k;
}

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

template <int dim> 
float dot_product(const Matrix<dim, 1>& vec1, const Matrix<dim, 1>& vec2) {
    float sum = 0;
    for(int i = 0; i < dim; i++) {
        sum += vec1[i] * vec2[i];
    }
    return sum;
};

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

