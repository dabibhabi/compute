---
tags:
  - index
  - compute-engine
aliases:
  - home
  - index
---

# Compute Engine

> [!info] What This Is
> A math compute engine built from scratch in **C++23 / CUDA / Python (nanobind)**.
> Every algorithm is hand-implemented — no math libraries as dependencies.
> Dual goal: a real HPC library **and** a LinkedIn content series (3–4 posts/week, one topic per post).

---

## Status

| Module | Tier | Status |
|--------|------|--------|
| Core infrastructure | — | ✅ Done |
| [[Linear Algebra]] | Tier 1 – Fundamentals | 🔄 In progress |
| [[Linear Algebra]] | Tier 2 – Performance | ⬜ Not started |
| [[Linear Algebra]] | Tier 3 – Decompositions | ⬜ Not started |
| [[Linear Algebra]] | Tier 4 – Eigenvalues | ⬜ Not started |
| [[Linear Algebra]] | Tier 5 – GPU | ⬜ Not started |
| [[DSA]] | Tiers 1–7 | ⬜ Not started |
| [[Numerical Analysis]] | — | ⬜ Not started |
| [[ODE Solvers]] | — | ⬜ Not started |
| [[PDE]] | — | ⬜ Not started |
| [[Statistical Analysis]] | — | ⬜ Not started |
| [[Probability Theory]] | — | ⬜ Not started |
| [[Combinatorics]] | — | ⬜ Not started |

---

## Map of Content

### Active Work
- [[Linear Algebra]] — start here
  - [[linalg/Tier 1 - Fundamentals|Tier 1: Fundamentals]] ← current
  - [[linalg/Tier 2 - Performance|Tier 2: Performance]]
  - [[linalg/Tier 3 - Decompositions|Tier 3: Decompositions]]
  - [[linalg/Tier 4 - Eigenvalues & Solvers|Tier 4: Eigenvalues & Solvers]]
  - [[linalg/Tier 5 - GPU|Tier 5: GPU]]

### DSA
- [[DSA]] — runs in parallel with math modules
  - [[dsa/Tier 1 - Memory Aware Foundations|Tier 1: Memory-Aware Foundations]]
  - [[dsa/Tier 2 - Trees & Range Structures|Tier 2: Trees & Range Structures]]
  - [[dsa/Tier 3 - Graph Algorithms|Tier 3: Graph Algorithms]]
  - [[dsa/Tier 4 - Sorting & Order Statistics|Tier 4: Sorting & Order Statistics]]
  - [[dsa/Tier 5 - Advanced & Amortized|Tier 5: Advanced & Amortized]]
  - [[dsa/Tier 6 - Paradigms & Complexity|Tier 6: Paradigms & Complexity]]
  - [[dsa/Tier 7 - GPU Parallel DSA|Tier 7: GPU-Parallel DSA]]

### Math Modules (after linear algebra)
- [[Numerical Analysis]]
- [[ODE Solvers]]
- [[PDE]]
- [[Statistical Analysis]]
- [[Probability Theory]]
- [[Combinatorics]]

### Reference
- [[References]] — all textbooks, papers, and free resources by module

---

## Tech Stack

| Layer | Choice | Notes |
|-------|--------|-------|
| Core | C++23 | `std::mdspan`, `std::expected`, `std::print` |
| GPU | CUDA SM 86 | RTX 3060, optional — `COMPUTE_ENABLE_CUDA=ON` |
| Python | nanobind | Zero-copy numpy arrays |
| Build | CMake 4.3 + Ninja | Presets: debug / relwithdebinfo / release / release-cuda |
| CPU parallel | OneTBB | Prefer over OpenMP |
| FFT | FFTW3 (CPU) / cuFFT (GPU) | |
| Tests | Catch2 + Google Benchmark | |
| Oracles | Eigen, scipy/numpy | Test-only, never linked into engine |

---

## Post Cadence

3–4 LinkedIn posts per week. Each post picks **one angle**:

| Angle | What it covers |
|-------|---------------|
| **Algorithm** | The math insight, convergence behavior, the "aha" |
| **C++ design** | Patterns used — `constexpr` tableau, dispatch, `std::expected` |
| **Devops/build** | CMake decisions, profiling, CI, Docker |
| **Performance** | FLOPS chart, GB/s number, before/after comparison |

---

## Hardware

- **CPU**: Ryzen 7 9800X3D — AVX-512, 96 MB 3D V-Cache
- **GPU**: RTX 3060 12 GB — Ampere, SM 8.6, 28 SMs
- **OS**: Arch Linux, GCC 16 / Clang 22, Python 3.14
