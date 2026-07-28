add_library(ch08_practice practice.cpp)
target_link_libraries(ch08_practice PUBLIC Threads::Threads)
target_include_directories(ch08_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch08_tests test_practice.cpp)
target_link_libraries(ch08_tests PRIVATE ch08_practice GTest::gtest_main)
gtest_discover_tests(ch08_tests)
