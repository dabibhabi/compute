#pragma once

#include <compute/core/error.hpp>
#include <cstddef>
#include <span>

namespace compute::linalg {

inline constexpr std::size_t colmajor_index(std::size_t i, std::size_t j, std::size_t rows) noexcept {
    return j * rows + i;
}

class const_matrix_view {
public:
    [[nodiscard]] static core::Result<const_matrix_view>
    create(std::span<const double> data, std::size_t rows, std::size_t cols) noexcept;

    [[nodiscard]] std::size_t rows() const noexcept { return rows_; }
    [[nodiscard]] std::size_t cols() const noexcept { return cols_; }
    [[nodiscard]] std::size_t size()  const noexcept { return rows_ * cols_; }
    [[nodiscard]] std::span<const double> data() const noexcept { return data_; }

    [[nodiscard]] double operator()(std::size_t i, std::size_t j) const noexcept {
        return data_[colmajor_index(i, j, rows_)];
    }

    [[nodiscard]] bool same_shape(const const_matrix_view& other) const noexcept {
        return rows_ == other.rows_ && cols_ == other.cols_;
    }

private:
    friend class matrix_view;

    const_matrix_view(std::span<const double> data, std::size_t rows, std::size_t cols) noexcept
        : data_(data), rows_(rows), cols_(cols) {}

    std::span<const double> data_;
    std::size_t rows_;
    std::size_t cols_;
};

class matrix_view {
public:
    [[nodiscard]] static core::Result<matrix_view>
    create(std::span<double> data, std::size_t rows, std::size_t cols) noexcept;

    [[nodiscard]] std::size_t rows() const noexcept { return rows_; }
    [[nodiscard]] std::size_t cols() const noexcept { return cols_; }
    [[nodiscard]] std::size_t size()  const noexcept { return rows_ * cols_; }
    [[nodiscard]] std::span<double> data() noexcept { return data_; }
    [[nodiscard]] std::span<const double> data() const noexcept { return data_; }

    [[nodiscard]] double& operator()(std::size_t i, std::size_t j) noexcept {
        return data_[colmajor_index(i, j, rows_)];
    }

    [[nodiscard]] double operator()(std::size_t i, std::size_t j) const noexcept {
        return data_[colmajor_index(i, j, rows_)];
    }

    [[nodiscard]] bool same_shape(const matrix_view& other) const noexcept {
        return rows_ == other.rows_ && cols_ == other.cols_;
    }

    [[nodiscard]] bool same_shape(const const_matrix_view& other) const noexcept {
        return rows_ == other.rows() && cols_ == other.cols();
    }

private:
    matrix_view(std::span<double> data, std::size_t rows, std::size_t cols) noexcept
        : data_(data), rows_(rows), cols_(cols) {}

    std::span<double> data_;
    std::size_t rows_;
    std::size_t cols_;
};

[[nodiscard]] inline core::Result<void>
check_same_shape(const const_matrix_view& A, const const_matrix_view& B, const matrix_view& C) noexcept {
    if (!A.same_shape(B))
        return core::make_error(core::ErrorCode::dimension_mismatch, "operand dimensions must match");
    if (!C.same_shape(A))
        return core::make_error(core::ErrorCode::dimension_mismatch, "output dimensions must match operands");
    return {};
}

} // namespace compute::linalg
