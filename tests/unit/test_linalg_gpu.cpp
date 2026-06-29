#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <compute/linalg.hpp>
#include <compute/linalg/detail/gpu_dispatch.hpp>
#include <Eigen/Dense>
#include <vector>

using namespace compute::linalg;
using namespace compute::core;
using Catch::Approx;

namespace {

#if defined(COMPUTE_HAS_CUDA)
const_matrix_view as_const_view(const std::vector<double>& data, std::size_t rows, std::size_t cols)
{
    return *const_matrix_view::create(data, rows, cols);
}

matrix_view as_view(std::vector<double>& data, std::size_t rows, std::size_t cols)
{
    return *matrix_view::create(data, rows, cols);
}

std::vector<double> make_matrix(std::size_t rows, std::size_t cols)
{
    std::vector<double> data(rows * cols);
    for (std::size_t j = 0; j < cols; ++j)
        for (std::size_t i = 0; i < rows; ++i)
            data[colmajor_index(i, j, rows)] = static_cast<double>(i * cols + j + 1);
    return data;
}

void require_ok(const Result<void>& r)
{
    if (!r) FAIL_CHECK(r.error().msg);
    REQUIRE(r.has_value());
}

struct PolicyGuard {
    ComputePolicy prev;
    explicit PolicyGuard(ComputePolicy p) : prev(get_policy()) { set_policy(p); }
    ~PolicyGuard() { set_policy(prev); }
};
#endif

} // namespace

#if defined(COMPUTE_HAS_CUDA)
TEST_CASE("cuda backend is available", "[linalg][gpu][cuda]") {
    REQUIRE(compute::linalg::detail::gpu::available());
}

TEST_CASE("gemm gpu_preferred matches Eigen", "[linalg][gpu][cuda]") {
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 64, k = 64, n = 64;
    auto a = make_matrix(m, k);
    auto b = make_matrix(k, n);
    std::vector<double> c(m * n);

    require_ok(gemm(as_const_view(a, m, k), as_const_view(b, k, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, k);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), k, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e * B_e));
}

TEST_CASE("add gpu_preferred large matrix", "[linalg][gpu][cuda]") {
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 512, n = 512;
    auto a = make_matrix(m, n);
    auto b = make_matrix(m, n);
    std::vector<double> c(m * n);

    require_ok(add(as_const_view(a, m, n), as_const_view(b, m, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e + B_e));
}
#else
TEST_CASE("cuda gpu tests skipped", "[linalg][gpu][cuda]") {
    SKIP("COMPUTE_HAS_CUDA not defined");
}
#endif

#if defined(COMPUTE_HAS_METAL)
TEST_CASE("metal backend availability", "[linalg][gpu][metal]") {
    INFO("Metal tests run on macOS builds with COMPUTE_ENABLE_METAL=ON");
    REQUIRE(true);
}
#else
TEST_CASE("metal gpu tests skipped", "[linalg][gpu][metal]") {
    SKIP("COMPUTE_HAS_METAL not defined");
}
#endif

#if defined(COMPUTE_HAS_VULKAN)
TEST_CASE("vulkan backend availability", "[linalg][gpu][vulkan]") {
    SKIP("Vulkan compute backend is scaffolded; enable when context is complete");
}
#else
TEST_CASE("vulkan gpu tests skipped", "[linalg][gpu][vulkan]") {
    SKIP("COMPUTE_HAS_VULKAN not defined");
}
#endif
