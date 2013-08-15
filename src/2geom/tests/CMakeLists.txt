# Use this variable for tests which provide their own main().
SET(2GEOM_TESTS_SRC
#bezier-utils-test
choose-test
convex-test
#lin_alg_test
matrix-inv-test
poly-test
#rotate-test
#rotate-test.h
#scale-test
#scale-test.h
#rotate-fns-test
#rotate-fns-test.h
cg-test
#svgd-test
#translate-test
#translate-test.h
types-test
#types-test.h
sbasis-text-test
root-find-test
implicitization-test
#timing-test
#rtree-performance-test
)

# Use this variable for GTest tests which should have a default main().
SET(2GEOM_GTESTS_SRC
affine-test
bezier-test
chain-test
path-test
#point-test
rect-test
sbasis-test
)

OPTION(2GEOM_IDX_LIBSPATIALINDEX
  "Build lib2geom with libSpatialIndex indexing support."
  OFF)

FOREACH(source ${2GEOM_GTESTS_SRC})
	ADD_EXECUTABLE(${source} ${source}.cpp)
	TARGET_LINK_LIBRARIES(${source} 2geom gtest_main)
	ADD_TEST(NAME ${source} COMMAND ${source})
ENDFOREACH()


FOREACH(source ${2GEOM_TESTS_SRC})
    IF(${source} STREQUAL timing-test)
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} rt)
        TARGET_LINK_LIBRARIES(${source} ${LINK_GSL} 2geom ${GTK2_LINK_FLAGS})
        ADD_TEST(NAME ${source} COMMAND ${source})
    ELSEIF(${source} STREQUAL rtree-performance-test)
        IF(2GEOM_IDX_LIBSPATIALINDEX AND 2GEOM_TOYS)
            # we use the lib from a tar and not a package, because there's a gap 
            # debian package (sid unstable) has 1.4.0 whereas the release version is 1.5.0
            INCLUDE_DIRECTORIES(/home/vag/projects/inkscape/spatialindex-lib/include/spatialindex)
            LINK_DIRECTORIES(/home/vag/projects/inkscape/spatialindex-lib/lib)

            ADD_EXECUTABLE(${source} ${source}.cpp)
            TARGET_LINK_LIBRARIES(${source} ${LINK_GSL} spatialindex 2geom toy-2 ${GTK2_LINK_FLAGS})
            ADD_TEST(NAME ${source} COMMAND ${source})
        ENDIF(${source} STREQUAL time-ops)
    ELSE()
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} ${LINK_GSL} 2geom ${GTK2_LINK_FLAGS} gtest)
        ADD_TEST(NAME ${source} COMMAND ${source})
    ENDIF()
ENDFOREACH(source)
