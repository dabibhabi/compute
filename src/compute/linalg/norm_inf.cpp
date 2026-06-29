#include <compute/linalg/norm_inf.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/linalg/detail/route.hpp>
#include <cmath>

namespace compute::linalg {

namespace {

core::Result<double> norm_inf_cpu(const_matrix_view A) noexcept
{
    double max_row_sum = 0.0;
    for (std::size_t i = 0; i < A.rows(); ++i) {
        double row_sum = 0.0;
        for (std::size_t j = 0; j < A.cols(); ++j)
            row_sum += std::abs(A(i, j));
        if (row_sum > max_row_sum)
            max_row_sum = row_sum;
    }
    return max_row_sum;
}

} // namespace

core::Result<double> norm_inf(const_matrix_view A) noexcept
{
    return detail::run_scalar_with_gpu(
        [&] { return norm_inf_cpu(A); },
        [&] { return detail::gpu::norm_inf(A); },
        [&] { return std::pair{A.size(), std::size_t{0}}; });
}

} // namespace compute::linalg
