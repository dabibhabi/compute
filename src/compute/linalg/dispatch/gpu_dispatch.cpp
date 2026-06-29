#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/backends/gpu_backend.hpp>
#include <compute/core/compute_policy.hpp>

namespace compute::linalg::detail::gpu {

using core::ComputePolicy;
using core::get_policy;

bool available() noexcept
{
#if defined(COMPUTE_HAS_CUDA)
    if (backends::cuda::available()) return true;
#endif
#if defined(COMPUTE_HAS_METAL)
    if (backends::metal::available()) return true;
#endif
#if defined(COMPUTE_HAS_VULKAN)
    if (backends::vulkan::available()) return true;
#endif
    return false;
}

bool should_offload(ComputePolicy policy, std::size_t elements, std::size_t gemm_n) noexcept
{
    if (policy == ComputePolicy::cpu_only) return false;
    if (!available()) return false;
    if (policy == ComputePolicy::gpu_preferred) return true;
    if (gemm_n > 0) return gemm_n >= gemm_n_threshold;
    if (elements >= element_threshold) return true;
    return elements >= vector_threshold;
}

namespace {

template<typename Ops>
core::Result<void> run_void(const Ops& ops) noexcept
{
#if defined(COMPUTE_HAS_CUDA)
    if (backends::cuda::available()) {
        if (auto r = ops.cuda(); r) return r;
    }
#endif
#if defined(COMPUTE_HAS_METAL)
    if (backends::metal::available()) {
        if (auto r = ops.metal(); r) return r;
    }
#endif
#if defined(COMPUTE_HAS_VULKAN)
    if (backends::vulkan::available()) {
        if (auto r = ops.vulkan(); r) return r;
    }
#endif
    return core::make_error(core::ErrorCode::cuda_error, "gpu backend failed");
}

template<typename Ops>
core::Result<double> run_scalar(const Ops& ops) noexcept
{
#if defined(COMPUTE_HAS_CUDA)
    if (backends::cuda::available()) {
        if (auto r = ops.cuda(); r) return r;
    }
#endif
#if defined(COMPUTE_HAS_METAL)
    if (backends::metal::available()) {
        if (auto r = ops.metal(); r) return r;
    }
#endif
#if defined(COMPUTE_HAS_VULKAN)
    if (backends::vulkan::available()) {
        if (auto r = ops.vulkan(); r) return r;
    }
#endif
    return core::make_error(core::ErrorCode::cuda_error, "gpu backend failed");
}

template<typename Ops>
core::Result<void> run_void_preferred(const Ops& ops) noexcept
{
#if defined(COMPUTE_HAS_CUDA)
    if (backends::cuda::available()) return ops.cuda();
#endif
#if defined(COMPUTE_HAS_METAL)
    if (backends::metal::available()) return ops.metal();
#endif
#if defined(COMPUTE_HAS_VULKAN)
    if (backends::vulkan::available()) return ops.vulkan();
#endif
    return core::make_error(core::ErrorCode::cuda_error, "gpu_preferred but no backend available");
}

template<typename Ops>
core::Result<double> run_scalar_preferred(const Ops& ops) noexcept
{
#if defined(COMPUTE_HAS_CUDA)
    if (backends::cuda::available()) return ops.cuda();
#endif
#if defined(COMPUTE_HAS_METAL)
    if (backends::metal::available()) return ops.metal();
#endif
#if defined(COMPUTE_HAS_VULKAN)
    if (backends::vulkan::available()) return ops.vulkan();
#endif
    return core::make_error(core::ErrorCode::cuda_error, "gpu_preferred but no backend available");
}

template<typename Ops>
core::Result<void> dispatch_void(const Ops& ops) noexcept
{
    if (get_policy() == ComputePolicy::gpu_preferred) return run_void_preferred(ops);
    return run_void(ops);
}

template<typename Ops>
core::Result<double> dispatch_scalar(const Ops& ops) noexcept
{
    if (get_policy() == ComputePolicy::gpu_preferred) return run_scalar_preferred(ops);
    return run_scalar(ops);
}

#define CUDA_OP(NAME, ...) \
    core::Result<void> cuda() const noexcept { return backends::cuda::NAME(__VA_ARGS__); }
#define METAL_OP(NAME, ...) \
    core::Result<void> metal() const noexcept { return backends::metal::NAME(__VA_ARGS__); }
#define VULKAN_OP(NAME, ...) \
    core::Result<void> vulkan() const noexcept { return backends::vulkan::NAME(__VA_ARGS__); }

#define CUDA_SCALAR(NAME, ...) \
    core::Result<double> cuda() const noexcept { return backends::cuda::NAME(__VA_ARGS__); }
#define METAL_SCALAR(NAME, ...) \
    core::Result<double> metal() const noexcept { return backends::metal::NAME(__VA_ARGS__); }
#define VULKAN_SCALAR(NAME, ...) \
    core::Result<double> vulkan() const noexcept { return backends::vulkan::NAME(__VA_ARGS__); }

#if !defined(COMPUTE_HAS_CUDA)
#define CUDA_OP(NAME, ...) \
    core::Result<void> cuda() const noexcept { \
        return core::make_error(core::ErrorCode::cuda_error, "cuda not compiled"); }
#define CUDA_SCALAR(NAME, ...) \
    core::Result<double> cuda() const noexcept { \
        return core::make_error(core::ErrorCode::cuda_error, "cuda not compiled"); }
#endif
#if !defined(COMPUTE_HAS_METAL)
#define METAL_OP(NAME, ...) \
    core::Result<void> metal() const noexcept { \
        return core::make_error(core::ErrorCode::cuda_error, "metal not compiled"); }
#define METAL_SCALAR(NAME, ...) \
    core::Result<double> metal() const noexcept { \
        return core::make_error(core::ErrorCode::cuda_error, "metal not compiled"); }
#endif
#if !defined(COMPUTE_HAS_VULKAN)
#define VULKAN_OP(NAME, ...) \
    core::Result<void> vulkan() const noexcept { \
        return core::make_error(core::ErrorCode::cuda_error, "vulkan not compiled"); }
#define VULKAN_SCALAR(NAME, ...) \
    core::Result<double> vulkan() const noexcept { \
        return core::make_error(core::ErrorCode::cuda_error, "vulkan not compiled"); }
#endif

} // namespace

core::Result<void> add(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    struct Ops {
        const_matrix_view A, B;
        matrix_view       C;
        CUDA_OP(add, A, B, C)
        METAL_OP(add, A, B, C)
        VULKAN_OP(add, A, B, C)
    };
    return dispatch_void(Ops{A, B, C});
}

core::Result<void> sub(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    struct Ops {
        const_matrix_view A, B;
        matrix_view       C;
        CUDA_OP(sub, A, B, C)
        METAL_OP(sub, A, B, C)
        VULKAN_OP(sub, A, B, C)
    };
    return dispatch_void(Ops{A, B, C});
}

core::Result<void> scale(double alpha, const_matrix_view A, matrix_view B) noexcept
{
    struct Ops {
        double            alpha;
        const_matrix_view A;
        matrix_view       B;
        CUDA_OP(scale, alpha, A, B)
        METAL_OP(scale, alpha, A, B)
        VULKAN_OP(scale, alpha, A, B)
    };
    return dispatch_void(Ops{alpha, A, B});
}

core::Result<void> hadamard(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    struct Ops {
        const_matrix_view A, B;
        matrix_view       C;
        CUDA_OP(hadamard, A, B, C)
        METAL_OP(hadamard, A, B, C)
        VULKAN_OP(hadamard, A, B, C)
    };
    return dispatch_void(Ops{A, B, C});
}

core::Result<void> transpose(const_matrix_view A, matrix_view B) noexcept
{
    struct Ops {
        const_matrix_view A;
        matrix_view       B;
        CUDA_OP(transpose, A, B)
        METAL_OP(transpose, A, B)
        VULKAN_OP(transpose, A, B)
    };
    return dispatch_void(Ops{A, B});
}

core::Result<double> dot(std::span<const double> u, std::span<const double> v) noexcept
{
    struct Ops {
        std::span<const double> u, v;
        CUDA_SCALAR(dot, u, v)
        METAL_SCALAR(dot, u, v)
        VULKAN_SCALAR(dot, u, v)
    };
    return dispatch_scalar(Ops{u, v});
}

core::Result<void> gemv(const_matrix_view A, std::span<const double> x, std::span<double> y) noexcept
{
    struct Ops {
        const_matrix_view       A;
        std::span<const double> x;
        std::span<double>       y;
        CUDA_OP(gemv, A, x, y)
        METAL_OP(gemv, A, x, y)
        VULKAN_OP(gemv, A, x, y)
    };
    return dispatch_void(Ops{A, x, y});
}

core::Result<void> gemm(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    struct Ops {
        const_matrix_view A, B;
        matrix_view       C;
        CUDA_OP(gemm, A, B, C)
        METAL_OP(gemm, A, B, C)
        VULKAN_OP(gemm, A, B, C)
    };
    return dispatch_void(Ops{A, B, C});
}

core::Result<double> trace(const_matrix_view A) noexcept
{
    struct Ops {
        const_matrix_view A;
        CUDA_SCALAR(trace, A)
        METAL_SCALAR(trace, A)
        VULKAN_SCALAR(trace, A)
    };
    return dispatch_scalar(Ops{A});
}

core::Result<double> norm_fro(const_matrix_view A) noexcept
{
    struct Ops {
        const_matrix_view A;
        CUDA_SCALAR(norm_fro, A)
        METAL_SCALAR(norm_fro, A)
        VULKAN_SCALAR(norm_fro, A)
    };
    return dispatch_scalar(Ops{A});
}

core::Result<double> norm_inf(const_matrix_view A) noexcept
{
    struct Ops {
        const_matrix_view A;
        CUDA_SCALAR(norm_inf, A)
        METAL_SCALAR(norm_inf, A)
        VULKAN_SCALAR(norm_inf, A)
    };
    return dispatch_scalar(Ops{A});
}

} // namespace compute::linalg::detail::gpu
