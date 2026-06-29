# Compute Engine — Claude Context

## What This Project Is

A math compute engine built from scratch in C++23/CUDA with Python bindings via nanobind. Every algorithm is hand-implemented — no third-party math libraries in the engine. Two simultaneous goals: a real HPC library AND a LinkedIn content series (one function per post, 3–4 posts/week).

---

## Tech Stack

| Layer | Choice | Notes |
|-------|--------|-------|
| Core language | C++23 | `std::expected`, `std::span`, `std::mdspan`, `std::print` |
| GPU | CUDA SM 86 (RTX 3060) | Optional via `COMPUTE_ENABLE_CUDA=ON` |
| Python | nanobind zero-copy `nb::ndarray` | Arrays must be Fortran-order (`order='F'`) |
| Build | CMake 4.3 + Ninja presets | `relwithdebinfo`, `release-cuda`, `release-gpu` |
| CPU parallel | OneTBB | Prefer over OpenMP or raw threads |
| FFT | FFTW3 (CPU), cuFFT (GPU) | System install |
| Testing | Catch2 (unit), Google Benchmark (perf) | Eigen/numpy as test oracles only |

---

## Hard Rules

- **No math libraries as dependencies.** Eigen and Boost.Math exist on the system but are only used in `tests/` as oracles. Never `#include` them in `include/` or `src/`.
- **No Boost in the engine.** All special functions (erf, gamma, beta, Legendre, Bessel) are implemented from scratch.
- **C++23 everywhere.** Use `std::expected` for error handling, `std::span` for buffer views.
- **CUDA is optional.** `COMPUTE_ENABLE_CUDA=OFF` must compile and pass all tests cleanly.
- **No GPU types in public headers.** `cudaStream_t`, `float4*`, etc. must not appear in `include/compute/`.
- **GPU kernels live in real shader/source files.** Metal kernels (MSL) go in a `.metal` file — never embedded as a C++ string literal; likewise CUDA in `.cu`, Vulkan in `.comp`. When the kernel compiler is unavailable (e.g. the Metal Toolchain `xcrun metal`/`metallib`), embed the `.metal` file into a generated header at build time (CMake `file(READ)` + `configure_file`) and compile it at runtime via `newLibraryWithSource:`; the `.metal` file stays the single source of truth, never a hand-written string.
- **`g_policy` lives in a `.cpp` file.** The `thread_local ComputePolicy g_policy` definition is in `src/compute/core/compute_policy.cpp`. Never `inline thread_local` in a header — nanobind shared objects trigger TLS relocation errors.
- **Higher modules depend on linalg, never the reverse.** `numerical`, `ode`, `stats`, `probability` call `compute::linalg` ops. `linalg` headers never include from those modules.

---

## Repository Layout

```
include/compute/             Public API — pure C++, no GPU types
  core/                      aligned_buffer, simd_traits, error.hpp, compute_policy
  linalg/                    layout + Tier 1 op headers
  linalg/detail/             gpu_dispatch.hpp, route.hpp, elementwise.hpp
  backends/                  gpu_backend.hpp (cuda/metal/vulkan backend signatures)
  numerical/ ode/ stats/     planned modules (placeholder headers)
  probability/ combinatorics/

src/compute/
  core/                      compute_policy.cpp
  linalg/                    CPU op .cpp files
  linalg/dispatch/           gpu_dispatch.cpp (backend selector)
  cuda/detail/               context.cpp, context.hpp (stream, cuBLAS handle)
  cuda/linalg/               kernels.cu + linalg_dispatch.cpp
  metal/ vulkan/             stubs

bindings/python/             nanobind extensions — _compute_<module>.cpp per module
tests/unit/                  Catch2 — test_<module>.cpp
tests/bench/                 Google Benchmark — bench_<module>.cpp
scripts/python/compute/      Python package (__init__.py re-exports extensions)
docs/diagrams/               Excalidraw architecture diagrams
docs/journal/                Dated session notes (LaTeX, mermaid, file refs)
cmake/                       CompilerOptions.cmake, CUDAConfig.cmake, PythonBindings.cmake
```

---

## Namespace Layout

```
compute::core        aligned_buffer, simd_traits, ComputePolicy, Result<T>
compute::linalg      matrix ops, decompositions, iterative solvers
compute::num         root-finding, interpolation, FFT, quadrature, autodiff
compute::ode         RK4, DOPRI5, SDIRK2, Verlet
compute::stats       descriptive stats, regression, hypothesis tests, KDE
compute::prob        PRNG, special functions, distributions, MCMC, Monte Carlo
compute::comb        combinatorial functions, DP algorithms, graph kernels
compute::backends    GPU dispatch interface (cuda, metal, vulkan sub-namespaces)
```

---

## Adding Functionality — Step-by-Step

This is the exact sequence for adding any new linalg op. Do not skip steps.

### New linalg op checklist

1. **`include/compute/linalg/<op>.hpp`** — public declaration using `const_matrix_view` / `matrix_view` / `std::span<double>`, `[[nodiscard]]`, `noexcept`, returns `core::Result<T>`.

2. **`src/compute/linalg/<op>.cpp`** — inner `<op>_cpu()` with validation + scalar loop (j outer, i inner for column-major). Public function calls `detail::run_with_gpu()` or `run_scalar_with_gpu()`.

3. **`src/compute/linalg/CMakeLists.txt`** — add `<op>.cpp` to `add_library(compute_linalg STATIC ...)`.

4. **`include/compute/linalg/detail/gpu_dispatch.hpp`** — add `[[nodiscard]] core::Result<...> <op>(...)` stub declaration inside `namespace compute::linalg::detail::gpu`.

5. **`src/compute/linalg/dispatch/gpu_dispatch.cpp`** — add `struct <Op>Ops` and wire into `run_void()` / `run_scalar()` following the existing pattern.

6. **`include/compute/linalg.hpp`** — add `#include "compute/linalg/<op>.hpp"`.

7. **`tests/unit/test_linalg.cpp`** — three `TEST_CASE` blocks: hand-computed small case, Eigen oracle, dimension mismatch error.

8. **`bindings/python/linalg_bind.cpp`** — `py_<op>()` wrapper using `require_fortran_matrix` / `as_const_matrix` / `as_matrix`, mapping `Result` errors to `std::runtime_error`.

9. **`scripts/python/compute/__init__.py`** — re-export `<op>` from `_compute_linalg`, add to `__all__`.

### New module checklist (numerical, ode, stats, ...)

1. Replace the INTERFACE placeholder in `src/compute/CMakeLists.txt` with a real STATIC library linking `PUBLIC compute_linalg`.
2. Add `include/compute/<module>/` headers and `include/compute/<module>.hpp` umbrella.
3. Add `src/compute/<module>/` implementation files.
4. Add `bindings/python/<module>_bind.cpp` and register with `compute_python_module()` in `bindings/python/CMakeLists.txt`.
5. Add `tests/unit/test_<module>.cpp`.
6. Implementations call `compute::linalg::gemm`, `gemv`, `dot`, etc. — no matrix loops.

---

## GPU Dispatch Architecture

```
<op>.cpp  →  detail::run_with_gpu(cpu_fn, gpu_fn, size_fn)
               │
               └── detail::gpu::<op>()          [dispatch/gpu_dispatch.cpp]
                     ├── backends::cuda::<op>()  [cuda/linalg/linalg_dispatch.cpp]
                     ├── backends::metal::<op>() [metal/stub.cpp]
                     └── backends::vulkan::<op>()[vulkan/stub.cpp]
```

GPU offload thresholds (in `include/compute/linalg/detail/gpu_dispatch.hpp`):
- Element-wise: `element_threshold = 65536` elements
- GEMM: `gemm_n_threshold = 400`
- Vectors: `vector_threshold = 4096` elements

CUDA `.cu` files contain only `__global__` kernels and `extern "C"` launchers — nvcc does not support C++23. Host dispatch uses `.cpp` with `std::expected`.

---

## Python Bindings — Critical Details

- NumPy arrays must be **column-major** (`order='F'`). Bindings enforce `stride(0) == 1` and throw `ValueError` otherwise.
- `Result<T>` errors convert to `std::runtime_error` only at the nanobind boundary (`bindings/python/`).
- `ComputePolicy` is thread-local. Setting it from Python affects the calling thread only.
- Build and run:
  ```bash
  cmake --preset release-cuda
  cmake --build build/release-cuda --parallel
  export PYTHONPATH="build/release-cuda/bindings/python:scripts/python"
  python scripts/python/demo_linalg.py
  ```

---

## Build Commands

```bash
# CPU-only dev build
cmake --preset relwithdebinfo
cmake --build build/relwithdebinfo --parallel
ctest --test-dir build/relwithdebinfo --output-on-failure

# CUDA + Python
cmake --preset release-cuda
cmake --build build/release-cuda --parallel
ctest --test-dir build/release-cuda --output-on-failure

# CUDA + Vulkan shaders
cmake --preset release-gpu
cmake --build build/release-gpu --parallel
```

---

## Current Progress

- [x] Week 1 — Core infrastructure: `aligned_buffer`, `simd_traits`, `ComputePolicy`, `Result<T>`, Python binding `_compute_core`
- [x] Week 2 — Linalg Tier 1: all 11 ops, column-major views, GPU dispatch layer, CUDA backend (cuBLAS + kernels), Metal backend (macOS, float32), Vulkan scaffold, Python bindings `_compute_linalg`, CI
- [ ] Week 3 — Linalg Tier 2: loop reorder, tiled GEMM, AVX-512 FMA micro-kernel, TBB parallel element-wise, bench_linalg
- [ ] Week 4 — Linalg Tier 3: LU, Cholesky, Householder QR, SVD
- [ ] Week 5+ — Numerical module: Newton, bisection, Gauss-Legendre, dual-number autodiff; uses linalg

See `todo.md` for the full module roadmap.

---

## Hardware Context

- **CPU**: Ryzen 7 9800X3D — AVX-512 (`simd_traits<double>::width = 8`), 96 MB 3D V-Cache. Tile sizes for blocked GEMM can be larger than textbook (96 MB vs typical 1–8 MB L3). Measure empirically with `bench_linalg`.
- **GPU**: RTX 3060 12 GB, Ampere SM 8.6. 28 SMs, 256 threads/block default launch.
- **OS**: Arch Linux, GCC 16 / Clang 22, Python 3.12+.

---

## Code Style

- No comments explaining what code does — identifiers do that.
- Comments only for non-obvious **why**: numerical stability, alignment invariants, cache tiling rationale, hardware-specific workarounds.
- No docstrings. No multi-line comment blocks in C++.
- `std::size_t` for sizes and indices. `ptrdiff_t` for signed differences.
- Column-major: element `(i,j)` of an `m x n` matrix is at `data[j*m + i]`.
- Test tolerances from the algorithm's theoretical error bound — never magic constants.

---

## LinkedIn Post Angles (per function)

1. **Algorithm** — the math insight, convergence behaviour, "aha" moment
2. **C++ design** — patterns used (`constexpr` tableau, `std::expected`, dispatch, SIMD traits)
3. **Devops/build** — CMake decisions, profiling setup, CI integration
4. **Performance** — GFLOPS chart, GB/s measurement, before/after comparison

---

## Common Mistakes to Avoid

- Putting `inline thread_local` policy in a header — breaks nanobind `.so` builds (TLS relocation)
- Adding CUDA/Metal/Vulkan types in `include/compute/` — violates public API purity
- Calling `backends::cuda::*` directly from an op `.cpp` — always route through `detail::run_with_gpu()`
- Using C-order NumPy arrays without copying — bindings explicitly reject `stride(0) != 1`
- Arbitrary test tolerances — derive from the algorithm's proven error bound
- Adding math library includes (`<Eigen/...>`, `<boost/math/...>`) to `include/` or `src/`
- Defining a new matrix class — use `const_matrix_view` / `matrix_view` from `layout.hpp`
- Forgetting `POSITION_INDEPENDENT_CODE ON` on any library linked into a Python extension
