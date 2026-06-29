# Compute Engine — Claude Context

## What This Project Is

A math compute engine built from scratch in C++/CUDA with Python bindings. Every algorithm is implemented by hand — no third-party math libraries in the engine itself. The goal is two things simultaneously: a real, benchmarkable HPC library AND a LinkedIn content series (3–4 posts/week, one topic per post).

## Tech Stack

| Layer | Choice |
|-------|--------|
| Core | C++23 (`std::mdspan`, `std::expected`, `std::print`) |
| GPU | CUDA (SM 86 — RTX 3060), enabled via `COMPUTE_ENABLE_CUDA=ON` |
| Python | nanobind (zero-copy `nb::ndarray` ↔ numpy) |
| Build | CMake 4.3 + Ninja, presets in `CMakePresets.json` |
| CPU parallelism | OneTBB |
| FFT | FFTW3 (CPU), cuFFT (GPU) |
| Testing | Catch2 (unit), Google Benchmark (perf) |
| Test oracles | Eigen, scipy/numpy — validation only, never linked into the engine |

## Hard Rules

- **No math libraries as dependencies.** Eigen and Boost.Math exist on the system but are only used in `tests/` as oracles to validate correctness. They are never `#include`d in `include/` or `src/`.
- **No Boost in the engine.** All special functions (erf, gamma, beta, Legendre, Bessel, etc.) are implemented from scratch.
- **C++23 everywhere.** Use `std::expected` for error handling (not exceptions in hot paths), `std::mdspan` for matrix views, `std::span` for buffer views.
- **CUDA is optional.** Every function that has a GPU path must also have a CPU path. The `COMPUTE_ENABLE_CUDA=OFF` build must compile and pass all tests cleanly.
- **No CUDA types in public headers.** `cudaStream_t`, `float4*`, etc. must not appear in `include/compute/`. The dispatch layer in `.cu` files is the only place CUDA types are used.
- **Metal kernels live in `.metal` files.** Any Metal shader/kernel code (MSL) must be written in a real `.metal` source file — never embedded as a C++ string literal. When the Metal Toolchain (`xcrun metal`/`metallib`) is unavailable, embed the `.metal` file into a generated header at build time (CMake `file(READ)` + `configure_file`) and compile it at runtime via `newLibraryWithSource:`; the `.metal` file stays the single source of truth. The same applies to other GPU kernels (`.cu`, `.comp`) — keep kernel code in real, syntax-highlighted source files, not strings.

## Repository Layout

```
include/compute/       ← public headers, one subdir per module
  core/                ← aligned_buffer, simd_traits, error.hpp, compute_policy
  linalg/
  numerical/
  ode/
  stats/
  probability/
  combinatorics/
src/compute/           ← compiled implementation units
  cuda/                ← .cu files, mirrors the module tree
bindings/python/       ← nanobind extension modules (_compute_<module>.cpp)
tests/unit/            ← Catch2, one file per module
tests/bench/           ← Google Benchmark, one file per module
scripts/python/        ← demo scripts, notebooks
docs/weekly/           ← LinkedIn post drafts (weekN.md)
cmake/                 ← CompilerOptions, CUDAConfig, PythonBindings
```

## Namespace Layout

```
compute::core        aligned_buffer, simd_traits, ComputePolicy, Result<T>
compute::linalg      matrix ops, decompositions, iterative solvers
compute::num         root-finding, interpolation, FFT, quadrature, autodiff
compute::ode         RK4, DOPRI5, SDIRK2, Verlet
compute::stats       descriptive stats, regression, hypothesis tests, KDE
compute::prob        PRNG, special functions, distributions, MCMC, Monte Carlo
compute::comb        combinatorial functions, DP algorithms, graph kernels
compute::cuda        GPU dispatch wrappers (CPU path always exists as fallback)
```

## Build Commands

```bash
# Configure (first time)
cmake --preset relwithdebinfo

# Build
cmake --build build/relwithdebinfo --parallel

# Test
cd build/relwithdebinfo && ctest --output-on-failure

# With CUDA (Week 4+, requires pacman -S cuda)
cmake --preset release-cuda
cmake --build build/release-cuda --parallel
```

## Current Progress

- [x] Week 1 — Foundation: CMake skeleton, core headers, Python binding working
- [ ] Week 2 — Linear Algebra Tier 1 (fundamentals: add, mul, transpose, dot, norms)
- GPU backends: CUDA (Linux/NVIDIA) and Metal (macOS, float32) implemented for all Tier 1 ops; Vulkan scaffold only.

See `todo.md` for the full module roadmap.

## Hardware Context

- **CPU**: Ryzen 7 9800X3D — AVX-512, 96 MB 3D V-Cache. Tile size decisions differ from textbook advice because of the massive L3 cache.
- **GPU**: RTX 3060 12 GB, Ampere SM 8.6. 28 SMs, 256 threads/block is the default launch config.
- **OS**: Arch Linux

## Code Style

- No comments explaining what the code does — identifiers should do that
- Comments only for non-obvious WHY: hidden constraints, numerical stability reasons, SIMD alignment invariants
- No docstrings. No multi-line comment blocks.
- Errors propagate via `compute::core::Result<T>` (`std::expected`). Convert to Python exceptions only at the nanobind boundary.
- Tests use tolerances derived from the algorithm's theoretical error bound, not magic constants.

## LinkedIn Post Angles (per function)

Each implemented function targets one of these post angles:
1. **Algorithm** — the math insight, the "aha", convergence behavior
2. **C++ design** — how it's structured, patterns used (`constexpr` tableau, dispatch, etc.)
3. **Devops/build** — CMake decisions, profiling setup, CI, Docker
4. **Performance** — FLOPS chart, GB/s number, before/after comparison
