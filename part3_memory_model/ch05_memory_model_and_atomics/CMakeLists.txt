add_library(ch05_practice practice.cpp)
target_link_libraries(ch05_practice PUBLIC Threads::Threads)
target_include_directories(ch05_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch05_tests test_practice.cpp)
target_link_libraries(ch05_tests PRIVATE ch05_practice GTest::gtest_main)
gtest_discover_tests(ch05_tests)
