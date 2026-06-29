#pragma once

#include <compute/linalg/layout.hpp>
#include <compute/core/error.hpp>

namespace compute::linalg {

[[nodiscard]] core::Result<void> hadamard(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;

} // namespace compute::linalg
