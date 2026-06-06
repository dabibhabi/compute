#pragma once
#include <cstddef>

// Compile-time SIMD width detection for AVX-512 / AVX2 / scalar fallback.
// Used throughout to choose tile sizes and loop strides without runtime dispatch overhead.

namespace compute::core {

template<typename T>
struct simd_traits {
    // Width: number of T-elements that fit in one SIMD register.
#if defined(__AVX512F__)
    static constexpr std::size_t width = 64 / sizeof(T);
#elif defined(__AVX2__)
    static constexpr std::size_t width = 32 / sizeof(T);
#elif defined(__SSE2__)
    static constexpr std::size_t width = 16 / sizeof(T);
#else
    static constexpr std::size_t width = 1;
#endif

    // Alignment required for aligned loads (same as register width in bytes).
    static constexpr std::size_t alignment = width * sizeof(T);

    // Round n up to the next multiple of width — useful for padding loop bounds.
    static constexpr std::size_t round_up(std::size_t n) noexcept {
        return (n + width - 1) & ~(width - 1);
    }
};

// Convenience aliases
using f32_traits = simd_traits<float>;
using f64_traits = simd_traits<double>;

} // namespace compute::core
