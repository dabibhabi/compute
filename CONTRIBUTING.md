# Contributing to Compute Engine

## Overview

Compute Engine is a hand-implemented C++23 math library. Every algorithm is written from scratch — no Eigen, BLAS, or Boost in `include/` or `src/`. Third-party libraries are allowed **only in tests** as oracles.

**Read this before touching GPU or dispatch code.** The stack is layered on purpose; skipping layers causes subtle bugs (wrong layout, silent CPU fallback, device memory leaks).

## Depth warning — how complex this codebase is

| Layer | Complexity | Touch if you… |
|-------|------------|---------------|
| `include/compute/linalg/*.hpp` | Low | Add a new public op signature |
| `src/compute/linalg/<op>.cpp` | Low–medium | Implement CPU loop + `run_with_gpu()` hook |
| `include/compute/linalg/detail/` | Medium | Shared validation, thresholds, routing helpers |
| `src/compute/linalg/dispatch/` | **High** | Change backend priority or policy semantics |
| `src/compute/cuda/` | **High** | CUDA kernels, cuBLAS, streams, device memory |
| `src/compute/metal/`, `vulkan/` | **High** | Platform GPU backends |
| `bindings/python/` | Medium | NumPy layout rules, exception mapping |

**Do not** open a CUDA `.cu` file to “fix” a Python shape error. **Do not** add `cudaStream_t` to public headers. **Do not** bypass `run_with_gpu()` and call `backends::cuda::*` from `add.cpp` — policy and CPU fallback live in one place.

Tier 1 CPU code is meant to be readable (~20 lines per op). GPU dispatch is inherently deeper: streams, transfers, leading dimensions, and three backends. Budget time accordingly.

Architecture diagram: [`docs/diagrams/gpu-dispatch.excalidraw`](docs/diagrams/gpu-dispatch.excalidraw) (open with [Excalidraw](https://excalidraw.com) or the VS Code extension).

Changelog: [`CHANGELOG.md`](CHANGELOG.md).

## Project layout

```
include/compute/       Public API (pure C++, no CUDA/Metal/Vulkan types)
src/compute/           Compiled implementations
  linalg/              CPU ops + policy routing
  linalg/dispatch/     Multi-backend GPU selection
  cuda/                NVIDIA path (kernels in .cu, dispatch in .cpp)
  metal/               Apple path (macOS)
  vulkan/              Cross-vendor compute (scaffold)
bindings/python/       nanobind extension modules
tests/unit/            Catch2 (+ Eigen oracle in tests only)
docs/diagrams/         Architecture drawings
```

## Design principles

### Non-owning views over raw buffers

Matrices are passed as `const_matrix_view` / `matrix_view` — thin wrappers around `std::span<double>` with row/column dimensions. Callers own the memory (`std::vector`, `core::aligned_buffer`, numpy arrays at the binding layer).

```cpp
std::vector<double> buf(m * n);
auto A = *const_matrix_view::create(buf, m, n);
double v = A(i, j);  // column-major: index = j * rows + i
```

Views are created via static factories that return `core::Result<T>`. Never construct a view with invalid `(data, rows, cols)`.

### Errors via `std::expected`

All fallible operations return `core::Result<T>` (`std::expected<T, ComputeError>`). Check with `.has_value()` or the boolean operator. Propagate errors by returning early — no exceptions in library code.

```cpp
if (auto check = check_same_shape(A, B, C); !check) return check;
```

Convert to Python exceptions **only** in `bindings/python/`.

### One header, one translation unit per operation

| File | Role |
|------|------|
| `include/compute/linalg/add.hpp` | Public declaration |
| `src/compute/linalg/add.cpp` | CPU + `run_with_gpu()` |
| `include/compute/linalg/detail/elementwise.hpp` | Shared internal helpers (not in umbrella header) |

Keep headers minimal: includes, namespace, `[[nodiscard]]` declaration. No logic in headers except trivial inline helpers in `layout.hpp`.

### Column-major storage

Element `(i, j)` of an `m × n` matrix is at `j * m + i`. Loop with `j` outer, `i` inner for cache-friendly column access. Eigen's default `MatrixXd` layout matches this — use it as the test oracle.

## Python bindings (nanobind)

GPU-powered ops are called from Python through the same C++ entry points — there is no separate “GPU Python API”. Policy controls whether CUDA/Metal/Vulkan runs.

### Modules

| Extension | Package import | Role |
|-----------|----------------|------|
| `_compute_core` | `compute.ComputePolicy`, `set_policy`, … | Thread-local dispatch policy |
| `_compute_linalg` | `compute.gemm`, `compute.add`, … | All Tier 1 linalg ops |

Add `scripts/python` to `PYTHONPATH`, or install the built extensions next to the package:

```bash
cmake --preset release-cuda
cmake --build build/release-cuda --parallel
export PYTHONPATH="build/release-cuda/bindings/python:scripts/python"
python scripts/python/demo_linalg.py   # requires numpy
```

Install NumPy for the demo (`pacman -S python-numpy`, or a venv with `pip install numpy`).

### NumPy layout (critical)

C++ storage is **column-major** (Fortran order). Bindings require `stride(0) == 1` on 2-D arrays.

```python
import numpy as np
from compute import ComputePolicy, gemm, set_policy

m, k, n = 512, 512, 512
A = np.empty((m, k), order="F")
B = np.empty((k, n), order="F")
C = np.empty((m, n), order="F")

set_policy(ComputePolicy.gpu_preferred)  # force CUDA when built with COMPUTE_ENABLE_CUDA
gemm(A, B, C)
```

Default C-order NumPy arrays **will be rejected** — this is intentional zero-copy safety, not a bug.

### Policy from Python

```python
from compute import ComputePolicy, get_policy, set_policy

set_policy(ComputePolicy.automatic)    # default — size thresholds apply
set_policy(ComputePolicy.gpu_preferred)  # require GPU; RuntimeError if unavailable
set_policy(ComputePolicy.cpu_only)   # never offload
```

`ComputePolicy` is thread-local (same as C++). The variable lives in `src/compute/core/compute_policy.cpp` so nanobind shared modules can link it (header-only `thread_local` breaks PIC/LTO).

### Adding a Python binding for a new op

1. Implement C++ op in `src/compute/linalg/`.
2. Add `py_<op>` wrapper in `bindings/python/linalg_bind.cpp` — validate shapes, map `Result` → `std::runtime_error`.
3. Export in `NB_MODULE(_compute_linalg, …)`.
4. Re-export from `scripts/python/compute/__init__.py`.

## GPU backends (CUDA / Metal / Vulkan)

Optional GPU paths live behind the dispatch layer. Public headers stay pure C++.

```
Python / C++ caller
    → compute::linalg::<op>     (src/compute/linalg/<op>.cpp)
    → detail::run_with_gpu()    (policy + thresholds)
    → detail::gpu::<op>         (src/compute/linalg/dispatch/gpu_dispatch.cpp)
    → backends::{cuda,metal,vulkan}::<op>
```

See [`docs/diagrams/gpu-dispatch.excalidraw`](docs/diagrams/gpu-dispatch.excalidraw).

### CMake options

| Option | Default | Platform |
|--------|---------|----------|
| `COMPUTE_ENABLE_CUDA` | OFF | Linux + NVIDIA |
| `COMPUTE_ENABLE_METAL` | OFF | macOS (`-DCOMPUTE_ENABLE_METAL=ON`) |
| `COMPUTE_ENABLE_VULKAN` | OFF | Linux (Lavapipe or discrete GPU) |
| `COMPUTE_ENABLE_PYTHON` | ON | nanobind modules |

```bash
cmake --preset release-cuda    # CUDA + Python
cmake --preset release-gpu     # CUDA + Vulkan shaders + Python
```

### Dispatch policy

`core::ComputePolicy` controls routing:

- `cpu_only` — never calls GPU
- `gpu_preferred` — GPU required; error if backend fails
- `automatic` (default) — GPU when problem size exceeds threshold, else CPU

Thresholds in `include/compute/linalg/detail/gpu_dispatch.hpp`:

| Workload | Threshold |
|----------|-----------|
| Element-wise | ≥ 65536 elements (`256×256`) |
| GEMM | `n ≥ 400` |
| Vectors (dot, gemv) | ≥ 4096 elements |

### Adding a GPU path for an operation

1. CPU implementation in `src/compute/linalg/<op>.cpp` with `detail::run_with_gpu()`.
2. `compute::backends::cuda::<op>` in `src/compute/cuda/linalg/` (kernels in `.cu`, host code in `.cpp`).
3. Mirror signature in `include/compute/backends/gpu_backend.hpp`.
4. Ops struct entry in `src/compute/linalg/dispatch/gpu_dispatch.cpp`.
5. `[gpu][cuda]` test in `tests/unit/test_linalg_gpu.cpp` with `gpu_preferred`.
6. Python wrapper in `bindings/python/linalg_bind.cpp` if exposing to users.

Metal and Vulkan mirror the same `compute::backends::*` interface. **CUDA host code uses C++23; `.cu` files contain only `__global__` kernels and C launchers** — nvcc does not use the same standard as the host compiler.

### Testing GPU code

```bash
cmake --preset release-cuda
cmake --build build/release-cuda --parallel
./build/release-cuda/tests/unit/test_linalg   # needs GPU access for CUDA cases
ctest -R linalg --output-on-failure
```

Metal tests: build on macOS with `COMPUTE_ENABLE_METAL=ON`. Tests tagged `[metal]` skip elsewhere.

CUDA types must not appear outside `src/compute/cuda/`.

## Adding a new linalg operation

1. **Header** — `include/compute/linalg/<op>.hpp` with `core::Result` signature using views or spans.
2. **Source** — `src/compute/linalg/<op>.cpp` with validation, CPU loop, `run_with_gpu()`.
3. **CMake** — add `<op>.cpp` to `src/compute/linalg/CMakeLists.txt`.
4. **Umbrella** — `#include` the header in `include/compute/linalg.hpp`.
5. **Test** — `TEST_CASE` in `tests/unit/test_linalg.cpp` (hand case + Eigen oracle + mismatch error).
6. **GPU** (optional) — backend + dispatch + `test_linalg_gpu.cpp`.
7. **Binding** — `bindings/python/linalg_bind.cpp` + `compute/__init__.py`.

## Code style

- C++23: `std::span`, `std::expected`, `[[nodiscard]]`
- `std::size_t` for sizes and indices
- No comments explaining what code does — names should be clear
- Comments only for non-obvious *why* (numerical stability, hardware constraints)
- No docstrings or multi-line comment blocks in C++ source

## Build and test

```bash
cmake --preset relwithdebinfo          # CPU only
cmake --preset release-cuda            # CPU + CUDA + Python
cmake --build build/<preset> --parallel
ctest -R '^(core|linalg)$' --output-on-failure
```

## What not to do

- `#include <Eigen/...>` or any math library in `include/` or `src/`
- CUDA / Metal / Vulkan types in public headers
- Owning matrix classes that hide allocation in the hot path API
- Exceptions for dimension mismatches inside `src/compute/` (except nanobind boundary)
- Arbitrary test tolerances — derive from the algorithm's error bound
- C-order NumPy arrays in Python bindings without copying (we reject them explicitly)
- Calling cuBLAS or GPU kernels directly from `linalg/<op>.cpp` — use the dispatch layer

## Current focus

**Linear Algebra Tier 1** — CPU fundamentals done; **CUDA GPU path** implemented; Metal/Vulkan scaffolds; **Python bindings** for all Tier 1 ops. Next: Tier 2 CPU GEMM, Vulkan runtime, Metal on macOS. See `todo.md` and `CHANGELOG.md`.
