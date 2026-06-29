#include "../detail/context.h"
#include <compute/backends/gpu_backend.hpp>
#import <Foundation/Foundation.h>
#include <cmath>
#include <cstdint>
#include <vector>

namespace compute::backends::metal {

namespace {

using compute::metal::detail::MetalContext;
using compute::metal::detail::upload;
using compute::metal::detail::download;

struct ElemParams { std::uint32_t n; std::int32_t op; float alpha; };
struct Dims2      { std::uint32_t rows; std::uint32_t cols; };
struct GemmDims   { std::uint32_t m; std::uint32_t n; std::uint32_t k; };

constexpr std::size_t tg_1d = 256;
constexpr std::size_t tg_2d = 16;

core::Result<void> ensure_ready() noexcept { return MetalContext::instance().init(); }

template<typename T>
core::Result<T> forward_error(const core::Result<void>& e) noexcept
{
    return core::make_error(e.error().code, e.error().msg);
}

MTLSize groups_1d(std::size_t n) { return MTLSizeMake((n + tg_1d - 1) / tg_1d, 1, 1); }
MTLSize groups_2d(std::size_t x, std::size_t y)
{
    return MTLSizeMake((x + tg_2d - 1) / tg_2d, (y + tg_2d - 1) / tg_2d, 1);
}

core::Result<void> run(id<MTLComputePipelineState> pso,
                       void (^configure)(id<MTLComputeCommandEncoder>),
                       MTLSize grid, MTLSize threadgroup) noexcept
{
    if (!pso) return core::make_error(core::ErrorCode::cuda_error, "metal pipeline missing");
    auto& ctx = MetalContext::instance();
    id<MTLCommandBuffer> cb = [ctx.queue commandBuffer];
    id<MTLComputeCommandEncoder> enc = [cb computeCommandEncoder];
    [enc setComputePipelineState:pso];
    configure(enc);
    [enc dispatchThreadgroups:grid threadsPerThreadgroup:threadgroup];
    [enc endEncoding];
    [cb commit];
    [cb waitUntilCompleted];
    if (cb.error) return core::make_error(core::ErrorCode::cuda_error, "metal command failed");
    return {};
}

core::Result<void> run_elementwise(const_matrix_view A, const_matrix_view B, matrix_view C,
                                   std::int32_t op, float alpha)
{
    if (auto e = ensure_ready(); !e) return e;
    auto& ctx = MetalContext::instance();

    const std::size_t n = A.size();
    id<MTLBuffer> bA = ctx.make_buffer(n);
    id<MTLBuffer> bB = ctx.make_buffer(n);
    id<MTLBuffer> bC = ctx.make_buffer(n);
    if (!bA || !bB || !bC) return core::make_error(core::ErrorCode::cuda_error, "metal alloc");

    upload(A.data(), bA);
    if (op != 2) upload(B.data(), bB);

    ElemParams p{static_cast<std::uint32_t>(n), op, alpha};
    id<MTLBuffer> bSecond = (op == 2) ? bA : bB;

    auto r = run(ctx.pipeline("elementwise"),
                 ^(id<MTLComputeCommandEncoder> enc) {
                     [enc setBuffer:bA offset:0 atIndex:0];
                     [enc setBuffer:bSecond offset:0 atIndex:1];
                     [enc setBuffer:bC offset:0 atIndex:2];
                     [enc setBytes:&p length:sizeof(p) atIndex:3];
                 },
                 groups_1d(n), MTLSizeMake(tg_1d, 1, 1));
    if (!r) return r;
    download(bC, C.data());
    return {};
}

} // namespace

bool available() noexcept
{
    return MetalContext::instance().init().has_value();
}

core::Result<void> add(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    return run_elementwise(A, B, C, 0, 1.0f);
}

core::Result<void> sub(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    return run_elementwise(A, B, C, 1, 1.0f);
}

core::Result<void> scale(double alpha, const_matrix_view A, matrix_view B) noexcept
{
    return run_elementwise(A, A, B, 2, static_cast<float>(alpha));
}

core::Result<void> hadamard(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    return run_elementwise(A, B, C, 3, 1.0f);
}

core::Result<void> transpose(const_matrix_view A, matrix_view B) noexcept
{
    if (auto e = ensure_ready(); !e) return e;
    auto& ctx = MetalContext::instance();

    id<MTLBuffer> bA = ctx.make_buffer(A.size());
    id<MTLBuffer> bB = ctx.make_buffer(B.size());
    if (!bA || !bB) return core::make_error(core::ErrorCode::cuda_error, "metal alloc");

    upload(A.data(), bA);
    Dims2 p{static_cast<std::uint32_t>(A.rows()), static_cast<std::uint32_t>(A.cols())};

    auto r = run(ctx.pipeline("transpose"),
                 ^(id<MTLComputeCommandEncoder> enc) {
                     [enc setBuffer:bA offset:0 atIndex:0];
                     [enc setBuffer:bB offset:0 atIndex:1];
                     [enc setBytes:&p length:sizeof(p) atIndex:2];
                 },
                 groups_2d(A.rows(), A.cols()), MTLSizeMake(tg_2d, tg_2d, 1));
    if (!r) return r;
    download(bB, B.data());
    return {};
}

core::Result<void> gemm(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    if (auto e = ensure_ready(); !e) return e;
    auto& ctx = MetalContext::instance();

    id<MTLBuffer> bA = ctx.make_buffer(A.size());
    id<MTLBuffer> bB = ctx.make_buffer(B.size());
    id<MTLBuffer> bC = ctx.make_buffer(C.size());
    if (!bA || !bB || !bC) return core::make_error(core::ErrorCode::cuda_error, "metal alloc");

    upload(A.data(), bA);
    upload(B.data(), bB);
    GemmDims p{static_cast<std::uint32_t>(A.rows()),
               static_cast<std::uint32_t>(B.cols()),
               static_cast<std::uint32_t>(A.cols())};

    auto r = run(ctx.pipeline("gemm"),
                 ^(id<MTLComputeCommandEncoder> enc) {
                     [enc setBuffer:bA offset:0 atIndex:0];
                     [enc setBuffer:bB offset:0 atIndex:1];
                     [enc setBuffer:bC offset:0 atIndex:2];
                     [enc setBytes:&p length:sizeof(p) atIndex:3];
                 },
                 groups_2d(A.rows(), B.cols()), MTLSizeMake(tg_2d, tg_2d, 1));
    if (!r) return r;
    download(bC, C.data());
    return {};
}

core::Result<void> gemv(const_matrix_view A, std::span<const double> x, std::span<double> y) noexcept
{
    if (auto e = ensure_ready(); !e) return e;
    auto& ctx = MetalContext::instance();

    id<MTLBuffer> bA = ctx.make_buffer(A.size());
    id<MTLBuffer> bx = ctx.make_buffer(x.size());
    id<MTLBuffer> by = ctx.make_buffer(y.size());
    if (!bA || !bx || !by) return core::make_error(core::ErrorCode::cuda_error, "metal alloc");

    upload(A.data(), bA);
    upload(x, bx);
    Dims2 p{static_cast<std::uint32_t>(A.rows()), static_cast<std::uint32_t>(A.cols())};

    auto r = run(ctx.pipeline("gemv"),
                 ^(id<MTLComputeCommandEncoder> enc) {
                     [enc setBuffer:bA offset:0 atIndex:0];
                     [enc setBuffer:bx offset:0 atIndex:1];
                     [enc setBuffer:by offset:0 atIndex:2];
                     [enc setBytes:&p length:sizeof(p) atIndex:3];
                 },
                 groups_1d(A.rows()), MTLSizeMake(tg_1d, 1, 1));
    if (!r) return r;
    download(by, y);
    return {};
}

core::Result<double> dot(std::span<const double> u, std::span<const double> v) noexcept
{
    if (auto e = ensure_ready(); !e) return forward_error<double>(e);
    auto& ctx = MetalContext::instance();

    const std::size_t n = u.size();
    const std::size_t groups = (n + tg_1d - 1) / tg_1d;
    id<MTLBuffer> bu = ctx.make_buffer(n);
    id<MTLBuffer> bv = ctx.make_buffer(n);
    id<MTLBuffer> bp = ctx.make_buffer(groups);
    if (!bu || !bv || !bp) return core::make_error(core::ErrorCode::cuda_error, "metal alloc");

    upload(u, bu);
    upload(v, bv);
    std::uint32_t nn = static_cast<std::uint32_t>(n);

    auto r = run(ctx.pipeline("reduce_dot"),
                 ^(id<MTLComputeCommandEncoder> enc) {
                     [enc setBuffer:bu offset:0 atIndex:0];
                     [enc setBuffer:bv offset:0 atIndex:1];
                     [enc setBuffer:bp offset:0 atIndex:2];
                     [enc setBytes:&nn length:sizeof(nn) atIndex:3];
                 },
                 MTLSizeMake(groups, 1, 1), MTLSizeMake(tg_1d, 1, 1));
    if (!r) return forward_error<double>(r);

    const auto* parts = static_cast<const float*>(bp.contents);
    double sum = 0.0;
    for (std::size_t i = 0; i < groups; ++i) sum += static_cast<double>(parts[i]);
    return sum;
}

core::Result<double> trace(const_matrix_view A) noexcept
{
    if (A.rows() != A.cols())
        return core::make_error(core::ErrorCode::dimension_mismatch, "trace requires square matrix");

    std::vector<double> diag(A.rows());
    for (std::size_t i = 0; i < A.rows(); ++i) diag[i] = A(i, i);
    std::vector<double> ones(A.rows(), 1.0);
    return dot(diag, ones);
}

core::Result<double> norm_fro(const_matrix_view A) noexcept
{
    std::vector<double> v(A.data().begin(), A.data().end());
    auto r = dot(v, v);
    if (!r) return r;
    return std::sqrt(*r);
}

core::Result<double> norm_inf(const_matrix_view A) noexcept
{
    if (auto e = ensure_ready(); !e) return forward_error<double>(e);
    auto& ctx = MetalContext::instance();

    const std::size_t rows = A.rows();
    id<MTLBuffer> bA = ctx.make_buffer(A.size());
    id<MTLBuffer> bout = ctx.make_buffer(rows);
    if (!bA || !bout) return core::make_error(core::ErrorCode::cuda_error, "metal alloc");

    upload(A.data(), bA);
    Dims2 p{static_cast<std::uint32_t>(rows), static_cast<std::uint32_t>(A.cols())};

    auto r = run(ctx.pipeline("row_abs_sum"),
                 ^(id<MTLComputeCommandEncoder> enc) {
                     [enc setBuffer:bA offset:0 atIndex:0];
                     [enc setBuffer:bout offset:0 atIndex:1];
                     [enc setBytes:&p length:sizeof(p) atIndex:2];
                 },
                 groups_1d(rows), MTLSizeMake(tg_1d, 1, 1));
    if (!r) return forward_error<double>(r);

    const auto* out = static_cast<const float*>(bout.contents);
    double max_sum = 0.0;
    for (std::size_t i = 0; i < rows; ++i) max_sum = std::fmax(max_sum, static_cast<double>(out[i]));
    return max_sum;
}

} // namespace compute::backends::metal
