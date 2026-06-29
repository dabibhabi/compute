add_library(compute_cuda STATIC
  detail/context.cpp
  linalg/linalg_dispatch.cpp
  linalg/kernels.cu
)

target_link_libraries(compute_cuda PUBLIC compute_core CUDA::cudart CUDA::cublas)
target_include_directories(compute_cuda PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_compile_definitions(compute_cuda PUBLIC COMPUTE_HAS_CUDA)
set_target_properties(compute_cuda PROPERTIES
  CUDA_SEPARABLE_COMPILATION ON
  POSITION_INDEPENDENT_CODE ON
)
set_source_files_properties(detail/context.cpp linalg/linalg_dispatch.cpp PROPERTIES
  COMPILE_FLAGS "-std=c++23"
)
