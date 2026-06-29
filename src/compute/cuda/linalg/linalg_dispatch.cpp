#include "../detail/context.hpp"
#include "../linalg/kernels.cuh"
#include <compute/backends/gpu_backend.hpp>
#include <cmath>
#include <vector>

namespace compute::backends::cuda {

namespace {

using compute::cuda::detail::Context;
using compute::cuda::detail::check_cuda;

struct DeviceBuffer {
    void* ptr = nullptr;
    const Context* ctx = nullptr;

    DeviceBuffer() = default;
    DeviceBuffer(const Context& c, std::size_t bytes) : ctx(&c)
    {
        if (auto p = c.alloc_device(bytes); p) ptr = *p;
    }
    ~DeviceBuffer() { if (ptr && ctx) ctx->free_device(ptr); }
    DeviceBuffer(const DeviceBuffer&)            = delete;
    DeviceBuffer& operator=(const DeviceBuffer&) = delete;
    explicit operator bool() const noexcept { return ptr != nullptr; }
};

core::Result<void> ensure_ready() noexcept
{
    return Context::instance().init();
}

template<typename T>
core::Result<T> forward_error(const core::Result<void>& e) noexcept
{
    return core::make_error(e.error().code, e.error().msg);
}

core::Result<void> launch_elementwise(const_matrix_view A, const_matrix_view B, matrix_view C,
                                      CudaElementwiseOp op, double alpha = 1.0)
{
    if (auto e = ensure_ready(); !e) return e;
    auto& ctx = Context::instance();

    const std::size_t bytes = A.size() * sizeof(double);
    DeviceBuffer d_A(ctx, bytes);
    DeviceBuffer d_B(ctx, op == CudaElementwiseOp::scale ? 0 : bytes);
    DeviceBuffer d_C(ctx, bytes);
    if (!d_A || !d_C) return core::make_error(core::ErrorCode::cuda_error, "cudaMalloc");
    void* b_ptr = d_B ? d_B.ptr : d_A.ptr;
    if (op != CudaElementwiseOp::scale && !d_B)
        return core::make_error(core::ErrorCode::cuda_error, "cudaMalloc");

    if (auto e = ctx.copy_h2d(A.data().data(), d_A.ptr, bytes); !e) return e;
    if (op != CudaElementwiseOp::scale)
        if (auto e = ctx.copy_h2d(B.data().data(), b_ptr, bytes); !e) return e;

    cuda_launch_elementwise(static_cast<const double*>(d_A.ptr),
                            static_cast<const double*>(b_ptr),
                            static_cast<double*>(d_C.ptr), A.size(), static_cast<int>(op), alpha,
                            ctx.stream);
    if (auto e = check_cuda(cudaGetLastError(), "elementwise_kernel"); !e) return e;
    if (auto e = ctx.sync(); !e) return e;
    if (auto e = ctx.copy_d2h(d_C.ptr, C.data().data(), bytes); !e) return e;
    return ctx.sync();
}

} // namespace

bool available() noexcept
{
    return Context::instance().init().has_value();
}

core::Result<void> add(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    return launch_elementwise(A, B, C, CudaElementwiseOp::add);
}

core::Result<void> sub(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    return launch_elementwise(A, B, C, CudaElementwiseOp::sub);
}

core::Result<void> scale(double alpha, const_matrix_view A, matrix_view B) noexcept
{
    return launch_elementwise(A, A, B, CudaElementwiseOp::scale, alpha);
}

core::Result<void> hadamard(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    return launch_elementwise(A, B, C, CudaElementwiseOp::hadamard);
}

core::Result<void> transpose(const_matrix_view A, matrix_view B) noexcept
{
    if (auto e = ensure_ready(); !e) return e;
    auto& ctx = Context::instance();

    DeviceBuffer d_A(ctx, A.size() * sizeof(double));
    DeviceBuffer d_B(ctx, B.size() * sizeof(double));
    if (!d_A || !d_B) return core::make_error(core::ErrorCode::cuda_error, "cudaMalloc");

    if (auto e = ctx.copy_h2d(A.data().data(), d_A.ptr, A.size() * sizeof(double)); !e) return e;
    cuda_launch_transpose(static_cast<const double*>(d_A.ptr), static_cast<double*>(d_B.ptr),
                          A.rows(), A.cols(), ctx.stream);
    if (auto e = check_cuda(cudaGetLastError(), "transpose_kernel"); !e) return e;
    if (auto e = ctx.sync(); !e) return e;
    if (auto e = ctx.copy_d2h(d_B.ptr, B.data().data(), B.size() * sizeof(double)); !e) return e;
    return ctx.sync();
}

core::Result<double> dot(std::span<const double> u, std::span<const double> v) noexcept
{
    if (auto e = ensure_ready(); !e) return forward_error<double>(e);
    auto& ctx = Context::instance();

    const std::size_t bytes = u.size() * sizeof(double);
    DeviceBuffer d_u(ctx, bytes);
    DeviceBuffer d_v(ctx, bytes);
    if (!d_u || !d_v) return core::make_error(core::ErrorCode::cuda_error, "cudaMalloc");

    if (auto e = ctx.copy_h2d(u.data(), d_u.ptr, bytes); !e) return forward_error<double>(e);
    if (auto e = ctx.copy_h2d(v.data(), d_v.ptr, bytes); !e) return forward_error<double>(e);

    double result = 0.0;
    if (cublasDdot(ctx.cublas, static_cast<int>(u.size()),
                   static_cast<const double*>(d_u.ptr), 1,
                   static_cast<const double*>(d_v.ptr), 1, &result) != CUBLAS_STATUS_SUCCESS)
        return core::make_error(core::ErrorCode::cuda_error, "cublasDdot");
    if (auto e = ctx.sync(); !e) return forward_error<double>(e);
    return result;
}

core::Result<void> gemv(const_matrix_view A, std::span<const double> x, std::span<double> y) noexcept
{
    if (auto e = ensure_ready(); !e) return e;
    auto& ctx = Context::instance();

    DeviceBuffer d_A(ctx, A.size() * sizeof(double));
    DeviceBuffer d_x(ctx, x.size() * sizeof(double));
    DeviceBuffer d_y(ctx, y.size() * sizeof(double));
    if (!d_A || !d_x || !d_y) return core::make_error(core::ErrorCode::cuda_error, "cudaMalloc");

    const int m = static_cast<int>(A.rows());
    const int n = static_cast<int>(A.cols());
    const double alpha = 1.0, beta = 0.0;

    if (auto e = ctx.copy_h2d(A.data().data(), d_A.ptr, A.size() * sizeof(double)); !e) return e;
    if (auto e = ctx.copy_h2d(x.data(), d_x.ptr, x.size() * sizeof(double)); !e) return e;

    if (cublasDgemv(ctx.cublas, CUBLAS_OP_N, m, n, &alpha,
                    static_cast<const double*>(d_A.ptr), m,
                    static_cast<const double*>(d_x.ptr), 1,
                    &beta, static_cast<double*>(d_y.ptr), 1) != CUBLAS_STATUS_SUCCESS)
        return core::make_error(core::ErrorCode::cuda_error, "cublasDgemv");
    if (auto e = ctx.copy_d2h(d_y.ptr, y.data(), y.size() * sizeof(double)); !e) return e;
    return ctx.sync();
}

core::Result<void> gemm(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept
{
    if (auto e = ensure_ready(); !e) return e;
    auto& ctx = Context::instance();

    DeviceBuffer d_A(ctx, A.size() * sizeof(double));
    DeviceBuffer d_B(ctx, B.size() * sizeof(double));
    DeviceBuffer d_C(ctx, C.size() * sizeof(double));
    if (!d_A || !d_B || !d_C) return core::make_error(core::ErrorCode::cuda_error, "cudaMalloc");

    const int m = static_cast<int>(A.rows());
    const int k = static_cast<int>(A.cols());
    const int n = static_cast<int>(B.cols());
    const double alpha = 1.0, beta = 0.0;

    if (auto e = ctx.copy_h2d(A.data().data(), d_A.ptr, A.size() * sizeof(double)); !e) return e;
    if (auto e = ctx.copy_h2d(B.data().data(), d_B.ptr, B.size() * sizeof(double)); !e) return e;

    if (cublasDgemm(ctx.cublas, CUBLAS_OP_N, CUBLAS_OP_N, m, n, k, &alpha,
                    static_cast<const double*>(d_A.ptr), m,
                    static_cast<const double*>(d_B.ptr), k,
                    &beta, static_cast<double*>(d_C.ptr), m) != CUBLAS_STATUS_SUCCESS)
        return core::make_error(core::ErrorCode::cuda_error, "cublasDgemm");
    if (auto e = ctx.copy_d2h(d_C.ptr, C.data().data(), C.size() * sizeof(double)); !e) return e;
    return ctx.sync();
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
    auto& ctx = Context::instance();

    const std::size_t rows = A.rows();
    DeviceBuffer d_A(ctx, A.size() * sizeof(double));
    DeviceBuffer d_out(ctx, rows * sizeof(double));
    if (!d_A || !d_out) return core::make_error(core::ErrorCode::cuda_error, "cudaMalloc");

    if (auto e = ctx.copy_h2d(A.data().data(), d_A.ptr, A.size() * sizeof(double)); !e)
        return forward_error<double>(e);
    cuda_launch_norm_inf(static_cast<const double*>(d_A.ptr), static_cast<double*>(d_out.ptr),
                         rows, A.cols(), ctx.stream);
    if (auto e = check_cuda(cudaGetLastError(), "norm_inf_kernel"); !e)
        return forward_error<double>(e);
    if (auto e = ctx.sync(); !e) return forward_error<double>(e);

    std::vector<double> row_sums(rows);
    if (auto e = ctx.copy_d2h(d_out.ptr, row_sums.data(), rows * sizeof(double)); !e)
        return forward_error<double>(e);
    if (auto e = ctx.sync(); !e) return forward_error<double>(e);

    double max_sum = 0.0;
    for (double s : row_sums) max_sum = std::fmax(max_sum, s);
    return max_sum;
}

} // namespace compute::backends::cuda
