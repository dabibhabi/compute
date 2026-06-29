#include <compute/linalg/transpose.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/linalg/detail/route.hpp>

namespace compute::linalg {

namespace {

core::Result<void> transpose_cpu(const_matrix_view A, matrix_view B) noexcept
{
    if (B.rows() != A.cols() || B.cols() != A.rows())
        return core::make_error(core::ErrorCode::dimension_mismatch,
                                "output dimensions must be the transpose of input");

    for (std::size_t j = 0; j < A.cols(); ++j)
        for (std::size_t i = 0; i < A.rows(); ++i)
            B(j, i) = A(i, j);

    return {};
}

} // namespace

core::Result<void> transpose(const_matrix_view A, matrix_view B) noexcept
{
    return detail::run_with_gpu(
        [&] { return transpose_cpu(A, B); },
        [&] { return detail::gpu::transpose(A, B); },
        [&] { return std::pair{A.size(), std::size_t{0}}; });
}

} // namespace compute::linalg
