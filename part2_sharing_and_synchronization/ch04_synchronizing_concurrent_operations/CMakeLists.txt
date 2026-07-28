add_library(ch04_practice practice.cpp)
target_link_libraries(ch04_practice PUBLIC Threads::Threads)
target_include_directories(ch04_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch04_tests test_practice.cpp)
target_link_libraries(ch04_tests PRIVATE ch04_practice GTest::gtest_main)
gtest_discover_tests(ch04_tests)
