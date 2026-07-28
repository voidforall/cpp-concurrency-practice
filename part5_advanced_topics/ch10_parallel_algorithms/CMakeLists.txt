add_library(ch10_practice practice.cpp)
target_link_libraries(ch10_practice PUBLIC Threads::Threads)
target_include_directories(ch10_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch10_tests test_practice.cpp)
target_link_libraries(ch10_tests PRIVATE ch10_practice GTest::gtest_main)
gtest_discover_tests(ch10_tests)
