# nanobind extension helpers — loaded when COMPUTE_ENABLE_PYTHON=ON

find_package(Python 3.12 REQUIRED COMPONENTS Interpreter Development.SABIModule)

# Convenience function: add a nanobind extension module with standard settings.
# Usage: compute_python_module(NAME _compute_linalg SOURCES linalg_bind.cpp LINK compute_linalg)
function(compute_python_module)
  cmake_parse_arguments(ARG "" "NAME" "SOURCES;LINK" ${ARGN})

  nanobind_add_module(${ARG_NAME}
    STABLE_ABI NB_STATIC
    ${ARG_SOURCES}
  )
  target_link_libraries(${ARG_NAME} PRIVATE ${ARG_LINK} compute_core)
  target_include_directories(${ARG_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
  )

  install(TARGETS ${ARG_NAME}
    LIBRARY DESTINATION compute
  )
endfunction()
