add_library(ch03_practice practice.cpp)
target_link_libraries(ch03_practice PUBLIC Threads::Threads)
target_include_directories(ch03_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch03_tests test_practice.cpp)
target_link_libraries(ch03_tests PRIVATE ch03_practice GTest::gtest_main)
gtest_discover_tests(ch03_tests)
