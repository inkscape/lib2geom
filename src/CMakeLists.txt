ADD_SUBDIRECTORY (2geom)
ADD_SUBDIRECTORY (tests)
ADD_SUBDIRECTORY (performance-tests)
ADD_SUBDIRECTORY (toys)

ADD_SUBDIRECTORY (cython)
ADD_SUBDIRECTORY (py2geom)

ADD_LIBRARY(gtest SHARED googletest/googletest/src/gtest-all.cc)
ADD_LIBRARY(gtest_main STATIC googletest/googletest/src/gtest_main.cc)
TARGET_INCLUDE_DIRECTORIES(gtest PRIVATE googletest/googletest)
