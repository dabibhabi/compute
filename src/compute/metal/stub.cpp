#include <compute/backends/gpu_backend.hpp>
#include <span>

namespace compute::backends::metal {

bool available() noexcept { return false; }

#define STUB_OP(NAME) \
    core::Result<void> NAME(const_matrix_view, const_matrix_view, matrix_view) noexcept { \
        return core::make_error(core::ErrorCode::cuda_error, "metal not available on this platform"); \
    }

#define STUB_OP1(NAME) \
    core::Result<void> NAME(double, const_matrix_view, matrix_view) noexcept { \
        return core::make_error(core::ErrorCode::cuda_error, "metal not available on this platform"); \
    }

#define STUB_SCALAR(NAME) \
    core::Result<double> NAME(const_matrix_view) noexcept { \
        return core::make_error(core::ErrorCode::cuda_error, "metal not available on this platform"); \
    }

STUB_OP(add)
STUB_OP(sub)
STUB_OP1(scale)
STUB_OP(hadamard)
core::Result<void> transpose(const_matrix_view, matrix_view) noexcept {
    return core::make_error(core::ErrorCode::cuda_error, "metal not available on this platform");
}
core::Result<double> dot(std::span<const double>, std::span<const double>) noexcept {
    return core::make_error(core::ErrorCode::cuda_error, "metal not available on this platform");
}
core::Result<void> gemv(const_matrix_view, std::span<const double>, std::span<double>) noexcept {
    return core::make_error(core::ErrorCode::cuda_error, "metal not available on this platform");
}
STUB_OP(gemm)
STUB_SCALAR(trace)
STUB_SCALAR(norm_fro)
STUB_SCALAR(norm_inf)

#undef STUB_OP
#undef STUB_OP1
#undef STUB_SCALAR

} // namespace compute::backends::metal
