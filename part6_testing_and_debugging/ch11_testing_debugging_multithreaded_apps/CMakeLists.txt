add_library(ch11_practice practice.cpp)
target_link_libraries(ch11_practice PUBLIC Threads::Threads)
target_include_directories(ch11_practice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ch11_tests test_practice.cpp)
target_link_libraries(ch11_tests PRIVATE ch11_practice GTest::gtest_main)
gtest_discover_tests(ch11_tests)
