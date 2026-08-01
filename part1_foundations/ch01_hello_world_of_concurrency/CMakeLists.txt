add_library(ch01_practice practice.cpp)
target_link_libraries(ch01_practice PUBLIC Threads::Threads)
target_include_directories(ch01_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch01_tests test_practice.cpp)
target_link_libraries(ch01_tests PRIVATE ch01_practice GTest::gtest_main)
gtest_discover_tests(ch01_tests)

add_executable(ch01_demo demo_main.cpp)
target_link_libraries(ch01_demo PRIVATE ch01_practice)
