cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)              # Compile as C++17.
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Require C++17 support.

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

project(cpp_concurrency)

# Includes header
include_directories(./include)

# Include lib
link_directories(./lib)

file(GLOB_RECURSE TEST_SOURCES "./test/*test.cpp")

# #########################################
# "make XYZ_test"
# #########################################
foreach (test_source ${TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(test_filename ${test_source} NAME)
    string(REPLACE ".cpp" "" test_name ${test_filename})

    # Add the test target separately and as part of "make check-tests".
    add_executable(${test_name} EXCLUDE_FROM_ALL ${test_source})
#    add_dependencies(build-tests ${bustub_test_name})
#    add_dependencies(check-tests ${bustub_test_name})

#    gtest_discover_tests(${bustub_test_name}
#            EXTRA_ARGS
#            --gtest_color=auto
#            --gtest_output=xml:${CMAKE_BINARY_DIR}/test/${bustub_test_name}.xml
#            --gtest_catch_exceptions=0
#            DISCOVERY_TIMEOUT 120
#            PROPERTIES
#            TIMEOUT 120
#            )

    target_link_libraries(${test_name} libgtest.a libgtest_main.a pthread)

    # Set test target properties and dependencies.
#    set_target_properties(${bustub_test_name}
#            PROPERTIES
#            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
#            COMMAND ${bustub_test_name}
#            )
endforeach ()

add_executable(thread_test thread_test.cpp)
target_link_libraries(thread_test pthread)

add_executable(threadpool_test include/utils/threadpool_test.cpp)
target_link_libraries(threadpool_test libgtest.a libgtest_main.a pthread)

add_executable(main include/algorithm/main.cpp)
target_link_libraries(main libgtest.a libgtest_main.a pthread)

add_executable(rwlock include/lock/rwlock.cpp)
