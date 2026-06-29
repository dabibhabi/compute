#include <compute/linalg/layout.hpp>

namespace compute::linalg {

core::Result<const_matrix_view>
const_matrix_view::create(std::span<const double> data, std::size_t rows, std::size_t cols) noexcept {
    if (data.size() != rows * cols)
        return core::make_error(core::ErrorCode::dimension_mismatch,
                                "buffer size does not match matrix dimensions");
    return const_matrix_view{data, rows, cols};
}

core::Result<matrix_view>
matrix_view::create(std::span<double> data, std::size_t rows, std::size_t cols) noexcept {
    if (data.size() != rows * cols)
        return core::make_error(core::ErrorCode::dimension_mismatch,
                                "buffer size does not match matrix dimensions");
    return matrix_view{data, rows, cols};
}

} // namespace compute::linalg
