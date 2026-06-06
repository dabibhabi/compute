#pragma once
#include <string_view>

namespace compute {

constexpr std::string_view version() noexcept { return "0.1.0"; }
constexpr int version_major() noexcept { return 0; }
constexpr int version_minor() noexcept { return 1; }
constexpr int version_patch() noexcept { return 0; }

} // namespace compute
