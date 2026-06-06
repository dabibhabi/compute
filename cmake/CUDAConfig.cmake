# CUDA configuration — loaded only when COMPUTE_ENABLE_CUDA=ON (Week 4+)

# Target SM 86 (RTX 3060, Ampere). Skip PTX in release for smaller binary / no JIT.
set(CMAKE_CUDA_ARCHITECTURES
  $<IF:$<CONFIG:Release>,86-real,86>
)

# Separate compilation required for device symbol linking across .cu files.
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)

# Pass host compiler flags to nvcc where compatible.
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler=-march=native")

find_package(CUDAToolkit REQUIRED)
