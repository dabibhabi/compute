# Changelog

All notable changes to this project are documented here.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Versioning is early-stage (`0.1.x`); breaking API changes are expected until `1.0`.

## [Unreleased]

### Added

- **Linear algebra Tier 1 (CPU)** — `add`, `sub`, `scale`, `hadamard`, `transpose`, `dot`, `gemv`, `gemm`, `trace`, `norm_fro`, `norm_inf` under `compute::linalg`.
- **Non-owning matrix views** — `const_matrix_view` / `matrix_view` with column-major layout and `core::Result` validation.
- **GPU dispatch layer** — `ComputePolicy`-driven routing (`cpu_only`, `gpu_preferred`, `automatic`) with size thresholds in `detail/gpu_dispatch.hpp`.
- **CUDA backend** (`COMPUTE_ENABLE_CUDA`):
  - Custom kernels: elementwise ops, transpose, infinity norm.
  - cuBLAS: `Dgemm`, `Dgemv`, `Ddot`.
  - Device context with stream + cuBLAS handle.
- **Metal backend scaffold** (`COMPUTE_ENABLE_METAL`) — stub on Linux; real MSL path reserved for macOS.
- **Vulkan backend scaffold** (`COMPUTE_ENABLE_VULKAN`) — GLSL compute shaders compiled at build time; runtime context pending.
- **Python bindings** — `_compute_linalg` nanobind module with zero-copy NumPy I/O (Fortran-order matrices).
- **Tests** — `test_linalg.cpp` (CPU + Eigen oracle), `test_linalg_gpu.cpp` (CUDA with `gpu_preferred`).
- **CMake presets** — `release-cuda`, `release-gpu`.
- **Docs** — `CONTRIBUTING.md` (expanded), `docs/diagrams/gpu-dispatch.excalidraw`.

### Changed

- `compute::core` — `g_policy` is no longer `inline thread_local` in a header (required for nanobind shared modules).
- `src/compute/CMakeLists.txt` — `compute_linalg` is a real static library (no longer an INTERFACE placeholder).
- `cmake/CUDAConfig.cmake` — `CMAKE_CUDA_ARCHITECTURES` set to `86` (RTX 3060).
- CUDA host dispatch split: C++23 `.cpp` for `Result` / cuBLAS, `.cu` for `__global__` kernels only.

### Fixed

- Python extension link failure (`R_X86_64_TPOFF32` / thread-local `g_policy`) — `ComputePolicy` state moved to `src/compute/core/compute_policy.cpp`; `compute_core` is now a PIC static library.
- Eigen FetchContent test pollution — `EIGEN_BUILD_TESTING=OFF` when fetching for unit tests.
- cuBLAS `Dgemm` leading dimensions corrected for column-major storage.

## [0.1.0] — Week 1

### Added

- CMake 4.3 + Ninja skeleton with `debug`, `relwithdebinfo`, `release` presets.
- `compute::core` — `aligned_buffer`, `simd_traits`, `ComputePolicy`, `Result<T>`, version.
- nanobind `_compute_core` — `version`, `set_policy`, `get_policy`, `ComputePolicy`.
- Catch2 unit tests for core module.

[Unreleased]: https://github.com/your-org/compute/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/your-org/compute/releases/tag/v0.1.0
