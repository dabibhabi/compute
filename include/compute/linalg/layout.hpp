#pragma once


/**
`layout.hpp` — column-major index `(i,j) → j*m+i`, dimension validation, `Result<void>` helpers

This file contains the layout of the matrix.
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <span>




namespace compute::linalg {
    template<typename T>
    class Matrix {
    private:
        std::span<T> data;
        std::size_t rows;
        std::size_t cols;

        public:
        Matrix(std::size_t rows, std::size_t cols) : data(rows * cols), rows(rows), cols(cols) {}
        Matrix(std::size_t rows, std::size_t cols, std::span<T> data) : data(data), rows(rows), cols(cols) {}
        Matrix(std::size_t rows, std::size_t cols, T* data) : data(data, rows * cols), rows(rows), cols(cols) {}

        std::size_t rows() const { return rows; }
        std::size_t cols() const { return cols; }
        std::span<T> data() const { return data; }
        T* data() { return data.data(); }
        T& operator()(std::size_t i, std::size_t j) { return data[j * rows + i]; }
        const T& operator()(std::size_t i, std::size_t j) const { return data[j * rows + i]; }
        T& operator[](std::size_t i) { return data[i]; }
    }
} // namespace compute::linalg