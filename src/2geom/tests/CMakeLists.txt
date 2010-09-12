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
rtree-performance
)

OPTION(2GEOM_IDX_LIBSPATIALINDEX
  "Build lib2geom with libSpatialIndex indexing support."
  OFF)

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
	ELSEIF(${source} STREQUAL rtree-performance)
	  IF(2GEOM_IDX_LIBSPATIALINDEX AND 2GEOM_TOYS)
	    # we use the lib from a tar and not a package, because there's a gap 
	    # debian package (sid unstable) has 1.4.0 whereas the release version is 1.5.0
	    INCLUDE_DIRECTORIES(/home/vag/projects/inkscape/spatialindex-lib/include/spatialindex)
	    LINK_DIRECTORIES(/home/vag/projects/inkscape/spatialindex-lib/lib)
	    	    
	    ADD_EXECUTABLE(${source} ${source}.cpp)
            TARGET_LINK_LIBRARIES(${source} spatialindex 2geom toy-2 ${GTK2_LINK_FLAGS})
	  ENDIF(2GEOM_IDX_LIBSPATIALINDEX AND 2GEOM_TOYS)
        ELSE()
            ADD_EXECUTABLE(${source} ${source}.cpp)
        ENDIF(${source} STREQUAL time-ops)

        TARGET_LINK_LIBRARIES(${source} ${LINK_GSL} 2geom ${GTK2_LINK_FLAGS})
    ENDFOREACH(source)
ENDIF(2GEOM_UNIT_TESTS)
