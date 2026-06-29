#pragma once

#include <compute/core/error.hpp>
#include <compute/linalg/layout.hpp>
#include <cstddef>
#include <span>

namespace compute::backends {

using linalg::const_matrix_view;
using linalg::matrix_view;

namespace cuda {

[[nodiscard]] bool available() noexcept;
[[nodiscard]] core::Result<void> add(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<void> sub(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<void> scale(double alpha, const_matrix_view A, matrix_view B) noexcept;
[[nodiscard]] core::Result<void> hadamard(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<void> transpose(const_matrix_view A, matrix_view B) noexcept;
[[nodiscard]] core::Result<double> dot(std::span<const double> u, std::span<const double> v) noexcept;
[[nodiscard]] core::Result<void> gemv(const_matrix_view A, std::span<const double> x, std::span<double> y) noexcept;
[[nodiscard]] core::Result<void> gemm(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<double> trace(const_matrix_view A) noexcept;
[[nodiscard]] core::Result<double> norm_fro(const_matrix_view A) noexcept;
[[nodiscard]] core::Result<double> norm_inf(const_matrix_view A) noexcept;

} // namespace cuda

namespace metal {

[[nodiscard]] bool available() noexcept;
[[nodiscard]] core::Result<void> add(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<void> sub(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<void> scale(double alpha, const_matrix_view A, matrix_view B) noexcept;
[[nodiscard]] core::Result<void> hadamard(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<void> transpose(const_matrix_view A, matrix_view B) noexcept;
[[nodiscard]] core::Result<double> dot(std::span<const double> u, std::span<const double> v) noexcept;
[[nodiscard]] core::Result<void> gemv(const_matrix_view A, std::span<const double> x, std::span<double> y) noexcept;
[[nodiscard]] core::Result<void> gemm(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<double> trace(const_matrix_view A) noexcept;
[[nodiscard]] core::Result<double> norm_fro(const_matrix_view A) noexcept;
[[nodiscard]] core::Result<double> norm_inf(const_matrix_view A) noexcept;

} // namespace metal

namespace vulkan {

[[nodiscard]] bool available() noexcept;
[[nodiscard]] core::Result<void> add(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<void> sub(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<void> scale(double alpha, const_matrix_view A, matrix_view B) noexcept;
[[nodiscard]] core::Result<void> hadamard(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<void> transpose(const_matrix_view A, matrix_view B) noexcept;
[[nodiscard]] core::Result<double> dot(std::span<const double> u, std::span<const double> v) noexcept;
[[nodiscard]] core::Result<void> gemv(const_matrix_view A, std::span<const double> x, std::span<double> y) noexcept;
[[nodiscard]] core::Result<void> gemm(const_matrix_view A, const_matrix_view B, matrix_view C) noexcept;
[[nodiscard]] core::Result<double> trace(const_matrix_view A) noexcept;
[[nodiscard]] core::Result<double> norm_fro(const_matrix_view A) noexcept;
[[nodiscard]] core::Result<double> norm_inf(const_matrix_view A) noexcept;

} // namespace vulkan

} // namespace compute::backends
