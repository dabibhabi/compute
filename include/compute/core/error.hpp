#pragma once
#include <expected>
#include <string_view>
#include <cstdint>

namespace compute::core {

enum class ErrorCode : uint8_t {
    ok = 0,
    dimension_mismatch,
    singular_matrix,
    not_converged,
    invalid_argument,
    out_of_memory,
    cuda_error,
};

struct ComputeError {
    ErrorCode   code;
    std::string_view msg;   // points to a string literal — zero allocation

    constexpr ComputeError(ErrorCode c, std::string_view m) noexcept
        : code(c), msg(m) {}
};

template<typename T>
using Result = std::expected<T, ComputeError>;

// Convenience factory helpers
inline constexpr auto make_error(ErrorCode c, std::string_view msg) noexcept {
    return std::unexpected(ComputeError{c, msg});
}

} // namespace compute::core
