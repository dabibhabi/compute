#pragma once

#include <compute/core/error.hpp>
#include <cstddef>
#include <span>

namespace compute::linalg {

[[nodiscard]] core::Result<double> dot(std::span<const double> u, std::span<const double> v) noexcept;

} // namespace compute::linalg
