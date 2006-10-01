PROJECT(lib2geom CXX)
SET(CMAKE_BACKWARDS_COMPATIBILITY 2.4)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts)
INCLUDE(Config2GeomDepends)

IF(gsl_FOUND)
# must have GSL
    SET(NEEDS_GSL 
    differential-constraint
    DC-conic-4
    )
    SET(LINK_GSL ${gsl_LINK_FLAGS})
ENDIF(gsl_FOUND) 

# Set Compiler Flags
SET(CMAKE_CXX_FLAGS "${GTK2_CFLAGS} ${cairo_CFLAGS} ${gsl_CFLAGS}")
SET(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j2")

SET(2GEOM_TOYS_SRC 
2dsb2d
arc-bez
clothoid
conic-2
conic-3
conic-4
conic
convex-test
fitter
one-D
poly-test
quad-test
rat-bez
root-finder-comparer
sb1d
sb2d
s-bez
#toy-cairo
unit-test-sbasis
gear
cheb
match-curve
"${NEEDS_GSL}"
)

SET(2GEOM_TOY-FRAMEWORK_SRC
interactive-bits.cpp
interactive-bits.h
toy-framework.cpp
toy-framework.h
path-cairo.cpp
path-cairo.h
)

SET(2GEOM_TESTS_SRC
bezier-utils-test
bez-test
choose_test
matrix-test
#matrix-test.h
point-fns-test
#point-fns-test.h
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
)
SET(2GEOM_SRC
arc-length.cpp
arc-length.h
bezier-to-sbasis.h
beztype.cpp
bezier-utils.cpp
bezier-utils.h
#b-spline.cpp
centroid.cpp
centroid.h
choose.h
circulator.h
conjugate_gradient.cpp
conjugate_gradient.h
convex-cover.cpp
convex-cover.h
convex-hull.h
convex-hull-ops.h
coord.h
cubic_bez_util.cpp
cubic_bez_util.h
decimal-round.h
dim2.h
floating.h
forward.h
geom.cpp
geom.h
i-coord.h
isnan.h
#libnr.def
macros.h
matrix.cpp
matrix-div.cpp
matrix-div.h
matrix-fns.cpp
matrix-fns.h
matrix.h
matrix-ops.h
matrix-rotate-ops.cpp
matrix-rotate-ops.h
matrix-scale-ops.cpp
matrix-scale-ops.h
matrix-translate-ops.cpp
matrix-translate-ops.h
maybe.h
multidim-sbasis.h
#nearestpoint.cpp
path-builder.h
#path-cubicto.cpp
path-find-points-of-interest.cpp
path-find-points-of-interest.h
path-nearest.cpp
path-nearest.h
path-intersect.cpp
path-intersect.h
path-metric.cpp
path-metric.h
path-ops.cpp
path-ops.h
path-poly-fns.h
path-sbasis.h
path-to-polyline.cpp
path-to-polyline.h
path.cpp
path.h
point-fns.cpp
point-fns.h
point.h
point-l.h
point-matrix-ops.h
point-ops.h
poly.cpp
poly-dk-solve.cpp
poly-dk-solve.h
poly.h
poly-laguerre-solve.cpp
poly-laguerre-solve.h
quadtree.cpp
quadtree.h
read-svgd.cpp
read-svgd.h
rect.cpp
rect.h
rect-ops.h
refcounted.h
rotate-fns.cpp
rotate-fns.h
rotate.h
rotate-matrix-ops.cpp
rotate-matrix-ops.h
rotate-ops.h
round.h
s-basis-2d.h
s-basis-2d.cpp
s-basis.cpp
s-basis.h
s-basis-roots.cpp
sbasis-poly.cpp
sbasis-poly.h
sbasis-to-bezier.cpp
sbasis-to-bezier.h
scale.h
scale-matrix-ops.cpp
scale-matrix-ops.h
scale-ops.h
scale-translate-ops.cpp
scale-translate-ops.h
solve-bezier-one-d.cpp
solve-bezier-parametric.cpp
solver.h
sturm.h
#toy.cpp
translate.h
translate-matrix-ops.cpp
translate-matrix-ops.h
translate-ops.h
translate-rotate-ops.cpp
translate-rotate-ops.h
translate-scale-ops.cpp
translate-scale-ops.h
types.cpp
types.h
)

# make lib for 2geom
ADD_LIBRARY(2geom STATIC ${2GEOM_SRC})
#TARGET_LINK_LIBRARIES(2geom blas gsl)

# make lib for toy
ADD_LIBRARY(toy STATIC ${2GEOM_TOY-FRAMEWORK_SRC})
TARGET_LINK_LIBRARIES(toy 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS}")

OPTION(2GEOM_TOYS
  "Build the projects Toy files"
  ON)
IF(2GEOM_TOYS)
    FOREACH(source ${2GEOM_TOYS_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} toy 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS} ${LINK_GSL}" )
    ENDFOREACH(source)
ENDIF(2GEOM_TOYS)

OPTION(2GEOM_UNIT_TESTS
  "Build the projects Unit Tests"
  OFF)
IF(2GEOM_UNIT_TESTS)
    FOREACH(source ${2GEOM_TESTS_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} 2geom "${LINK_GSL}")
    ENDFOREACH(source)
ENDIF(2GEOM_UNIT_TESTS)
