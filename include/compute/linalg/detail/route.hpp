#pragma once

#include <compute/core/compute_policy.hpp>
#include <compute/core/error.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <cstddef>
#include <utility>

namespace compute::linalg::detail {

template<typename CpuFn, typename GpuFn, typename SizeFn>
core::Result<void> run_with_gpu(CpuFn&& cpu, GpuFn&& gpu, SizeFn&& size_fn) noexcept
{
    const auto policy = core::get_policy();
    const auto [elements, gemm_n] = size_fn();
    if (gpu::should_offload(policy, elements, gemm_n)) {
        auto r = gpu();
        if (r) return r;
        if (policy == core::ComputePolicy::gpu_preferred)
            return core::make_error(core::ErrorCode::cuda_error, "gpu offload failed");
    }
    return cpu();
}

template<typename CpuFn, typename GpuFn, typename SizeFn>
core::Result<double> run_scalar_with_gpu(CpuFn&& cpu, GpuFn&& gpu, SizeFn&& size_fn) noexcept
{
    const auto policy = core::get_policy();
    const auto [elements, gemm_n] = size_fn();
    if (gpu::should_offload(policy, elements, gemm_n)) {
        auto r = gpu();
        if (r) return r;
        if (policy == core::ComputePolicy::gpu_preferred)
            return core::make_error(core::ErrorCode::cuda_error, "gpu offload failed");
    }
    return cpu();
}

} // namespace compute::linalg::detail
