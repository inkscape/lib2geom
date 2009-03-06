SET(2GEOM_TOY-FRAMEWORK-2_SRC
toy-framework-2.cpp
toy-framework-2.h
path-cairo.cpp
path-cairo.h
)


SET(2GEOM_TOYS-2_SRC
rect_01
rect_02
rect_03
pencil
intersect-data
sb2d-solver
minsb2d-solver
aa
ineaa
arc-bez
arc-length-param
boolops
bound-path
box3d
center-warp
circle-fitting
circle-intersect
collinear-normal
conic-3
conic-4
convex-toy
curvature-test
curve-curve-distance
curve-curve-nearest-point-3
curve-intersection-by-bezier-clipping
curve-intersection-by-implicitization
d2sbasis-fitting
d2sbasis-fitting-with-np
ellipse-fitting
ellipse-area-minimizer
#ellipse-transform
elliptiarc-3point-center-fitting
#elliptiarc-5points-fitting
elliptiarc-curve-fitting
evolute
hvlineseg-toy
implicit-toy
inner-product-clip
#involute
line-toy
load-svgd
rect-toy
mesh-warp
metro
match-curve
#normal-bundle
point-curve-nearest-point
offset-toy
pair-intersect
parametrics
parser
path-along-path
path-effects
plane3d
pw-funcs
pw-toy
quad-toy
sb-math-test
sb-to-bez
sbasis-fitting
scribble
self-intersect
squiggles
svg-elliptical-arc-test-toy
uncross
worms
winding-test
sanitize
# these ones have only had a trivial rewrite to toy-2
2dsb2d
bounds-test
cylinder3d
draw-toy
gear
inverse-test
mesh-grad
paptest
precise-flat
pw-compose-test
sb-zeros
sb1d
sb2d
sweep
root-finder-comparer
curvature-curve
sb-of-sb
sb-of-interval
sbasisdim
convole
hatches
rdm-area
ray-sorting-test
sketch-fitter
)


OPTION(2GEOM_TOYS
  "Build the projects Toy files"
  ON)
IF(2GEOM_TOYS)
    # make lib for toy
    ADD_LIBRARY(toy-2 ${LIB_TYPE} ${2GEOM_TOY-FRAMEWORK-2_SRC})
    TARGET_LINK_LIBRARIES(toy-2 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS}")

    FOREACH(source ${2GEOM_TOYS-2_SRC})
        IF(${source} STREQUAL aa)
            ADD_EXECUTABLE(${source} EXCLUDE_FROM_ALL ${source}.cpp)
            TARGET_LINK_LIBRARIES(${source} affa)
        ELSEIF(${source} STREQUAL ineaa)
            ADD_EXECUTABLE(${source} EXCLUDE_FROM_ALL ${source}.cpp)
            TARGET_LINK_LIBRARIES(${source} affa)
        ELSEIF(${source} STREQUAL implicit-toy)
            ADD_EXECUTABLE(${source} EXCLUDE_FROM_ALL ${source}.cpp)
            TARGET_LINK_LIBRARIES(${source} affa)
        ELSE(${source} STREQUAL aa)
            ADD_EXECUTABLE(${source} ${source}.cpp)
        ENDIF(${source} STREQUAL aa)
        IF (WIN32)
        ELSE (WIN32)
	  TARGET_LINK_LIBRARIES(toy-2 "-lrt")
          #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -lrt")
        ENDIF(WIN32)
        TARGET_LINK_LIBRARIES(${source} toy-2 ${LINK_GSL} 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS}")
    ENDFOREACH(source)

ENDIF(2GEOM_TOYS)

