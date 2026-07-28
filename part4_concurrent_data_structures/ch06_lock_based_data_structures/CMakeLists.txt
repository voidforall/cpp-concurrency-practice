add_library(ch06_practice practice.cpp)
target_link_libraries(ch06_practice PUBLIC Threads::Threads)
target_include_directories(ch06_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch06_tests test_practice.cpp)
target_link_libraries(ch06_tests PRIVATE ch06_practice GTest::gtest_main)
gtest_discover_tests(ch06_tests)
