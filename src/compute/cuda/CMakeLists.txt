# CUDA module — compiled only when COMPUTE_ENABLE_CUDA=ON
# Populated starting Week 4.

add_library(compute_cuda STATIC)
target_link_libraries(compute_cuda PUBLIC compute_core CUDA::cublas CUDA::cusparse CUDA::curand)
target_include_directories(compute_cuda PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)
set_target_properties(compute_cuda PROPERTIES
  CUDA_SEPARABLE_COMPILATION ON
)

# Source files added per week:
# Week 4: linalg/gemm_dispatch.cu
