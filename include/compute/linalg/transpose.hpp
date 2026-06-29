#pragma once

#include <compute/linalg/layout.hpp>
#include <compute/core/error.hpp>

namespace compute::linalg {

[[nodiscard]] core::Result<void> transpose(const_matrix_view A, matrix_view B) noexcept;

} // namespace compute::linalg
