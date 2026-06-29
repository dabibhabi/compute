if(APPLE)
  # Embed kernels.metal into a generated header (compiled at runtime via
  # newLibraryWithSource:). This keeps the shaders in a real, readable .metal file
  # without requiring the separate Metal Toolchain needed for .metallib builds.
  set(_metal_kernels ${CMAKE_CURRENT_SOURCE_DIR}/linalg/kernels.metal)
  file(READ ${_metal_kernels} COMPUTE_METAL_KERNEL_SRC)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_metal_kernels})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/detail/kernels_source.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/metal_kernels_source.h
    @ONLY
  )

  add_library(compute_metal STATIC
    detail/context.mm
    linalg/linalg_dispatch.mm
  )
  set_source_files_properties(
    detail/context.mm
    linalg/linalg_dispatch.mm
    PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
  )
  target_include_directories(compute_metal PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
  target_link_libraries(compute_metal PUBLIC compute_core
    "-framework Foundation"
    "-framework Metal"
  )
else()
  add_library(compute_metal STATIC stub.cpp)
  target_link_libraries(compute_metal PUBLIC compute_core)
endif()

target_include_directories(compute_metal PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/compute>
)
target_compile_definitions(compute_metal PUBLIC COMPUTE_HAS_METAL)
set_target_properties(compute_metal PROPERTIES POSITION_INDEPENDENT_CODE ON)
