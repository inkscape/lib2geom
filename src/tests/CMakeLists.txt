SET(2GEOM_TESTS_SRC
bezier-utils-test
bez-test
choose_test
matrix-test
#matrix-test.h
point-fns-test
#point-fns-test.h
poly-test
rotate-test
#rotate-test.h
scale-test
#scale-test.h
rotate-fns-test
#rotate-fns-test.h
test_cg
translate-test
#translate-test.h
types-test
#types-test.h
unit-test-sbasis
)

OPTION(2GEOM_UNIT_TESTS
  "Build the projects Unit Tests"
  OFF)
IF(2GEOM_UNIT_TESTS)
    FOREACH(source ${2GEOM_TESTS_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} 2geom "${LINK_GSL}")
    ENDFOREACH(source)
ENDIF(2GEOM_UNIT_TESTS)
