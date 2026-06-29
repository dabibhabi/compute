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

#if defined(COMPUTE_HAS_CUDA) || defined(COMPUTE_HAS_METAL)
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

std::vector<double> make_vector(std::size_t n)
{
    std::vector<double> v(n);
    for (std::size_t i = 0; i < n; ++i) v[i] = static_cast<double>((i % 13) + 1);
    return v;
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
// Metal computes in float32 (MSL has no double / Apple GPUs lack float64), so the
// oracle tolerance is the float32 bound, not Eigen's default 1e-12. For an inner
// product of length k the relative error is ~k*eps32 (eps32 = 2^-24 ~ 6e-8); with
// k <= 512 that is ~3e-5, so accumulating ops use 1e-4 and pure element-wise ops
// (a single rounding) use 1e-5.
namespace {
constexpr double metal_elem_tol = 1e-5;
constexpr double metal_accum_tol = 1e-4;

bool metal_ready() { return compute::linalg::detail::gpu::available(); }
} // namespace

TEST_CASE("metal backend is available", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device on this machine");
    REQUIRE(compute::linalg::detail::gpu::available());
}

TEST_CASE("metal add matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 512, n = 512;
    auto a = make_matrix(m, n);
    auto b = make_matrix(m, n);
    std::vector<double> c(m * n);
    require_ok(add(as_const_view(a, m, n), as_const_view(b, m, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e + B_e, metal_elem_tol));
}

TEST_CASE("metal sub matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 128, n = 96;
    auto a = make_matrix(m, n);
    auto b = make_matrix(m, n);
    std::vector<double> c(m * n);
    require_ok(sub(as_const_view(a, m, n), as_const_view(b, m, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e - B_e, metal_elem_tol));
}

TEST_CASE("metal scale matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 128, n = 128;
    const double alpha = 2.5;
    auto a = make_matrix(m, n);
    std::vector<double> b(m * n);
    require_ok(scale(alpha, as_const_view(a, m, n), as_view(b, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> B_e(b.data(), m, n);
    REQUIRE(B_e.isApprox(alpha * A_e, metal_elem_tol));
}

TEST_CASE("metal hadamard matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 100, n = 100;
    auto a = make_matrix(m, n);
    auto b = make_matrix(m, n);
    std::vector<double> c(m * n);
    require_ok(hadamard(as_const_view(a, m, n), as_const_view(b, m, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e.cwiseProduct(B_e), metal_accum_tol));
}

TEST_CASE("metal transpose matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 64, n = 48;
    auto a = make_matrix(m, n);
    std::vector<double> b(n * m);
    require_ok(transpose(as_const_view(a, m, n), as_view(b, n, m)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> B_e(b.data(), n, m);
    REQUIRE(B_e.isApprox(A_e.transpose(), metal_elem_tol));
}

TEST_CASE("metal gemm matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 64, k = 64, n = 64;
    auto a = make_matrix(m, k);
    auto b = make_matrix(k, n);
    std::vector<double> c(m * n);
    require_ok(gemm(as_const_view(a, m, k), as_const_view(b, k, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, k);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), k, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e * B_e, metal_accum_tol));
}

TEST_CASE("metal gemv matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 128, n = 96;
    auto a = make_matrix(m, n);
    auto x = make_vector(n);
    std::vector<double> y(m);
    require_ok(gemv(as_const_view(a, m, n), std::span<const double>(x), std::span<double>(y)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<const Eigen::VectorXd> x_e(x.data(), n);
    Eigen::Map<Eigen::VectorXd> y_e(y.data(), m);
    REQUIRE(y_e.isApprox(A_e * x_e, metal_accum_tol));
}

TEST_CASE("metal dot matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t n = 4096;
    auto u = make_vector(n);
    auto v = make_vector(n);
    auto r = dot(std::span<const double>(u), std::span<const double>(v));
    REQUIRE(r.has_value());

    Eigen::Map<const Eigen::VectorXd> u_e(u.data(), n);
    Eigen::Map<const Eigen::VectorXd> v_e(v.data(), n);
    REQUIRE(*r == Approx(u_e.dot(v_e)).epsilon(metal_accum_tol));
}

TEST_CASE("metal trace matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t n = 256;
    auto a = make_matrix(n, n);
    auto r = trace(as_const_view(a, n, n));
    REQUIRE(r.has_value());

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), n, n);
    REQUIRE(*r == Approx(A_e.trace()).epsilon(metal_accum_tol));
}

TEST_CASE("metal norm_fro matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 128, n = 96;
    auto a = make_matrix(m, n);
    auto r = norm_fro(as_const_view(a, m, n));
    REQUIRE(r.has_value());

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    REQUIRE(*r == Approx(A_e.norm()).epsilon(metal_accum_tol));
}

TEST_CASE("metal norm_inf matches Eigen", "[linalg][gpu][metal]") {
    if (!metal_ready()) SKIP("no Metal device");
    PolicyGuard guard(ComputePolicy::gpu_preferred);

    constexpr std::size_t m = 128, n = 96;
    auto a = make_matrix(m, n);
    auto r = norm_inf(as_const_view(a, m, n));
    REQUIRE(r.has_value());

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    const double expected = A_e.cwiseAbs().rowwise().sum().maxCoeff();
    REQUIRE(*r == Approx(expected).epsilon(metal_accum_tol));
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
