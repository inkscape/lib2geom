SET(2GEOM_TOYS_SRC 
2dsb2d
boolops
bounds-test
cylinder3d
draw-toy
gear
inverse-test
mesh-grad
#nearest-points
#nearest-points2
paptest
precise-flat
pw-compose-test
sb-zeros
sb1d
sb2d
shuttle
sweep
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
arc-bez
arc-length-param
bound-path
box3d
center-warp
circle-intersect
conic-3
conic-4
convex-toy
curvature-test
curve-curve-distance
curve-curve-nearest-point-3
d2sbasis-fitting
d2sbasis-fitting-with-np
ellipse-fitting
elliptiarc-3point-center-fitting
elliptiarc-5points-fitting
elliptiarc-curve-fitting
fitter
hvlineseg-toy
inner-product-clip
load-svgd
mesh-warp
match-curve
#normal-bundle
point-curve-nearest-point
offset-test
pair-intersect
parametrics
parser
path-along-path
plane3d
pw-funcs
pw-toy
quad-toy
sb-math-test
sbasis-fitting
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

