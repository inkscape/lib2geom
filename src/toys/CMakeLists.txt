SET(2GEOM_TOYS_SRC 
2dsb2d
arc-bez
arc-length-param
#b-spline
bounds-test
cheb
circle-intersect
clothoid
conic-3
conic-4
convex-toy
curvature-test
#distort
draw-toy
fitter
gear
vid-intro
#inverse-test
match-curve
mesh-grad
#normal-bundle
offset-test
pair-intersect
parametrics
parser
path-along-path
precise-flat
pw-compose-test
pw-toy
quad-toy
#rat-bez
sb-math-test
sb1d
sb2d
self-intersect
#sketch-toy
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
