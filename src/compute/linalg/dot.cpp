#include <compute/linalg/dot.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <compute/linalg/detail/route.hpp>

namespace compute::linalg {

namespace {

core::Result<double> dot_cpu(std::span<const double> u, std::span<const double> v) noexcept
{
    if (u.size() != v.size())
        return core::make_error(core::ErrorCode::dimension_mismatch, "vector lengths must match");

    double sum = 0.0;
    for (std::size_t i = 0; i < u.size(); ++i)
        sum += u[i] * v[i];
    return sum;
}

} // namespace

core::Result<double> dot(std::span<const double> u, std::span<const double> v) noexcept
{
    return detail::run_scalar_with_gpu(
        [&] { return dot_cpu(u, v); },
        [&] { return detail::gpu::dot(u, v); },
        [&] { return std::pair{u.size(), std::size_t{0}}; });
}

} // namespace compute::linalg
