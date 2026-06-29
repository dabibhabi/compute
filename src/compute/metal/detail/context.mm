#include "context.h"
#include <metal_kernels_source.h>
#import <Foundation/Foundation.h>

namespace compute::metal::detail {

MetalContext& MetalContext::instance()
{
    static MetalContext ctx;
    return ctx;
}

core::Result<void> MetalContext::init() noexcept
{
    if (ready) return {};

    device = MTLCreateSystemDefaultDevice();
    if (!device) return core::make_error(core::ErrorCode::cuda_error, "no metal device");

    queue = [device newCommandQueue];
    if (!queue) return core::make_error(core::ErrorCode::cuda_error, "metal newCommandQueue");

    NSError* err = nil;
    NSString* source = [NSString stringWithUTF8String:kShaderSource];
    library = [device newLibraryWithSource:source options:nil error:&err];
    if (!library) return core::make_error(core::ErrorCode::cuda_error, "metal shader compile failed");

    pipelines = [NSMutableDictionary dictionary];
    ready = true;
    return {};
}

id<MTLComputePipelineState> MetalContext::pipeline(const char* name) noexcept
{
    NSString* key = [NSString stringWithUTF8String:name];
    if (id<MTLComputePipelineState> cached = pipelines[key]) return cached;

    id<MTLFunction> fn = [library newFunctionWithName:key];
    if (!fn) return nil;

    NSError* err = nil;
    id<MTLComputePipelineState> pso = [device newComputePipelineStateWithFunction:fn error:&err];
    if (pso) pipelines[key] = pso;
    return pso;
}

id<MTLBuffer> MetalContext::make_buffer(std::size_t float_count) const noexcept
{
    const std::size_t bytes = (float_count == 0 ? 1 : float_count) * sizeof(float);
    return [device newBufferWithLength:bytes options:MTLResourceStorageModeShared];
}

void upload(std::span<const double> src, id<MTLBuffer> dst) noexcept
{
    auto* p = static_cast<float*>(dst.contents);
    for (std::size_t i = 0; i < src.size(); ++i) p[i] = static_cast<float>(src[i]);
}

void download(id<MTLBuffer> src, std::span<double> dst) noexcept
{
    const auto* p = static_cast<const float*>(src.contents);
    for (std::size_t i = 0; i < dst.size(); ++i) dst[i] = static_cast<double>(p[i]);
}

} // namespace compute::metal::detail
