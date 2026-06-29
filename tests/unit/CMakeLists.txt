find_package(Catch2 3 REQUIRED)

find_package(Eigen3 QUIET NO_MODULE)
if(NOT Eigen3_FOUND)
  set(EIGEN_BUILD_TESTING OFF CACHE BOOL "" FORCE)
  include(FetchContent)
  FetchContent_Declare(
    Eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG        3.4.0
    GIT_SHALLOW    TRUE
  )
  FetchContent_MakeAvailable(Eigen3)
endif()

# Week 1: core tests
add_executable(test_core test_core.cpp)
target_link_libraries(test_core PRIVATE Catch2::Catch2WithMain compute_core)
add_test(NAME core COMMAND test_core)

add_executable(test_linalg test_linalg.cpp test_linalg_gpu.cpp)
target_link_libraries(test_linalg PRIVATE Catch2::Catch2WithMain compute_linalg Eigen3::Eigen)
add_test(NAME linalg COMMAND test_linalg)
