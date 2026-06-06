#include <catch2/catch_test_macros.hpp>
#include <compute/core.hpp>

using namespace compute::core;

TEST_CASE("version string is set", "[core]") {
    REQUIRE(compute::version() == "0.1.0");
}

TEST_CASE("aligned_buffer allocates with correct alignment", "[core]") {
    aligned_buffer<double> buf(128);
    REQUIRE(buf.size() == 128);
    REQUIRE(buf.data() != nullptr);
    // 64-byte alignment required for AVX-512 aligned loads
    auto addr = reinterpret_cast<std::uintptr_t>(buf.data());
    REQUIRE(addr % 64 == 0);
}

TEST_CASE("aligned_buffer move semantics", "[core]") {
    aligned_buffer<float> a(64);
    float* raw = a.data();

    aligned_buffer<float> b = std::move(a);
    REQUIRE(b.data() == raw);
    REQUIRE(a.data() == nullptr);
    REQUIRE(a.size() == 0);
}

TEST_CASE("aligned_buffer as_span", "[core]") {
    aligned_buffer<int> buf(10);
    auto sp = buf.as_span();
    REQUIRE(sp.size() == 10);
    REQUIRE(sp.data() == buf.data());
}

TEST_CASE("simd_traits width is positive and a power of two", "[core]") {
    auto w = f64_traits::width;
    REQUIRE(w >= 1);
    REQUIRE((w & (w - 1)) == 0);  // power-of-two check
}

TEST_CASE("simd_traits round_up", "[core]") {
    REQUIRE(f64_traits::round_up(0) == 0);
    REQUIRE(f64_traits::round_up(1) == f64_traits::width);
    REQUIRE(f64_traits::round_up(f64_traits::width) == f64_traits::width);
    REQUIRE(f64_traits::round_up(f64_traits::width + 1) == 2 * f64_traits::width);
}

TEST_CASE("ComputePolicy thread-local default is automatic", "[core]") {
    REQUIRE(get_policy() == ComputePolicy::automatic);
    set_policy(ComputePolicy::cpu_only);
    REQUIRE(get_policy() == ComputePolicy::cpu_only);
    set_policy(ComputePolicy::automatic);  // restore
}

TEST_CASE("Result / std::expected happy path", "[core]") {
    auto ok = Result<int>{42};
    REQUIRE(ok.has_value());
    REQUIRE(*ok == 42);
}

TEST_CASE("Result / std::expected error path", "[core]") {
    Result<int> err = make_error(ErrorCode::dimension_mismatch, "rows don't match");
    REQUIRE(!err.has_value());
    REQUIRE(err.error().code == ErrorCode::dimension_mismatch);
}
