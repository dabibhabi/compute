---
tags:
  - linear-algebra
  - module
---

# Linear Algebra

> [!info] Module Goal
> Build from fundamentals to production-grade decompositions and GPU acceleration.
> The progression is intentional — each tier builds vocabulary the next tier needs.

---

## Tiers

| Tier | Focus | Status |
|------|-------|--------|
| [[linalg/Tier 1 - Fundamentals\|Tier 1]] | Addition, transpose, dot product, naive GEMM, norms | 🔄 In progress |
| [[linalg/Tier 2 - Performance\|Tier 2]] | Tiled GEMM, AVX-512, cache-oblivious, Strassen | ⬜ |
| [[linalg/Tier 3 - Decompositions\|Tier 3]] | LU, Cholesky, QR, SVD, RRQR | ⬜ |
| [[linalg/Tier 4 - Eigenvalues & Solvers\|Tier 4]] | Power iteration, CG, Arnoldi, GMRES | ⬜ |
| [[linalg/Tier 5 - GPU\|Tier 5]] | CUDA SGEMM kernel, cuBLAS dispatch, cuSPARSE | ⬜ |

---

## Namespace

```cpp
compute::linalg
```

All public headers live in `include/compute/linalg/`.
Compiled units in `src/compute/linalg/`.
GPU kernels in `src/compute/cuda/linalg/`.

---

## Key Design Decisions

> [!note] Column-major storage
> All matrices use column-major (Fortran order) layout. This makes column operations — which appear in every decomposition (Householder, LU panel factorization) — cache-friendly. The `std::mdspan` view is set up with a column-major mapping.

> [!note] No CUDA types in public headers
> The GPU path is hidden behind the dispatch layer. `include/compute/linalg/` is pure C++. Users call the same function regardless of whether CUDA is enabled.

---

## References (quick links)

- Fundamentals → Strang *Introduction to Linear Algebra* Ch 1–4, MIT OCW 18.06
- Decompositions → Trefethen & Bau *Numerical Linear Algebra*
- Production algorithms → Golub & Van Loan *Matrix Computations* 4th ed
- Full reference list → [[References]]
