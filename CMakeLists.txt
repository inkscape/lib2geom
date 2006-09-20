PROJECT(lib2geom CXX)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts)

# Dependencies Packages
IF(UNIX)
    IF(APPLE)
    ELSE(APPLE)
        include(UsePkgConfig)   
        PKGCONFIG(gtk+-2.0 GTK2_INCLUDE_DIR GTK2_LINK_DIR GTK2_LINK_FLAGS GTK2_CFLAGS)
        message(STATUS "GTK2 Configuration")
        message(STATUS "inc: -${GTK2_INCLUDE_DIR}")
        message(STATUS "link flags: -${GTK2_LINK_FLAGS}")
        message(STATUS "cflags: -${GTK2_CFLAGS}")
        message(STATUS "link dir: -${GTK2_LINK_DIR}")

        PKGCONFIG(cairo CAIRO_INCLUDE_DIR CAIRO_LINK_DIR CAIRO_LINK_FLAGS CAIRO_CFLAGS)
        message(STATUS "CAIRO Configuration")
        message(STATUS "inc: -${CAIRO_INCLUDE_DIR}")
        message(STATUS "link flags: -${CAIRO_LINK_FLAGS}")
        message(STATUS "cflags: -${CAIRO_CFLAGS}")
        message(STATUS "link dir: -${CAIRO_LINK_DIR}")
    ENDIF(APPLE)
ELSE(UNIX)
    IF(WIN32)
        SET(GTK2_INCLUDE_DIR c:/gtk28/include)
        SET(GTK2_LINK_FLAGS "-lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lpangowin32-1.0 -lgdi32 -lpangocairo-1.0 -lfontconfig -lfreetype -lpango-1.0 -lm -lcairo -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lintl -liconv -lgsl -lgslcblas")
        SET(GTK2_CFLAGS "-Ic:/gtk28/include/gtk-2.0 -Ic:/gtk28/lib/gtk-2.0/include -Ic:/gtk28/include/atk-1.0 -Ic:/gtk28/include/cairo -Ic:/gtk28/include/pango-1.0 -Ic:/gtk28/include/glib-2.0 -Ic:/gtk28/lib/glib-2.0/include -Ic:/gtk28/include -Lc:/gtk28/lib")
        SET(GTK2_LINK_DIR c:/gtk28/lib)

        message(STATUS "GTK2 Configuration")
        message(STATUS "inc: -${GTK2_INCLUDE_DIR}")
        message(STATUS "link flags: -${GTK2_LINK_FLAGS}")
        message(STATUS "cflags: -${GTK2_CFLAGS}")
        message(STATUS "link dir: -${GTK2_LINK_DIR}")

    ENDIF(WIN32)
ENDIF(UNIX)
# end Dependencies
    
# Set Compiler Flags
SET(CMAKE_CXX_FLAGS "${GTK2_CFLAGS} ${CAIRO_CFLAGS}")

SET(2GEOM_TOYS_SRC 
2dsb2d
arc-bez
clothoid
#conic-2
conic-3
conic-4
conic
convex-test
fitter
one-D
path
#path.h
path-to-svgd
#path-to-svgd.h
poly-test
quad-test
rat-bez
root-finder-comparer
sb1d
sb2d
s-bez
#tensor-reparam
toy-cairo
unit-test-sbasis
gear
)

#SET(2GEOM_EXTRAS_SRC
#)

SET(2GEOM_TOY-FRAMEWORK_SRC
interactive-bits.cpp
interactive-bits.h
toy-framework.cpp
toy-framework.h
path-cairo.cpp
path-cairo.h
#path-cairo.h.gch
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
#ode-toy-cairo.cpp
path-builder.h
#path-cubicto.cpp
path-find-points-of-interest.cpp
path-find-points-of-interest.h
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
TARGET_LINK_LIBRARIES(toy 2geom "${GTK2_LINK_FLAGS} ${CAIRO_LINK_FLAGS}")

OPTION(2GEOM_TOYS
  "Build the projects Toy files"
  ON)
FOREACH(source ${2GEOM_TOYS_SRC})
    ADD_EXECUTABLE(${source} ${source}.cpp)
    TARGET_LINK_LIBRARIES(${source} toy 2geom "${GTK2_LINK_FLAGS} ${CAIRO_LINK_FLAGS}")
ENDFOREACH(source)
ENDIF(2GEOM_TOYS)

OPTION(2GEOM_UNIT_TESTS
  "Build the projects Unit Tests"
  OFF)
FOREACH(source ${2GEOM_TESTS_SRC})
    ADD_EXECUTABLE(${source} ${source}.cpp)
    TARGET_LINK_LIBRARIES(${source} 2geom)
ENDFOREACH(source)
ENDIF(2GEOM_UNIT_TESTS)
