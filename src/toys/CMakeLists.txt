SET(2GEOM_TOY-FRAMEWORK-2_SRC
toy-framework-2.cpp
toy-framework-2.h
path-cairo.cpp
path-cairo.h
)
SET(2GEOM_LPE_TOY_FRAMEWORK_SRC
${2GEOM_TOY-FRAMEWORK-2_SRC}
lpe-framework.cpp
lpe-framework.h
)

SET(2GEOM_TOYS-2_SRC
2dsb2d
aa
arc-bez
arc-length-param
boolops-toy
bound-path
bounds-test
box3d
center-warp
circle-fitting
circle-intersect
circle-line-intersect
circle-tangent-fitting
collinear-normal
conic-3
conic-4
conic-5
conic-6
conic-section-toy
#convex-toy
convole
curvature-curve
curvature-test
curve-curve-distance
curve-curve-nearest-time
curve-intersection-by-bezier-clipping
curve-intersection-by-implicitization
cylinder3d
d2sbasis-fitting
d2sbasis-fitting-with-np
draw-toy
ellipse-area-minimizer
ellipse-bezier-intersect-toy
ellipse-fitting
ellipse-intersect-toy
ellipse-line-intersect-toy
#ellipse-transform
elliptiarc-3point-center-fitting
#elliptiarc-5points-fitting
elliptiarc-curve-fitting
elliptical-arc-toy
evolute
filet-minion
find-derivative
gear
hatches
implicit-toy
ineaa
inner-product-clip
intersect-data
inverse-test
#involute
#kerning
kinematic_templates
levelsets-test
line-toy
load-svgd
match-curve
mesh-grad
metro
minsb2d-solver
#normal-bundle
offset-toy
pair-intersect
paptest
parametrics
parser
path-along-path
path-effects
pencil
pencil-2
plane3d
point-curve-nearest-time
portion-test
precise-flat
pw-compose-test
pw-funcs
pw-toy
#ray-sorting-test
rdm-area
rect_01
rect_02
rect_03
rect-toy
#redblack-toy
root-finder-comparer
rtree-toy
sanitize
sb1d
sb2d
sb2d-solver
sbasisdim
sbasis-fitting
sb-math-test
sb-of-interval
sb-of-sb
sb-to-bez
sb-zeros
scribble
self-intersect
sketch-fitter
smash-intersector
squiggles
sweep
sweeper-toy
# these ones have only had a trivial rewrite to toy-2
uncross
winding-test
worms
)

SET(2GEOM_LPE_TOYS_SRC
lpe-test
)

OPTION(2GEOM_TOYS_LPE
  "Build Inkscape Live Path Effect (LPE) Toy files"
  ON)
IF(2GEOM_TOYS_LPE)
    # make lib for lpetoy
    ADD_LIBRARY(lpetoy ${LIB_TYPE} ${2GEOM_LPE_TOY_FRAMEWORK_SRC})
    TARGET_LINK_LIBRARIES(lpetoy 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS}")

    FOREACH(source ${2GEOM_LPE_TOYS_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)

        IF (WIN32 OR APPLE)
        ELSE (WIN32 OR APPLE)
            TARGET_LINK_LIBRARIES(lpetoy "-lrt")
          #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -lrt")
        ENDIF(WIN32 OR APPLE)
        TARGET_LINK_LIBRARIES(${source} lpetoy ${LINK_GSL} 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS}")
    ENDFOREACH(source)

ENDIF(2GEOM_TOYS_LPE)

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
		ELSEIF(${source} STREQUAL boolops-cgal)
			
        ELSE(${source} STREQUAL aa)
            ADD_EXECUTABLE(${source} ${source}.cpp)
        ENDIF(${source} STREQUAL aa)
        IF (WIN32 OR APPLE)
        ELSE (WIN32 OR APPLE)
	  TARGET_LINK_LIBRARIES(toy-2 "-lrt")
          #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -lrt")
        ENDIF(WIN32 OR APPLE)
        TARGET_LINK_LIBRARIES(${source} toy-2 ${LINK_GSL} 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS}")
    ENDFOREACH(source)
ENDIF(2GEOM_TOYS)

