# Compute Engine — Todo

## Status
- [x] Week 1 — Project foundation (CMake, core headers, Python binding)

---

## Linear Algebra (current focus)

Build from fundamentals up. Each item is one LinkedIn post.

### Tier 1 — Fundamentals

Scaffold files created under `include/compute/linalg/`, `src/compute/linalg/`, etc.
Implement in this order — each op follows the same pipeline: header → `.cpp` → test → binding.

#### Setup (do once before any op)
- [x] `layout.hpp` — column-major index `(i,j) → j*m+i`, dimension validation, `Result<void>` helpers
- [x] `linalg.hpp` — umbrella `#include` of all op headers
- [x] `src/compute/linalg/CMakeLists.txt` — `compute_linalg` STATIC lib listing all `.cpp` files
- [x] `src/compute/CMakeLists.txt` — replace linalg INTERFACE placeholder with `add_subdirectory(linalg)`
- [x] `tests/unit/CMakeLists.txt` — `test_linalg` + Eigen3 (FetchContent fallback)
- [ ] `tests/bench/CMakeLists.txt` — `bench_linalg` (GEMM baseline GFLOPS)
- [ ] `bindings/python/CMakeLists.txt` — `_compute_linalg` module
- [x] Verify build: `cmake --preset release-cuda && ctest -R linalg`

#### Operations (one LinkedIn post each)
- [x] 1. Matrix addition / subtraction        (`add.hpp`, `sub.hpp` — `A + B`, `A - B`)
- [x] 2. Scalar multiplication                (`scale.hpp` — `alpha * A`)
- [x] 3. Element-wise multiply (Hadamard)     (`hadamard.hpp` — `A ⊙ B`)
- [x] 4. Matrix transpose                     (`transpose.hpp` — out-of-place `A^T`)
- [x] 5. Dot product                          (`dot.hpp` — `u · v`)
- [x] 6. Matrix-vector multiply               (`gemv.hpp` — `Ax`)
- [x] 7. Matrix-matrix multiply (naive)       (`gemm.hpp` — `AB`, intentional `i-j-k` loop)
- [x] 8. Trace, Frobenius norm, infinity norm (`trace.hpp`, `norm_fro.hpp`, `norm_inf.hpp`)

#### Per-op checklist (repeat for each)
- [x] Public header in `include/compute/linalg/` — pure C++, views, `core::Result<T>`, no CUDA types
- [x] Implementation in `src/compute/linalg/` — validate dims, scalar loop, column-major
- [x] `TEST_CASE` in `tests/unit/test_linalg.cpp` — hand-computed tiny case + Eigen oracle + mismatch error
- [x] Binding in `bindings/python/linalg_bind.cpp` — numpy in/out, `Result` → exception at boundary
- [ ] (GEMM/GEMV) benchmark in `tests/bench/bench_linalg.cpp` — record baseline GFLOPS for Tier 2

### Tier 2 — Performance
- [ ] Cache-friendly GEMM (tiled, column-major storage)
- [ ] AVX-512 FMA GEMM kernel (manual SIMD, leaf of recursive descent)
- [ ] Cache-oblivious recursive GEMM
- [ ] Strassen algorithm (7-multiply recursive scheme)

### Tier 3 — Decompositions
- [ ] LU decomposition with partial pivoting
- [ ] Cholesky (for symmetric positive definite systems)
- [ ] Householder QR
- [ ] Thin SVD (Golub-Reinsch)
- [ ] Rank-revealing QR

### Tier 4 — Eigenvalues & Iterative Solvers
- [ ] Power iteration
- [ ] Inverse iteration
- [ ] Rayleigh quotient iteration
- [ ] Conjugate Gradient (sparse Ax=b)
- [ ] Arnoldi iteration

### Tier 5 — GPU (Week 4+, requires `pacman -S cuda`)
- [ ] Tiled SGEMM CUDA kernel (educational — then compare to cuBLAS)
- [ ] cuBLAS DGEMM dispatch (CPU/GPU crossover at n≈400)
- [ ] Batched matrix inversion (cuBLAS, 10K independent 4×4)
- [ ] Sparse matvec via cuSPARSE

---

## DSA (Data Structures & Algorithms)

Performance-first perspective — assume CLRS-level knowledge, focus on cache behavior,
amortized analysis, and HPC-relevant variants. Each item is one LinkedIn post.

### Tier 1 — Memory-Aware Foundations
- [ ] Array-of-Structs vs Struct-of-Arrays layout (cache line analysis, SIMD implications)
- [ ] Ring buffer / circular queue (lock-free variant with atomics)
- [ ] Robin Hood hash table (open addressing, cache-friendly, better than std::unordered_map)
- [ ] Memory pool allocator (slab allocator — replaces malloc in hot paths)

### Tier 2 — Trees & Range Structures
- [ ] Binary heap (min/max, used in Dijkstra and ODE event detection)
- [ ] Fenwick tree / BIT (prefix sums in O(log n) — connects to GPU scan kernel)
- [ ] Segment tree with lazy propagation (range update, range query)
- [ ] Van Emde Boas tree (O(log log U) — cache-oblivious layout, theory post)
- [ ] B-tree / cache-oblivious BST layout (disk-friendly and L3-friendly access patterns)

### Tier 3 — Graph Algorithms
- [ ] BFS / DFS with adjacency list (CSR format — same format as sparse matrices)
- [ ] Dijkstra with binary heap vs Fibonacci heap (amortized analysis comparison)
- [ ] Bellman-Ford (negative cycle detection, dynamic programming view)
- [ ] Floyd-Warshall (all-pairs shortest path, DP on adjacency matrix)
- [ ] Topological sort — Kahn's algorithm and DFS variant
- [ ] Kruskal's MST + Union-Find with path compression + union by rank (inverse Ackermann analysis)
- [ ] Prim's MST (priority queue variant, dense graph comparison)

### Tier 4 — Sorting & Order Statistics
- [ ] Introsort (quicksort + heapsort fallback — what std::sort actually does)
- [ ] Mergesort — external sort variant (cache-oblivious merge, relevant for large datasets)
- [ ] Radix sort LSD (integer keys, O(nk) — connects to GPU parallel radix sort)
- [ ] Order statistics tree (augmented BST, median in O(log n))
- [ ] Median of medians (deterministic O(n) selection, theoretical vs practical tradeoff)

### Tier 5 — Advanced & Amortized
- [ ] Union-Find amortized analysis (inverse Ackermann function — math undergrad post)
- [ ] Splay trees (amortized O(log n), self-adjusting, working set theorem)
- [ ] Skip list (probabilistic alternative to balanced BST, lock-free variant)
- [ ] Persistent data structures (path copying, fat node — purely functional view)
- [ ] Suffix array + LCP array (SA-IS O(n) construction, used in string matching)

### Tier 6 — Algorithmic Paradigms & Complexity
- [ ] Divide and conquer master theorem (connects to Strassen, FFT, merge sort)
- [ ] Amortized analysis: aggregate, accounting, potential methods (Tarjan's framework)
- [ ] Randomized algorithms: Las Vegas vs Monte Carlo (connects to MCMC module)
- [ ] NP-completeness and reductions (theoretical context for TSP, combinatorics)
- [ ] Approximation algorithms: vertex cover, set cover, FPTAS (theory post)
- [ ] Online algorithms and competitive analysis (ski rental, k-server problem)

### Tier 7 — GPU-Parallel DSA
- [ ] Parallel prefix sum / scan (already in linalg GPU tier — link here)
- [ ] GPU parallel radix sort (connect to LSD radix sort from Tier 4)
- [ ] Parallel BFS (Beamer's direction-optimizing BFS)
- [ ] GPU hash table (cuckoo hashing on device memory)

---

## Upcoming Modules (after linear algebra)

- [ ] Numerical Analysis (root-finding, interpolation, FFT, quadrature, autodiff)
- [ ] ODE Solvers (RK4, DOPRI5, SDIRK2, Verlet, GPU batch IVP)
- [ ] PDE (finite differences, Crank-Nicolson, multigrid, GPU stencil)
- [ ] Statistical Analysis (Welford, OLS via QR, KDE, t-test, GPU covariance)
- [ ] Probability Theory (PCG64, special functions from scratch, MCMC, Monte Carlo)
- [ ] Combinatorics (Stirling, partition function, Held-Karp TSP, NTT, generating functions)

---

## Meta
- Tools: C++23, CUDA, Python (nanobind)
- Post cadence: 3–4 posts/week, one topic per post
- Each post angle: algorithm insight | C++ design | devops/build | performance result
- No third-party math libs in the engine itself (Eigen/Boost only as test oracles)
- Background: math + CS undergrad — skip basics, go deep on theory and performance
