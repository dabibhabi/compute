#include <compute/linalg/trace.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/linalg/detail/route.hpp>

namespace compute::linalg {

namespace {

core::Result<double> trace_cpu(const_matrix_view A) noexcept
{
    if (A.rows() != A.cols())
        return core::make_error(core::ErrorCode::dimension_mismatch, "trace requires a square matrix");

    double sum = 0.0;
    for (std::size_t i = 0; i < A.rows(); ++i)
        sum += A(i, i);
    return sum;
}

} // namespace

core::Result<double> trace(const_matrix_view A) noexcept
{
    return detail::run_scalar_with_gpu(
        [&] { return trace_cpu(A); },
        [&] { return detail::gpu::trace(A); },
        [&] { return std::pair{A.size(), std::size_t{0}}; });
}

} // namespace compute::linalg
