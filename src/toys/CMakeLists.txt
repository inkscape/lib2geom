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
circle-intersect
center-warp
clothoid
conic-3
conic-4
convex-toy
curve-toy
curvature-test
cylinder3d
#distort
draw-toy
fitter
gear
worms
inner-product-clip
inverse-test
load-svgd
match-curve
mesh-grad
nearest-points
nearest-points2
#normal-bundle
offset-test
pair-intersect
paptest
parametrics
parser
path-along-path
plane3d
precise-flat
pw-compose-test
pw-toy
pw-funcs
quad-toy
#rat-bez
sb-math-test
sb-zeros
sb1d
sb2d
self-intersect
shuttle
#sketch-toy
sweep
svg-arc
svg-elliptic-arc-toy
#line-toy
#toy-cairo
winding-test
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

ENDIF(2GEOM_TOYS)
