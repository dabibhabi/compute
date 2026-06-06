---
tags:
  - references
  - bibliography
aliases:
  - refs
  - bibliography
---

# References

> [!info] Notation
> **(free)** = legally available as PDF or web resource. Read these first.

---

## Linear Algebra — Fundamentals

- **Strang, Gilbert** — *Introduction to Linear Algebra*, 5th ed. (2016, Wellesley-Cambridge)
  Ch 1–4: vectors, matrix operations, orthogonality, four fundamental subspaces
- **(free)** **MIT OCW 18.06** — Strang's lecture videos: ocw.mit.edu/18.06
- **Lay, David C.** — *Linear Algebra and Its Applications*, 5th ed. — Ch 2.1–2.3

---

## Linear Algebra — Decompositions & Numerical

- **Trefethen, Lloyd N. & Bau, David III** — *Numerical Linear Algebra* (1997, SIAM)
  Lectures 7–11 (QR), 20–22 (LU), 24–29 (eigenvalues), 31 (SVD), 32–38 (Krylov/GMRES)
  *The essential reference for numerical linear algebra.*
- **Golub, Gene H. & Van Loan, Charles F.** — *Matrix Computations*, 4th ed. (2013, JHU Press)
  Ch 3.4 (LU), Ch 5 (Householder, Gram-Schmidt), Ch 6 (RRQR), Ch 8.6 (Golub-Reinsch SVD), Ch 10 (Arnoldi/Lanczos)
- **(free)** **Golub & Reinsch** — "SVD and Least Squares Solutions" *Numer. Math.* 1970 — the original SVD paper

---

## Iterative Methods & Optimization

- **(free)** **Saad, Yousef** — *Iterative Methods for Sparse Linear Systems*, 2nd ed. (SIAM)
  www-users.cse.umn.edu/~saad/IterMethBook_2ndEd.pdf
  Ch 6: CG, GMRES, Arnoldi, ILU(0) preconditioner
- **Nocedal, Jorge & Wright, Stephen J.** — *Numerical Optimization*, 2nd ed. (2006, Springer)
  Ch 3 (Armijo/Wolfe line search), Ch 7 (L-BFGS), Ch 16 (L-BFGS-B)

---

## Numerical Analysis

- **Burden, Faires & Burden** — *Numerical Analysis*, 10th ed. (2015, Cengage)
  Ch 2 (root-finding), Ch 3 (interpolation), Ch 4 (quadrature)
- **(free)** **Trefethen, Lloyd N.** — *Approximation Theory and Approximation Practice* (SIAM)
  people.maths.ox.ac.uk/trefethen/ATAP/ — Ch 3–4 (Chebyshev), Ch 12 (Clenshaw-Curtis vs Gaussian)
- **Brigham, E. Oran** — *The Fast Fourier Transform and Its Applications* (1988, Prentice Hall) — Ch 7
- **(free)** **Frigo & Johnson** — "The Design and Implementation of FFTW3" *IEEE* 2005: fftw.org/fftw-paper-ieee.pdf
- **(free)** **Baydin et al.** — "Automatic Differentiation in ML: a Survey" *JMLR* 2018: arxiv.org/abs/1502.05767

---

## ODE Solvers

- **Hairer, Nørsett & Wanner** — *Solving Ordinary Differential Equations I*, 2nd ed. (1993, Springer)
  Ch 1–2 (RK methods, Butcher tableaux), Ch 4 (DOPRI5, PI step controller p. 168)
  *DOPRI5 was published here. Read Ch 4 before implementing it.*
- **Hairer & Wanner** — *Solving Ordinary Differential Equations II*, 2nd ed. (1996, Springer)
  Ch 4 (SDIRK methods for stiff systems)
- **Leimkuhler & Reich** — *Simulating Hamiltonian Dynamics* (2004, Cambridge) — Ch 1–2 (Verlet, symplecticity)
- **(free)** **MIT 18.337J** — mitmath.github.io/18337 — Lectures 7–10

---

## PDE

- **LeVeque, Randall J.** — *Finite Difference Methods for ODEs and PDEs* (2007, SIAM)
  Ch 1 (truncation error), Ch 2 (Thomas algorithm), Ch 3 (5-point stencil/Poisson), Ch 9 (heat equation, CFL)
- **(free)** **Briggs, Henson & McCormick** — *A Multigrid Tutorial*, 2nd ed. (SIAM)
  cs.iit.edu/~cs524/papersbig/multigrid_tutorial.pdf — Ch 1–3
- **Strang** — *Computational Science and Engineering* (2007, Wellesley-Cambridge)
  **(free)** MIT OCW 18.085: ocw.mit.edu/18.085 — Ch 6–7

---

## Statistical Analysis

- **Casella, George & Berger, Roger L.** — *Statistical Inference*, 2nd ed. (2002, Cengage) — Ch 5, 8, 10
- **Rice, John A.** — *Mathematical Statistics and Data Analysis*, 3rd ed. (2006, Cengage) — Ch 11, 14
- **Silverman, Bernard W.** — *Density Estimation for Statistics and Data Analysis* (1986, Chapman & Hall)
  Ch 2–3 (Silverman's rule, FFT acceleration for KDE)

---

## Probability Theory and Special Functions

- **(free)** **Abramowitz & Stegun** / **NIST DLMF** — dlmf.nist.gov
  §6 (erf, erfinv polynomial approximations), §8 (incomplete gamma/beta), §22 (Legendre polynomials)
  *Every special function implementation traces back here.*
- **Robert, Christian P. & Casella, George** — *Monte Carlo Statistical Methods*, 2nd ed. (2004, Springer)
  Ch 2 (inverse CDF sampling), Ch 6 (Metropolis-Hastings), Ch 10 (variance reduction, antithetic/control variates)
- **(free)** **Durrett, Rick** — *Probability: Theory and Examples*, 5th ed.
  services.math.duke.edu/~rtd/PTE/PTE5_011119.pdf — Ch 3 (CLT), Ch 5 (Markov chains)
- **Ross, Sheldon M.** — *A First Course in Probability*, 10th ed. — Ch 5–7
- **(free)** **O'Neill, Melissa E.** — "PCG: A Family of Simple Fast PRNGs" 2014: pcg-random.org/paper.html
  *Read this before implementing the PRNG.*
- **Press et al.** — *Numerical Recipes*, 3rd ed. — Ch 6 (special functions), Ch 7 (RNG)

---

## Combinatorics

- **Graham, Knuth & Patashnik** — *Concrete Mathematics*, 2nd ed. (1994, Addison-Wesley)
  Ch 2 (sums, recurrences), Ch 5 (Stirling numbers, binomial coefficients), Ch 7 (generating functions)
- **Andrews, George E.** — *The Theory of Partitions* (1976, Cambridge) — Ch 1 (pentagonal theorem)
- **(free)** **Stanley, Richard P.** — *Enumerative Combinatorics* Vol. 1, 2nd ed.
  math.mit.edu/~rstan/ec/ec1.pdf — Ch 1.1–1.3, 1.8
- **(free)** **Held & Karp** — "A Dynamic Programming Approach to Sequencing Problems" *J. SIAM* 1962
  Search: "Held Karp 1962 dynamic programming sequencing"

---

## DSA — Data Structures and Algorithms

- **Cormen, Leiserson, Rivest & Stein (CLRS)** — *Introduction to Algorithms*, 4th ed. (2022, MIT Press)
  Ch 6 (heaps), Ch 8–9 (linear-time sorting, order statistics), Ch 15–17 (DP, greedy, amortized), Ch 21 (Union-Find), Ch 34–35 (NP, approximation)
- **Skiena, Steven S.** — *The Algorithm Design Manual*, 3rd ed. (2020, Springer) — Ch 3, 6, 8
- **Knuth, Donald E.** — *TAOCP* Vol. 1–4B — Vol. 3 §5 (sorting), §6.4 (hashing), Vol. 4A §7.2
- **(free)** **Drepper, Ulrich** — "What Every Programmer Should Know About Memory" (2007, Red Hat)
  people.freebsd.org/~lstewart/articles/cpumemory.pdf — §3, §6 — **required reading**
- **(free)** **Tarjan** — "Amortized Computational Complexity" *SIAM* 1985 — potential method
- **(free)** **Sleator & Tarjan** — "Self-Adjusting Binary Search Trees" *JACM* 1985 — splay trees
- **(free)** **Celis, Larson & Munro** — "Robin Hood Hashing" *FOCS* 1985
- **(free)** **Frigo, Leiserson, Prokop & Ramachandran** — "Cache-Oblivious Algorithms" *FOCS* 1999
- **(free)** **Tarjan** — "Efficiency of a Good But Not Linear Set Union Algorithm" *JACM* 1975 — Union-Find analysis
- **(free)** **Abseil Swiss Tables design doc** — abseil.io/about/design/swisstables

---

## Complexity Theory

- **Sipser, Michael** — *Introduction to the Theory of Computation*, 3rd ed. (2012, Cengage) — Ch 7–8
- **(free)** **Arora & Barak** — *Computational Complexity: A Modern Approach* (2009, Cambridge)
  theory.cs.princeton.edu/complexity/book.pdf — Ch 1–2, Ch 16
- **Motwani, Rajeev & Raghavan, Prabhakar** — *Randomized Algorithms* (1995, Cambridge) — Ch 1, 3, 12
- **(free)** **Aaronson, Scott** — *Quantum Computing Since Democritus* lecture notes: scottaaronson.com/democritus/

---

## HPC, SIMD and CUDA

- **Kirk, David B. & Hwu, Wen-mei W.** — *Programming Massively Parallel Processors*, 4th ed. (2022, Morgan Kaufmann)
  Ch 3–5 (thread model, shared memory, bank conflicts, occupancy), Ch 10 (scan/reduction), Ch 13 (sorting)
- **Hennessy & Patterson** — *Computer Architecture: A Quantitative Approach*, 6th ed. — Ch 2, 4
- **Patterson & Hennessy** — *Computer Organization and Design*, 6th ed. — Ch 4–5
- **(free)** **Williams, Waterman & Patterson** — "Roofline: An Insightful Visual Performance Model" *CACM* 2009
  Search: "Williams Waterman Patterson Roofline 2009" — **read before the Week 12 benchmarking sprint**
- **(free)** **Intel Intrinsics Guide** — software.intel.com/sites/landingpage/IntrinsicsGuide/
- **(free)** **Agner Fog** — Optimization Manuals: agner.org/optimize/ — instruction latencies for Zen 5
- **(free)** **Beamer, Asanović & Patterson** — "Direction-Optimizing BFS" *SC* 2012
  Search: "Beamer Asanovic Patterson direction optimizing BFS 2012"
- **(free)** **Blelloch** — "Prefix Sums and Their Applications" CMU tech report 1990

---

## General Scientific Computing

- **Strang** — *Computational Science and Engineering* (2007) + **(free)** MIT OCW 18.085
- **(free)** **Wilson et al.** — "Best Practices for Scientific Computing" *PLOS Biology* 2014
- **(free)** **Knuth** — "Literate Programming" *Computer Journal* 1984
