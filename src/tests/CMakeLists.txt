SET(2GEOM_TESTS_SRC
#bezier-utils-test
bez-test
choose-test
#matrix-test
#point-fns-test
#poly-test
rotate-test
#rotate-test.h
scale-test
#scale-test.h
rotate-fns-test
#rotate-fns-test.h
test-cg
translate-test
#translate-test.h
types-test
#types-test.h
unit-test-sbasis
root-find-ut
)

OPTION(2GEOM_UNIT_TESTS
  "Build the projects Unit Tests"
  OFF)
IF(2GEOM_UNIT_TESTS)
    FOREACH(source ${2GEOM_TESTS_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} 2geom "${LINK_GSL} ${GTK2_LINK_FLAGS}")
    ENDFOREACH(source)
ENDIF(2GEOM_UNIT_TESTS)
