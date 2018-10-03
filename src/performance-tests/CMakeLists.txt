SET(2GEOM_PERFORMANCE_TESTS_SRC
example-performance-test
boolops-performance-test
bendpath-test
bezier-utils-test
parse-svg-test
path-operations-test
)

add_custom_target(perf)

OPTION(2GEOM_PERFORMANCE_TESTS
  "Build the performance tests"
  ON)
IF(2GEOM_PERFORMANCE_TESTS)
    FOREACH(source ${2GEOM_PERFORMANCE_TESTS_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} ${LINK_GSL} 2geom)
        add_dependencies(perf ${source})
        add_custom_command(TARGET perf COMMAND ${source})
    ENDFOREACH(source)
ENDIF()

IF(WIN32 AND 2GEOM_BUILD_SHARED)
    ADD_CUSTOM_TARGET(copy-perf ALL COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/src/2geom/lib2geom.dll ${CMAKE_BINARY_DIR}/src/performance-tests/lib2geom.dll)
    ADD_DEPENDENCIES(copy-perf 2geom)
ENDIF()
