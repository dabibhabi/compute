#pragma once

#include <compute/core/error.hpp>
#include <cublas_v2.h>
#include <cuda_runtime.h>

namespace compute::cuda::detail {

struct Context {
    static Context& instance();

    bool        ready = false;
    cublasHandle_t cublas = nullptr;
    cudaStream_t   stream = nullptr;

    [[nodiscard]] core::Result<void> init() noexcept;
    void shutdown() noexcept;

    [[nodiscard]] core::Result<void*>
    alloc_device(std::size_t bytes) const noexcept;

    [[nodiscard]] core::Result<void>
    copy_h2d(const void* host, void* device, std::size_t bytes) const noexcept;

    [[nodiscard]] core::Result<void>
    copy_d2h(const void* device, void* host, std::size_t bytes) const noexcept;

    [[nodiscard]] core::Result<void> sync() const noexcept;

    void free_device(void* ptr) const noexcept;
};

[[nodiscard]] core::Result<void> check_cuda(cudaError_t err, const char* what) noexcept;
[[nodiscard]] core::Result<void> check_cublas(cublasStatus_t status, const char* what) noexcept;

} // namespace compute::cuda::detail
