add_library(gtest SHARED googletest/googletest/src/gtest-all.cc)
target_include_directories(gtest PRIVATE googletest/googletest)
target_link_libraries(gtest Threads::Threads)
add_library(gtest_main STATIC googletest/googletest/src/gtest_main.cc)

add_subdirectory(2geom)
add_subdirectory(tests)
add_subdirectory(performance-tests)
add_subdirectory(toys)

add_subdirectory(cython)
add_subdirectory(py2geom)
