cmake_minimum_required(VERSION 3.20)
project(cpp_concurrency_practice CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

option(ENABLE_TSAN "Build with ThreadSanitizer" OFF)
if(ENABLE_TSAN)
  add_compile_options(-fsanitize=thread -g -O1)
  add_link_options(-fsanitize=thread)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

add_subdirectory(part1_foundations/ch01_hello_world_of_concurrency)
add_subdirectory(part1_foundations/ch02_managing_threads)
add_subdirectory(part2_sharing_and_synchronization/ch03_sharing_data_between_threads)
add_subdirectory(part2_sharing_and_synchronization/ch04_synchronizing_concurrent_operations)
add_subdirectory(part3_memory_model/ch05_memory_model_and_atomics)
add_subdirectory(part4_concurrent_data_structures/ch06_lock_based_data_structures)
add_subdirectory(part4_concurrent_data_structures/ch07_lock_free_data_structures)
add_subdirectory(part4_concurrent_data_structures/ch08_designing_concurrent_code)
add_subdirectory(part5_advanced_topics/ch09_advanced_thread_management)
add_subdirectory(part5_advanced_topics/ch10_parallel_algorithms)
add_subdirectory(part6_testing_and_debugging/ch11_testing_debugging_multithreaded_apps)
