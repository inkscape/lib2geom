# Use this variable for tests which provide their own main().
SET(2GEOM_TESTS_SRC
#bezier-utils-test
#lin_alg_test
sbasis-text-test
root-find-test
implicitization-test
#timing-test
#rtree-performance-test
)

# Use this variable for GTest tests which should have a default main().
SET(2GEOM_GTESTS_SRC
affine-test
angle-test
bezier-test
choose-test
circle-test
convex-hull-test
coord-test
ellipse-test
elliptical-arc-test
intersection-graph-test
line-test
nl-vector-test
path-test
point-test
polynomial-test
rect-test
sbasis-test
)

FOREACH(source ${2GEOM_GTESTS_SRC})
	ADD_EXECUTABLE(${source} ${source}.cpp)
	TARGET_LINK_LIBRARIES(${source} 2geom gtest_main gtest)
	ADD_TEST(NAME ${source} COMMAND ${source})
ENDFOREACH()

FOREACH(source ${2GEOM_TESTS_SRC})
    ADD_EXECUTABLE(${source} ${source}.cpp)
    TARGET_LINK_LIBRARIES(${source} 2geom ${LINK_GSL} ${GTK2_LINK_FLAGS} gtest)
    ADD_TEST(NAME ${source} COMMAND ${source})
ENDFOREACH(source)

IF(WIN32 AND 2GEOM_BUILD_SHARED)
    ADD_CUSTOM_TARGET(copy ALL COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/src/2geom/lib2geom.dll ${CMAKE_BINARY_DIR}/src/performance-tests/lib2geom.dll)
    ADD_DEPENDENCIES(copy 2geom)
ENDIF()
