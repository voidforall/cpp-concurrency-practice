add_library(ch02_practice practice.cpp)
target_link_libraries(ch02_practice PUBLIC Threads::Threads)
target_include_directories(ch02_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch02_tests test_practice.cpp)
target_link_libraries(ch02_tests PRIVATE ch02_practice GTest::gtest_main)
gtest_discover_tests(ch02_tests)
