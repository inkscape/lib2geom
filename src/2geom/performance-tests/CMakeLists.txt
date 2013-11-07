SET(2GEOM_PERFORMANCE_TESTS_SRC
example-performance-test
bendpath
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
        add_custom_command(TARGET perf COMMAND ${source}.exe)
    ENDFOREACH(source)
ENDIF()