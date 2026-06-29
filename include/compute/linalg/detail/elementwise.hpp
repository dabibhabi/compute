#pragma once

#include <compute/linalg/layout.hpp>
#include <utility>

namespace compute::linalg::detail {

template<typename Fn>
[[nodiscard]] core::Result<void>
elementwise_binary(const const_matrix_view& A, const const_matrix_view& B, matrix_view& C, Fn op) noexcept {
    if (auto check = check_same_shape(A, B, C); !check) return check;

    for (std::size_t j = 0; j < A.cols(); ++j)
        for (std::size_t i = 0; i < A.rows(); ++i)
            C(i, j) = op(A(i, j), B(i, j));

    return {};
}

template<typename Fn>
[[nodiscard]] core::Result<void>
elementwise_unary(const const_matrix_view& A, matrix_view& B, Fn op) noexcept {
    if (!B.same_shape(A))
        return core::make_error(core::ErrorCode::dimension_mismatch,
                                "output dimensions must match input");

    for (std::size_t j = 0; j < A.cols(); ++j)
        for (std::size_t i = 0; i < A.rows(); ++i)
            B(i, j) = op(A(i, j));

    return {};
}

} // namespace compute::linalg::detail
