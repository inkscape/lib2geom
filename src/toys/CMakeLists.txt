SET(2GEOM_TOYS_SRC 
2dsb2d
arc-bez
arc-length-param
#b-spline
boolops
bounds-test
box3d
box3d-2
#cheb
center-warp
convex-toy
curvature-test
cylinder3d
dcc-toy
#distort
draw-toy
fitter
gear
inverse-test
load-svgd
mesh-grad
#nearest-points
#nearest-points2
#normal-bundle
paptest
parametrics
parser
plane3d
precise-flat
pw-compose-test
pw-funcs
quad-toy
#rat-bez
sb-zeros
sb1d
sb2d
shuttle
#sketch-toy
sweep
#toy-cairo
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

SET(2GEOM_TOY-FRAMEWORK-2_SRC
interactive-bits.cpp
interactive-bits.h
toy-framework-2.cpp
toy-framework-2.h
path-cairo.cpp
path-cairo.h
)


SET(2GEOM_TOYS-2_SRC
bound-path
circle-intersect
conic-3
conic-4
curve-curve-nearest-point-3
elliptiarc-3point-center-fitting
elliptiarc-5points-fitting
elliptiarc-curve-fitting
hvlineseg-toy
inner-product-clip
mesh-warp
match-curve
point-curve-nearest-point
offset-test
pair-intersect
path-along-path
pw-toy
sb-math-test
self-intersect
squiggles
svg-elliptic-arc-toy
worms
winding-test
)


OPTION(2GEOM_TOYS
  "Build the projects Toy files"
  ON)
IF(2GEOM_TOYS)
    # make lib for toy
    ADD_LIBRARY(toy ${LIB_TYPE} ${2GEOM_TOY-FRAMEWORK_SRC})
    TARGET_LINK_LIBRARIES(toy 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS}")

    FOREACH(source ${2GEOM_TOYS_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} toy 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS} ${LINK_GSL}" )
    ENDFOREACH(source)
    ADD_LIBRARY(toy-2 ${LIB_TYPE} ${2GEOM_TOY-FRAMEWORK-2_SRC})
    TARGET_LINK_LIBRARIES(toy-2 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS}")
    FOREACH(source ${2GEOM_TOYS-2_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} toy-2 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS} ${LINK_GSL}" )
    ENDFOREACH(source)

ENDIF(2GEOM_TOYS)

