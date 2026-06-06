# Module libraries — each is a static lib that links compute_core.
# Week 1: only compute_core (header-only). Source-compiled modules added per week.

# Placeholder targets so the rest of the build system has something to link.
# Each becomes a real compiled library as its week arrives.

foreach(module linalg numerical ode stats probability combinatorics)
  add_library(compute_${module} INTERFACE)
  target_link_libraries(compute_${module} INTERFACE compute_core)
  target_include_directories(compute_${module} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  )
endforeach()

# CUDA modules (enabled Week 4+)
if(COMPUTE_ENABLE_CUDA)
  add_subdirectory(cuda)
endif()
