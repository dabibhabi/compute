"""Compute Engine — Python package."""

from _compute_core import ComputePolicy, get_policy, set_policy, version
from _compute_linalg import (
    add,
    dot,
    gemm,
    gemv,
    hadamard,
    norm_fro,
    norm_inf,
    scale,
    sub,
    trace,
    transpose,
)

__version__ = version()

__all__ = [
    "ComputePolicy",
    "get_policy",
    "set_policy",
    "version",
    "add",
    "sub",
    "scale",
    "hadamard",
    "transpose",
    "dot",
    "gemv",
    "gemm",
    "trace",
    "norm_fro",
    "norm_inf",
]
