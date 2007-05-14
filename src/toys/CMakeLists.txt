SET(2GEOM_TOYS_SRC 
2dsb2d
arc-bez
#b-spline
circle-intersect
clothoid
conic-2
conic-3
conic-4
conic
convex-toy
distort
fitter
#inverse-test
quad-toy
#rat-bez
sb1d
sb2d
sketch-toy
pair-intersect
self-intersect
precise-flat
pw-toy
#toy-cairo
gear
cheb
match-curve
mesh-grad
#bez-grad
draw-toy
normal-bundle
path-along-path
offset-test
curvature-test
bounds-test
pw-compose-test
arc-length-param
sb-calc-test
sketch-toy
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
