#include <nanobind/nanobind.h>
#include <nanobind/stl/string_view.h>
#include <compute/core/version.hpp>
#include <compute/core/compute_policy.hpp>

namespace nb = nanobind;
using namespace compute::core;

NB_MODULE(_compute_core, m) {
    m.doc() = "compute engine — core module";

    m.def("version", &compute::version, "Library version string");

    nb::enum_<ComputePolicy>(m, "ComputePolicy")
        .value("cpu_only",       ComputePolicy::cpu_only)
        .value("gpu_preferred",  ComputePolicy::gpu_preferred)
        .value("automatic",      ComputePolicy::automatic);

    m.def("set_policy", &set_policy, nb::arg("policy"),
          "Set the thread-local compute dispatch policy");
    m.def("get_policy", &get_policy,
          "Get the current thread-local compute dispatch policy");
}
