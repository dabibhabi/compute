add_library(compute_metal STATIC stub.cpp)

target_link_libraries(compute_metal PUBLIC compute_core)
target_include_directories(compute_metal PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/compute>
)
target_compile_definitions(compute_metal PUBLIC COMPUTE_HAS_METAL)
