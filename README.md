# Compute Engine

Hand-implemented C++23 math library with optional CUDA/Metal/Vulkan acceleration and Python bindings via nanobind. No Eigen, BLAS, or Boost in the engine — third-party libs are test oracles only.

## Quick start

```bash
cmake --preset relwithdebinfo
cmake --build build/relwithdebinfo --parallel
ctest --test-dir build/relwithdebinfo --output-on-failure
```

With CUDA and Python:

```bash
cmake --preset release-cuda
cmake --build build/release-cuda --parallel
export PYTHONPATH="build/release-cuda/bindings/python:scripts/python"
python scripts/python/demo_linalg.py
```

NumPy arrays must be **column-major** (`order="F"`). See [CONTRIBUTING.md](CONTRIBUTING.md#python-bindings-nanobind).

## Documentation

| Doc | Purpose |
|-----|---------|
| [CONTRIBUTING.md](CONTRIBUTING.md) | How to add ops, GPU paths, and Python bindings — **read before touching dispatch/CUDA** |
| [CHANGELOG.md](CHANGELOG.md) | Release notes |
| [docs/diagrams/gpu-dispatch.excalidraw](docs/diagrams/gpu-dispatch.excalidraw) | Architecture: Python → nanobind → linalg → CPU/GPU backends |
| [todo.md](todo.md) | Module roadmap |

## Python API

```python
import numpy as np
from compute import ComputePolicy, gemm, set_policy

A = np.empty((512, 512), order="F")
B = np.empty((512, 512), order="F")
C = np.empty((512, 512), order="F")

set_policy(ComputePolicy.gpu_preferred)  # CUDA when built with COMPUTE_ENABLE_CUDA
gemm(A, B, C)
```

Modules: `_compute_core` (policy), `_compute_linalg` (Tier 1 linear algebra).

## Build presets

| Preset | Use |
|--------|-----|
| `relwithdebinfo` | Default dev — CPU only |
| `release-cuda` | CUDA + Python |
| `release-gpu` | CUDA + Vulkan shaders + Python |

## License

See repository for license terms.
