add_library(compute_linalg STATIC
  layout.cpp
  dispatch/gpu_dispatch.cpp
  add.cpp
  sub.cpp
  scale.cpp
  hadamard.cpp
  transpose.cpp
  dot.cpp
  gemv.cpp
  gemm.cpp
  trace.cpp
  norm_fro.cpp
  norm_inf.cpp
)

target_link_libraries(compute_linalg PUBLIC compute_core)
target_include_directories(compute_linalg PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/compute>
)
