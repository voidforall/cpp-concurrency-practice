add_library(ch07_practice practice.cpp)
target_link_libraries(ch07_practice PUBLIC Threads::Threads)
target_include_directories(ch07_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch07_tests test_practice.cpp)
target_link_libraries(ch07_tests PRIVATE ch07_practice GTest::gtest_main)
gtest_discover_tests(ch07_tests)
