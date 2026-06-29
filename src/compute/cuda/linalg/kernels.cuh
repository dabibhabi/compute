#pragma once

#include <cstddef>

enum class CudaElementwiseOp : int { add = 0, sub = 1, scale = 2, hadamard = 3 };

extern "C" {
void cuda_launch_elementwise(const double* A, const double* B, double* C, std::size_t n,
                             int op, double alpha, void* stream);
void cuda_launch_transpose(const double* A, double* B, std::size_t rows, std::size_t cols, void* stream);
void cuda_launch_norm_inf(const double* A, double* row_sums, std::size_t rows, std::size_t cols,
                          void* stream);
}
