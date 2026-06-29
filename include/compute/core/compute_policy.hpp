#pragma once

namespace compute::core {

// Controls CPU vs GPU dispatch for modules that support both paths.
// Each algorithm checks the thread-local policy and routes accordingly.
// Default is AUTO: GPU used only if available and problem size is above threshold.
enum class ComputePolicy {
    cpu_only,       // always use CPU path
    gpu_preferred,  // always attempt GPU path (error if CUDA not compiled in)
    automatic,      // engine decides based on problem size (default)
};

// Thread-local policy — change per-thread without locks.
extern thread_local ComputePolicy g_policy;

inline void set_policy(ComputePolicy p) noexcept { g_policy = p; }
inline ComputePolicy get_policy()        noexcept { return g_policy; }

} // namespace compute::core
