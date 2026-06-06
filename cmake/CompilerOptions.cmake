# Compiler flags applied globally to all non-CUDA targets.
# Applied via target properties on compute_core (INTERFACE) so every module inherits them.

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

  add_compile_options(
    # Architecture — native tuning for the build host (Zen 5 + AVX-512 on the dev machine)
    -march=native
    -mtune=native

    # Warnings
    -Wall -Wextra -Wpedantic
    -Wno-unused-parameter

    # Debug build extras
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>

    # Release extras
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:Release>:-ffast-math>
  )

  # ASAN for debug builds
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
  endif()

  # LTO for release
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  endif()

endif()
