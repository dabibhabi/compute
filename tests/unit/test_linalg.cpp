#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <compute/linalg.hpp>
#include <Eigen/Dense>
#include <vector>

using namespace compute::linalg;
using namespace compute::core;
using Catch::Approx;

namespace {

std::vector<double> make_matrix(std::size_t rows, std::size_t cols) {
    std::vector<double> data(rows * cols);
    for (std::size_t j = 0; j < cols; ++j)
        for (std::size_t i = 0; i < rows; ++i)
            data[colmajor_index(i, j, rows)] = static_cast<double>(i * cols + j + 1);
    return data;
}

const_matrix_view as_const_view(const std::vector<double>& data, std::size_t rows, std::size_t cols) {
    return *const_matrix_view::create(data, rows, cols);
}

matrix_view as_view(std::vector<double>& data, std::size_t rows, std::size_t cols) {
    return *matrix_view::create(data, rows, cols);
}

void require_ok(const Result<void>& r) {
    if (!r) FAIL_CHECK(r.error().msg);
    REQUIRE(r.has_value());
}

} // namespace

TEST_CASE("matrix_view create rejects size mismatch", "[linalg]") {
    std::vector<double> data(4);
    auto view = const_matrix_view::create(data, 2, 3);
    REQUIRE(!view.has_value());
    REQUIRE(view.error().code == ErrorCode::dimension_mismatch);
}

TEST_CASE("add — hand-computed 2x2", "[linalg][add]") {
    std::vector<double> a{1, 2, 3, 4};
    std::vector<double> b{5, 6, 7, 8};
    std::vector<double> c(4, 0.0);

    require_ok(add(as_const_view(a, 2, 2), as_const_view(b, 2, 2), as_view(c, 2, 2)));

    REQUIRE(c[0] == Approx(6.0));
    REQUIRE(c[1] == Approx(8.0));
    REQUIRE(c[2] == Approx(10.0));
    REQUIRE(c[3] == Approx(12.0));
}

TEST_CASE("add — dimension mismatch", "[linalg][add]") {
    std::vector<double> a(4), b(6), c(4);
    auto r = add(as_const_view(a, 2, 2), as_const_view(b, 2, 3), as_view(c, 2, 2));
    REQUIRE(!r.has_value());
}

TEST_CASE("add — Eigen oracle", "[linalg][add]") {
    constexpr std::size_t m = 5, n = 7;
    auto a = make_matrix(m, n);
    auto b = make_matrix(m, n);
    std::vector<double> c(m * n);

    require_ok(add(as_const_view(a, m, n), as_const_view(b, m, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e + B_e));
}

TEST_CASE("sub — matches Eigen", "[linalg][sub]") {
    constexpr std::size_t m = 4, n = 3;
    auto a = make_matrix(m, n);
    auto b = make_matrix(m, n);
    std::vector<double> c(m * n);

    require_ok(sub(as_const_view(a, m, n), as_const_view(b, m, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e - B_e));
}

TEST_CASE("scale — matches Eigen", "[linalg][scale]") {
    constexpr std::size_t m = 3, n = 4;
    auto a = make_matrix(m, n);
    std::vector<double> b(m * n);
    constexpr double alpha = 2.5;

    require_ok(scale(alpha, as_const_view(a, m, n), as_view(b, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> B_e(b.data(), m, n);
    REQUIRE(B_e.isApprox(alpha * A_e));
}

TEST_CASE("hadamard — matches Eigen", "[linalg][hadamard]") {
    constexpr std::size_t m = 3, n = 3;
    auto a = make_matrix(m, n);
    auto b = make_matrix(m, n);
    std::vector<double> c(m * n);

    require_ok(hadamard(as_const_view(a, m, n), as_const_view(b, m, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), m, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e.cwiseProduct(B_e)));
}

TEST_CASE("transpose — rectangular", "[linalg][transpose]") {
    std::vector<double> a{1, 2, 3, 4, 5, 6};
    std::vector<double> b(6);

    require_ok(transpose(as_const_view(a, 2, 3), as_view(b, 3, 2)));

    REQUIRE(b[0] == Approx(1.0));
    REQUIRE(b[1] == Approx(3.0));
    REQUIRE(b[2] == Approx(5.0));
    REQUIRE(b[3] == Approx(2.0));
    REQUIRE(b[4] == Approx(4.0));
    REQUIRE(b[5] == Approx(6.0));
}

TEST_CASE("dot — vector inner product", "[linalg][dot]") {
    std::vector<double> u{1, 2, 3};
    std::vector<double> v{4, 5, 6};
    auto r = dot(u, v);
    REQUIRE(r.has_value());
    REQUIRE(*r == Approx(32.0));
}

TEST_CASE("gemv — matches Eigen", "[linalg][gemv]") {
    constexpr std::size_t m = 4, n = 3;
    auto a = make_matrix(m, n);
    std::vector<double> x{1, -1, 2};
    std::vector<double> y(m);

    require_ok(gemv(as_const_view(a, m, n), x, y));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, n);
    Eigen::Map<const Eigen::VectorXd> x_e(x.data(), n);
    Eigen::Map<Eigen::VectorXd> y_e(y.data(), m);
    REQUIRE(y_e.isApprox(A_e * x_e));
}

TEST_CASE("gemm — matches Eigen", "[linalg][gemm]") {
    constexpr std::size_t m = 3, k = 4, n = 2;
    auto a = make_matrix(m, k);
    auto b = make_matrix(k, n);
    std::vector<double> c(m * n);

    require_ok(gemm(as_const_view(a, m, k), as_const_view(b, k, n), as_view(c, m, n)));

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), m, k);
    Eigen::Map<const Eigen::MatrixXd> B_e(b.data(), k, n);
    Eigen::Map<Eigen::MatrixXd> C_e(c.data(), m, n);
    REQUIRE(C_e.isApprox(A_e * B_e));
}

TEST_CASE("trace — square matrix", "[linalg][trace]") {
    std::vector<double> a{1, 0, 0, 4};
    auto r = trace(as_const_view(a, 2, 2));
    REQUIRE(r.has_value());
    REQUIRE(*r == Approx(5.0));
}

TEST_CASE("trace — rejects non-square", "[linalg][trace]") {
    std::vector<double> a(6);
    auto r = trace(as_const_view(a, 2, 3));
    REQUIRE(!r.has_value());
}

TEST_CASE("norm_fro — matches Eigen", "[linalg][norm_fro]") {
    auto a = make_matrix(3, 3);
    auto r = norm_fro(as_const_view(a, 3, 3));
    REQUIRE(r.has_value());
    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), 3, 3);
    REQUIRE(*r == Approx(A_e.norm()));
}

TEST_CASE("norm_inf — max absolute row sum", "[linalg][norm_inf]") {
    auto a = make_matrix(4, 3);
    auto r = norm_inf(as_const_view(a, 4, 3));
    REQUIRE(r.has_value());

    Eigen::Map<const Eigen::MatrixXd> A_e(a.data(), 4, 3);
    const double expected = A_e.cwiseAbs().rowwise().sum().maxCoeff();
    REQUIRE(*r == Approx(expected));
}
