# Module libraries — each is a static lib that links compute_core.

add_subdirectory(linalg)

if(COMPUTE_ENABLE_CUDA)
  add_subdirectory(cuda)
  target_link_libraries(compute_linalg PRIVATE compute_cuda)
  target_compile_definitions(compute_linalg PUBLIC COMPUTE_HAS_CUDA)
endif()

if(COMPUTE_ENABLE_METAL)
  add_subdirectory(metal)
  target_link_libraries(compute_linalg PRIVATE compute_metal)
  target_compile_definitions(compute_linalg PUBLIC COMPUTE_HAS_METAL)
endif()

if(COMPUTE_ENABLE_VULKAN)
  add_subdirectory(vulkan)
  target_link_libraries(compute_linalg PRIVATE compute_vulkan)
  target_compile_definitions(compute_linalg PUBLIC COMPUTE_HAS_VULKAN)
endif()

foreach(module numerical ode stats probability combinatorics)
  add_library(compute_${module} INTERFACE)
  target_link_libraries(compute_${module} INTERFACE compute_core)
  target_include_directories(compute_${module} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  )
endforeach()
