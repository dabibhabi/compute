#!/usr/bin/env python3
"""Smoke test: linalg via nanobind with optional GPU dispatch."""

import numpy as np
from compute import ComputePolicy, add, gemm, set_policy


def main() -> None:
    set_policy(ComputePolicy.automatic)

    m, n, k = 512, 512, 512
    a = np.arange(m * k, dtype=np.float64).reshape(m, k, order="F")
    b = np.arange(k * n, dtype=np.float64).reshape(k, n, order="F")
    c = np.empty((m, n), order="F")

    gemm(a, b, c)
    print("automatic gemm — C[0,0] =", c[0, 0])

    x = np.arange(m * n, dtype=np.float64).reshape(m, n, order="F")
    y = np.arange(m * n, dtype=np.float64).reshape(m, n, order="F")
    z = np.empty((m, n), order="F")
    add(x, y, z)
    print("automatic add  — Z[0,0] =", z[0, 0])

    set_policy(ComputePolicy.gpu_preferred)
    c_gpu = np.empty((m, n), order="F")
    gemm(a, b, c_gpu)
    print("gpu_preferred gemm — max |diff| =", np.max(np.abs(c - c_gpu)))


if __name__ == "__main__":
    main()
