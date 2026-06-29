#pragma once

#include <compute/core/error.hpp>
#import <Metal/Metal.h>
#include <cstddef>
#include <span>

namespace compute::metal::detail {

struct MetalContext {
    static MetalContext& instance();

    id<MTLDevice>       device  = nil;
    id<MTLCommandQueue> queue   = nil;
    id<MTLLibrary>      library = nil;
    NSMutableDictionary<NSString*, id<MTLComputePipelineState>>* pipelines = nil;
    bool ready = false;

    [[nodiscard]] core::Result<void> init() noexcept;
    [[nodiscard]] id<MTLComputePipelineState> pipeline(const char* name) noexcept;
    [[nodiscard]] id<MTLBuffer> make_buffer(std::size_t float_count) const noexcept;
};

void upload(std::span<const double> src, id<MTLBuffer> dst) noexcept;
void download(id<MTLBuffer> src, std::span<double> dst) noexcept;

} // namespace compute::metal::detail
