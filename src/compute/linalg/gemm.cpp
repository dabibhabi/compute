#include <compute/linalg/gemm.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/linalg/detail/route.hpp>

namespace compute::linalg {

namespace {

core::Result<void> gemm_cpu(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    const std::size_t m = A.rows();
    const std::size_t k = A.cols();
    const std::size_t n = B.cols();

    if (B.rows() != k)
        return core::make_error(core::ErrorCode::dimension_mismatch,
                                "inner dimensions of A and B must match");
    if (C.rows() != m || C.cols() != n)
        return core::make_error(core::ErrorCode::dimension_mismatch,
                                "output dimensions must be m x n");

    for (std::size_t j = 0; j < n; ++j)
        for (std::size_t i = 0; i < m; ++i)
            C(i, j) = 0.0;

    for (std::size_t i = 0; i < m; ++i)
        for (std::size_t j = 0; j < n; ++j)
            for (std::size_t p = 0; p < k; ++p)
                C(i, j) += A(i, p) * B(p, j);

    return {};
}

} // namespace

core::Result<void> gemm(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    const std::size_t n = B.cols();
    return detail::run_with_gpu(
        [&] { return gemm_cpu(A, B, C); },
        [&] { return detail::gpu::gemm(A, B, C); },
        [&] { return std::pair{C.size(), n}; });
}

} // namespace compute::linalg
