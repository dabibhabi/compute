#pragma once

#include <compute/linalg/layout.hpp>
#include <compute/core/error.hpp>

namespace compute::linalg {

[[nodiscard]] core::Result<double> norm_fro(const_matrix_view A) noexcept;

} // namespace compute::linalg
