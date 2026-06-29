#include <compute/core/compute_policy.hpp>

namespace compute::core {

thread_local ComputePolicy g_policy = ComputePolicy::automatic;

} // namespace compute::core
