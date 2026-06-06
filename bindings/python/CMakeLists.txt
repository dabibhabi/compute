include(${CMAKE_SOURCE_DIR}/cmake/PythonBindings.cmake)

# Root module: exposes compute.version() and imports sub-modules
compute_python_module(
  NAME    _compute_core
  SOURCES core_bind.cpp
  LINK    compute_core
)

