SET(2GEOM_TESTS_SRC
#bezier-utils-test
bezier-test
choose-test
#lin_alg_test
matrix-inv
#point-fns-test
poly-test
#rotate-test
#rotate-test.h
#scale-test
#scale-test.h
#rotate-fns-test
#rotate-fns-test.h
test-cg
#translate-test
#translate-test.h
#types-test
#types-test.h
unit-test-sbasis
root-find-ut
implicitization
time-ops
)

OPTION(2GEOM_UNIT_TESTS
  "Build the projects Unit Tests"
  OFF)
IF(2GEOM_UNIT_TESTS)
  # temporary fix. use #include <2geom/bezier.h> instead of #include "bezier.h"
  INCLUDE_DIRECTORIES( "../" )


    FOREACH(source ${2GEOM_TESTS_SRC})
        IF(${source} STREQUAL time-ops)
            ADD_EXECUTABLE(${source} EXCLUDE_FROM_ALL ${source}.cpp)
            TARGET_LINK_LIBRARIES(${source} rt)
        ELSE()
            ADD_EXECUTABLE(${source} ${source}.cpp)
        ENDIF(${source} STREQUAL time-ops)
        TARGET_LINK_LIBRARIES(${source} ${LINK_GSL} 2geom ${GTK2_LINK_FLAGS})
    ENDFOREACH(source)
ENDIF(2GEOM_UNIT_TESTS)
