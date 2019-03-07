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

foreach(source ${2GEOM_GTESTS_SRC})
	add_executable(${source} ${source}.cpp)
	target_include_directories(${source} PRIVATE ${GSL_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
	target_link_libraries(${source} 2geom gtest gtest_main ${GSL_LIBRARIES} ${GTK3_LIBRARIES})
	add_test(NAME ${source} COMMAND ${source})
endforeach()

foreach(source ${2GEOM_TESTS_SRC})
    add_executable(${source} ${source}.cpp)
    target_include_directories(${source} PRIVATE ${GSL_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
    target_link_libraries(${source} 2geom gtest ${GSL_LIBRARIES} ${GTK3_LIBRARIES})
    add_test(NAME ${source} COMMAND ${source})
endforeach(source)

if(WIN32 AND 2GEOM_BUILD_SHARED)
    add_custom_target(copy ALL COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/src/2geom/lib2geom.dll ${CMAKE_BINARY_DIR}/src/performance-tests/lib2geom.dll)
    add_dependencies(copy 2geom)
endif()
