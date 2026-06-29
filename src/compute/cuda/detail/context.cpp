#include "context.hpp"
#include <compute/core/error.hpp>

namespace compute::cuda::detail {

core::Result<void> check_cuda(cudaError_t err, const char* what) noexcept
{
    if (err == cudaSuccess) return {};
    return core::make_error(core::ErrorCode::cuda_error, what);
}

core::Result<void> check_cublas(cublasStatus_t status, const char* what) noexcept
{
    if (status == CUBLAS_STATUS_SUCCESS) return {};
    return core::make_error(core::ErrorCode::cuda_error, what);
}

Context& Context::instance()
{
    static Context ctx;
    return ctx;
}

core::Result<void> Context::init() noexcept
{
    if (ready) return {};

    int count = 0;
    if (auto e = check_cuda(cudaGetDeviceCount(&count), "cudaGetDeviceCount"); !e) return e;
    if (count == 0)
        return core::make_error(core::ErrorCode::cuda_error, "no cuda devices");

    if (auto e = check_cuda(cudaSetDevice(0), "cudaSetDevice"); !e) return e;
    if (auto e = check_cuda(cudaStreamCreate(&stream), "cudaStreamCreate"); !e) return e;

    if (cublasCreate(&cublas) != CUBLAS_STATUS_SUCCESS)
        return core::make_error(core::ErrorCode::cuda_error, "cublasCreate");
    cublasSetStream(cublas, stream);

    ready = true;
    return {};
}

void Context::shutdown() noexcept
{
    if (!ready) return;
    cublasDestroy(cublas);
    cudaStreamDestroy(stream);
    cublas = nullptr;
    stream = nullptr;
    ready  = false;
}

core::Result<void*> Context::alloc_device(std::size_t bytes) const noexcept
{
    void* ptr = nullptr;
    if (auto e = check_cuda(cudaMalloc(&ptr, bytes), "cudaMalloc"); !e)
        return core::make_error(e.error().code, e.error().msg);
    return ptr;
}

core::Result<void> Context::copy_h2d(const void* host, void* device, std::size_t bytes) const noexcept
{
    return check_cuda(cudaMemcpyAsync(device, host, bytes, cudaMemcpyHostToDevice, stream),
                     "cudaMemcpyAsync H2D");
}

core::Result<void> Context::copy_d2h(const void* device, void* host, std::size_t bytes) const noexcept
{
    return check_cuda(cudaMemcpyAsync(host, device, bytes, cudaMemcpyDeviceToHost, stream),
                     "cudaMemcpyAsync D2H");
}

core::Result<void> Context::sync() const noexcept
{
    return check_cuda(cudaStreamSynchronize(stream), "cudaStreamSynchronize");
}

void Context::free_device(void* ptr) const noexcept
{
    if (ptr) cudaFree(ptr);
}

} // namespace compute::cuda::detail
