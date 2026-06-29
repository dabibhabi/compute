#include <compute/linalg/hadamard.hpp>
#include <compute/linalg/detail/elementwise.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/linalg/detail/route.hpp>

namespace compute::linalg {

core::Result<void> hadamard(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    return detail::run_with_gpu(
        [&] { return detail::elementwise_binary(A, B, C, [](double a, double b) { return a * b; }); },
        [&] { return detail::gpu::hadamard(A, B, C); },
        [&] { return std::pair{A.size(), std::size_t{0}}; });
}

} // namespace compute::linalg
