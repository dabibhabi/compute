#include <metal_stdlib>
using namespace metal;

// Metal Shading Language has no double type and Apple GPUs lack float64, so every
// kernel computes in float32. Storage is column-major: index(i, j) = j * rows + i.

struct ElemParams { uint n; int op; float alpha; };
struct Dims2      { uint rows; uint cols; };
struct GemmDims   { uint m; uint n; uint k; };

kernel void elementwise(device const float* a       [[buffer(0)]],
                        device const float* b       [[buffer(1)]],
                        device float*       c       [[buffer(2)]],
                        constant ElemParams& p       [[buffer(3)]],
                        uint gid                     [[thread_position_in_grid]]) {
    if (gid >= p.n) return;
    switch (p.op) {
        case 0: c[gid] = a[gid] + b[gid];   break;
        case 1: c[gid] = a[gid] - b[gid];   break;
        case 2: c[gid] = p.alpha * a[gid];  break;
        case 3: c[gid] = a[gid] * b[gid];   break;
    }
}

kernel void transpose(device const float* a   [[buffer(0)]],
                      device float*       b   [[buffer(1)]],
                      constant Dims2&     p   [[buffer(2)]],
                      uint2 gid               [[thread_position_in_grid]]) {
    uint i = gid.x;
    uint j = gid.y;
    if (i >= p.rows || j >= p.cols) return;
    b[i * p.cols + j] = a[j * p.rows + i];
}

kernel void gemm(device const float* a   [[buffer(0)]],
                 device const float* b   [[buffer(1)]],
                 device float*       c   [[buffer(2)]],
                 constant GemmDims&  p   [[buffer(3)]],
                 uint2 gid               [[thread_position_in_grid]]) {
    uint i = gid.x;
    uint j = gid.y;
    if (i >= p.m || j >= p.n) return;
    float sum = 0.0f;
    for (uint q = 0u; q < p.k; ++q)
        sum += a[q * p.m + i] * b[j * p.k + q];
    c[j * p.m + i] = sum;
}

kernel void gemv(device const float* a   [[buffer(0)]],
                 device const float* x   [[buffer(1)]],
                 device float*       y   [[buffer(2)]],
                 constant Dims2&     p   [[buffer(3)]],
                 uint gid                [[thread_position_in_grid]]) {
    uint i = gid;
    if (i >= p.rows) return;
    float sum = 0.0f;
    for (uint j = 0u; j < p.cols; ++j)
        sum += a[j * p.rows + i] * x[j];
    y[i] = sum;
}

kernel void reduce_dot(device const float* u        [[buffer(0)]],
                       device const float* v        [[buffer(1)]],
                       device float*       partials [[buffer(2)]],
                       constant uint&      n        [[buffer(3)]],
                       uint gid    [[thread_position_in_grid]],
                       uint lid    [[thread_position_in_threadgroup]],
                       uint tg     [[threadgroup_position_in_grid]]) {
    threadgroup float scratch[256];
    scratch[lid] = (gid < n) ? u[gid] * v[gid] : 0.0f;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    for (uint s = 128u; s > 0u; s >>= 1) {
        if (lid < s) scratch[lid] += scratch[lid + s];
        threadgroup_barrier(mem_flags::mem_threadgroup);
    }
    if (lid == 0u) partials[tg] = scratch[0];
}

kernel void row_abs_sum(device const float* a   [[buffer(0)]],
                        device float*       out [[buffer(1)]],
                        constant Dims2&     p   [[buffer(2)]],
                        uint gid                [[thread_position_in_grid]]) {
    uint i = gid;
    if (i >= p.rows) return;
    float sum = 0.0f;
    for (uint j = 0u; j < p.cols; ++j)
        sum += fabs(a[j * p.rows + i]);
    out[i] = sum;
}
