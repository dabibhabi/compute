#include "kernels.cuh"
#include <cuda_runtime.h>
#include <cmath>

__global__ void elementwise_kernel(const double* A, const double* B, double* C, std::size_t n,
                                   int op, double alpha)
{
    const std::size_t i = static_cast<std::size_t>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (i >= n) return;
    switch (op) {
    case 0: C[i] = A[i] + B[i]; break;
    case 1: C[i] = A[i] - B[i]; break;
    case 2: C[i] = alpha * A[i]; break;
    case 3: C[i] = A[i] * B[i]; break;
    default: break;
    }
}

__global__ void transpose_kernel(const double* A, double* B, std::size_t rows, std::size_t cols)
{
    const std::size_t j = static_cast<std::size_t>(blockIdx.x) * blockDim.x + threadIdx.x;
    const std::size_t i = static_cast<std::size_t>(blockIdx.y) * blockDim.y + threadIdx.y;
    if (i >= rows || j >= cols) return;
    B[j * rows + i] = A[i + j * rows];
}

__global__ void norm_inf_kernel(const double* A, double* row_sums, std::size_t rows, std::size_t cols)
{
    extern __shared__ double smem[];
    const std::size_t row = static_cast<std::size_t>(blockIdx.x);
    const std::size_t tid = threadIdx.x;
    if (row >= rows) return;

    double local = 0.0;
    for (std::size_t j = tid; j < cols; j += blockDim.x)
        local += fabs(A[row + j * rows]);

    smem[tid] = local;
    __syncthreads();

    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < static_cast<unsigned>(stride))
            smem[tid] = fmax(smem[tid], smem[tid + stride]);
        __syncthreads();
    }
    if (tid == 0) row_sums[row] = smem[0];
}

extern "C" void cuda_launch_elementwise(const double* A, const double* B, double* C, std::size_t n,
                                        int op, double alpha, void* stream)
{
    const int threads = 256;
    const int blocks  = static_cast<int>((n + threads - 1) / threads);
    elementwise_kernel<<<blocks, threads, 0, static_cast<cudaStream_t>(stream)>>>(A, B, C, n, op, alpha);
}

extern "C" void cuda_launch_transpose(const double* A, double* B, std::size_t rows, std::size_t cols,
                                      void* stream)
{
    dim3 block(16, 16);
    dim3 grid((cols + block.x - 1) / block.x, (rows + block.y - 1) / block.y);
    transpose_kernel<<<grid, block, 0, static_cast<cudaStream_t>(stream)>>>(A, B, rows, cols);
}

extern "C" void cuda_launch_norm_inf(const double* A, double* row_sums, std::size_t rows,
                                     std::size_t cols, void* stream)
{
    const int threads = 256;
    const std::size_t smem = threads * sizeof(double);
    norm_inf_kernel<<<static_cast<int>(rows), threads, smem, static_cast<cudaStream_t>(stream)>>>(
        A, row_sums, rows, cols);
}
