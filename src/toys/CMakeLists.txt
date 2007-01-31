SET(2GEOM_TOYS_SRC 
2dsb2d
arc-bez
b-spline
circle-intersect
clothoid
conic-2
conic-3
#conic-4
conic
convex-toy
contour
fitter
one-D
quad-toy
rat-bez
sb1d
sb2d
s-bez
pair-intersect
precise-flat
toy-cairo
gear
cheb
match-curve
mesh-grad
bez-grad
draw-toy
normal-bundle
offset-test
curvature-test
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

# make lib for toy
ADD_LIBRARY(toy ${LIB_TYPE} ${2GEOM_TOY-FRAMEWORK_SRC})
TARGET_LINK_LIBRARIES(toy 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS}")
INSTALL(TARGETS toy
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES ${files} DESTINATION include/2geom/2geom)


OPTION(2GEOM_TOYS
  "Build the projects Toy files"
  ON)
IF(2GEOM_TOYS)
    FOREACH(source ${2GEOM_TOYS_SRC})
        ADD_EXECUTABLE(${source} ${source}.cpp)
        TARGET_LINK_LIBRARIES(${source} toy 2geom "${GTK2_LINK_FLAGS} ${cairo_LINK_FLAGS} ${LINK_GSL}" )
    ENDFOREACH(source)
    INSTALL(TARGETS ${2GEOM_TOYS_SRC}
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
    )

ENDIF(2GEOM_TOYS)
