SET(2GEOM_PERFORMANCE_TESTS_SRC
skeleton-performance-test
)


OPTION(2GEOM_PERFORMANCE_TESTS
  "Build the performance tests"
  ON)
IF(2GEOM_PERFORMANCE_TESTS)
    FOREACH(source ${2GEOM_PERFORMANCE_TESTS_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} ${LINK_GSL} 2geom)
        ADD_TEST(NAME ${source} COMMAND ${source})
    ENDFOREACH(source)
ENDIF()