#include <compute/linalg/scale.hpp>
#include <compute/linalg/detail/elementwise.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/linalg/detail/route.hpp>

namespace compute::linalg {

core::Result<void> scale(double alpha, const_matrix_view A, matrix_view B) noexcept
{
    return detail::run_with_gpu(
        [&] { return detail::elementwise_unary(A, B, [alpha](double x) { return alpha * x; }); },
        [&] { return detail::gpu::scale(alpha, A, B); },
        [&] { return std::pair{A.size(), std::size_t{0}}; });
}

} // namespace compute::linalg
