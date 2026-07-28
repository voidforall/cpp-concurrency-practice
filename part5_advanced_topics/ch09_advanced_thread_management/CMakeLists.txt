add_library(ch09_practice practice.cpp)
target_link_libraries(ch09_practice PUBLIC Threads::Threads)
target_include_directories(ch09_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch09_tests test_practice.cpp)
target_link_libraries(ch09_tests PRIVATE ch09_practice GTest::gtest_main)
gtest_discover_tests(ch09_tests)
