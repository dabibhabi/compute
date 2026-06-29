#pragma once

#include <compute/linalg/layout.hpp>
#include <compute/core/error.hpp>

namespace compute::linalg {

[[nodiscard]] core::Result<void> gemv(const_matrix_view A, std::span<const double> x, std::span<double> y) noexcept;

} // namespace compute::linalg
