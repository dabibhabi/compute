find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)

set(VULKAN_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${VULKAN_SHADER_DIR})

set(VULKAN_GLSL_SHADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/elementwise.comp
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/gemm.comp
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/transpose.comp
)

foreach(shader ${VULKAN_GLSL_SHADERS})
  get_filename_component(name ${shader} NAME_WE)
  set(spv ${VULKAN_SHADER_DIR}/${name}.spv)
  add_custom_command(
    OUTPUT ${spv}
    COMMAND ${GLSLANG_VALIDATOR} -V ${shader} -o ${spv}
    DEPENDS ${shader}
    COMMENT "Compiling Vulkan shader ${name}"
  )
  list(APPEND VULKAN_SPV_OUTPUTS ${spv})
endforeach()

add_custom_target(compute_vulkan_shaders DEPENDS ${VULKAN_SPV_OUTPUTS})

add_library(compute_vulkan STATIC stub.cpp)
add_dependencies(compute_vulkan compute_vulkan_shaders)

target_link_libraries(compute_vulkan PUBLIC compute_core Vulkan::Vulkan)
target_include_directories(compute_vulkan PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/compute>
)
target_compile_definitions(compute_vulkan PUBLIC
  COMPUTE_HAS_VULKAN
  COMPUTE_VULKAN_SHADER_DIR="${VULKAN_SHADER_DIR}"
)
