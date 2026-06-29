#include <compute/linalg/gemv.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/linalg/detail/route.hpp>

namespace compute::linalg {

namespace {

core::Result<void> gemv_cpu(const_matrix_view A, std::span<const double> x, std::span<double> y) noexcept
{
    if (x.size() != A.cols())
        return core::make_error(core::ErrorCode::dimension_mismatch,
                                "vector x length must match column count of A");
    if (y.size() != A.rows())
        return core::make_error(core::ErrorCode::dimension_mismatch,
                                "vector y length must match row count of A");

    for (std::size_t i = 0; i < A.rows(); ++i) {
        double sum = 0.0;
        for (std::size_t j = 0; j < A.cols(); ++j)
            sum += A(i, j) * x[j];
        y[i] = sum;
    }
    return {};
}

} // namespace

core::Result<void> gemv(const_matrix_view A, std::span<const double> x, std::span<double> y) noexcept
{
    return detail::run_with_gpu(
        [&] { return gemv_cpu(A, x, y); },
        [&] { return detail::gpu::gemv(A, x, y); },
        [&] { return std::pair{A.size(), std::size_t{0}}; });
}

} // namespace compute::linalg
