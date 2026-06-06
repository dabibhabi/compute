find_package(Catch2 3 REQUIRED)

# Week 1: core tests
add_executable(test_core test_core.cpp)
target_link_libraries(test_core PRIVATE Catch2::Catch2WithMain compute_core)
add_test(NAME core COMMAND test_core)

# Module tests added per week:
# add_executable(test_linalg test_linalg.cpp)
# target_link_libraries(test_linalg PRIVATE Catch2::Catch2WithMain compute_linalg)
# add_test(NAME linalg COMMAND test_linalg)
