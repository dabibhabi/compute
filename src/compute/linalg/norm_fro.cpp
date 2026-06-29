#include <compute/linalg/norm_fro.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/linalg/detail/route.hpp>
#include <cmath>

namespace compute::linalg {

namespace {

core::Result<double> norm_fro_cpu(const_matrix_view A) noexcept
{
    double sum_sq = 0.0;
    for (std::size_t j = 0; j < A.cols(); ++j)
        for (std::size_t i = 0; i < A.rows(); ++i) {
            const double v = A(i, j);
            sum_sq += v * v;
        }
    return std::sqrt(sum_sq);
}

} // namespace

core::Result<double> norm_fro(const_matrix_view A) noexcept
{
    return detail::run_scalar_with_gpu(
        [&] { return norm_fro_cpu(A); },
        [&] { return detail::gpu::norm_fro(A); },
        [&] { return std::pair{A.size(), std::size_t{0}}; });
}

} // namespace compute::linalg
