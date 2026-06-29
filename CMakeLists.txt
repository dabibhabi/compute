cmake_minimum_required(VERSION 3.25)
project(compute VERSION 0.1.0 LANGUAGES CXX)

# ── Options ──────────────────────────────────────────────────────────────────
option(COMPUTE_ENABLE_CUDA    "Build CUDA kernels"         OFF)
option(COMPUTE_ENABLE_METAL    "Build Metal kernels (macOS)" OFF)
option(COMPUTE_ENABLE_VULKAN  "Build Vulkan compute kernels" OFF)
option(COMPUTE_ENABLE_PYTHON  "Build Python bindings"      ON)
option(COMPUTE_ENABLE_TESTS   "Build unit tests"           ON)
option(COMPUTE_ENABLE_BENCH   "Build benchmarks"           ON)

# ── C++ standard ─────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── CUDA (optional, enabled in Week 4+) ──────────────────────────────────────
if(COMPUTE_ENABLE_CUDA)
  enable_language(CUDA)
  set(CMAKE_CUDA_STANDARD 17)
  set(CMAKE_CUDA_STANDARD_REQUIRED ON)
  include(cmake/CUDAConfig.cmake)
endif()

# ── Compiler flags ────────────────────────────────────────────────────────────
include(cmake/CompilerOptions.cmake)

# ── Dependencies ─────────────────────────────────────────────────────────────
include(FetchContent)

# nanobind
if(COMPUTE_ENABLE_PYTHON)
  FetchContent_Declare(
    nanobind
    GIT_REPOSITORY https://github.com/wjakob/nanobind
    GIT_TAG        v2.4.0
    GIT_SHALLOW    TRUE
  )
  FetchContent_MakeAvailable(nanobind)
  include(cmake/PythonBindings.cmake)
endif()

# Google Benchmark (added Week 2)
if(COMPUTE_ENABLE_BENCH)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG        v1.9.1
    GIT_SHALLOW    TRUE
  )
  FetchContent_MakeAvailable(benchmark)
endif()

# System libraries (already installed via pacman)
find_package(TBB REQUIRED)
find_package(FFTW3 QUIET)  # optional; used in numerical and stats modules

if(COMPUTE_ENABLE_VULKAN)
  find_package(Vulkan REQUIRED)
endif()

# ── Core library ─────────────────────────────────────────────────────────────
add_library(compute_core STATIC
  src/compute/core/compute_policy.cpp
)
target_include_directories(compute_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_options(compute_core PUBLIC
  $<$<CXX_COMPILER_ID:GNU,Clang>:$<$<CONFIG:Release,RelWithDebInfo>:-O3>>
)
target_link_libraries(compute_core PUBLIC TBB::tbb)
set_target_properties(compute_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ── Module libraries ──────────────────────────────────────────────────────────
add_subdirectory(src/compute)

# ── Python bindings ───────────────────────────────────────────────────────────
if(COMPUTE_ENABLE_PYTHON)
  add_subdirectory(bindings/python)
endif()

# ── Tests ────────────────────────────────────────────────────────────────────
if(COMPUTE_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests/unit)
endif()

# ── Benchmarks ───────────────────────────────────────────────────────────────
if(COMPUTE_ENABLE_BENCH)
  add_subdirectory(tests/bench)
endif()
