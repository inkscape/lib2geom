SET(2GEOM_SRC
arc-length.cpp
arc-length.h
bezier-to-sbasis.h
bezier-utils.cpp
bezier-utils.h
centroid.cpp
centroid.h
choose.h
circulator.h
conjugate_gradient.cpp
conjugate_gradient.h
convex-cover.cpp
convex-cover.h
coord.h
dim2.h
floating.h
forward.h
geom.cpp
geom.h
i-coord.h
isnan.h
macros.h
matrix.cpp
matrix.h
math-utils.h
math-utils.cpp
maybe.h
multidim-sbasis.h
sb-geometric.h
#nearestpoint.cpp
path-builder.h
path2-builder.h
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
path-sbasis.h
path-to-polyline.cpp
path-to-polyline.h
path.cpp
path.h
path2.cpp
path2.h
point.h
point.cpp
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
rotate.h
rotate.cpp
s-basis-2d.h
s-basis-2d.cpp
s-basis.cpp
s-basis.h
s-basis-roots.cpp
sb-geometric.cpp
sbasis-poly.cpp
sbasis-poly.h
sbasis-to-bezier.cpp
sbasis-to-bezier.h
scale.h
solve-bezier-one-d.cpp
solve-bezier-parametric.cpp
solver.h
sturm.h
transforms.cpp
transforms.h
translate.h
)

# make lib for 2geom
ADD_LIBRARY(2geom ${LIB_TYPE} ${2GEOM_SRC})
#TARGET_LINK_LIBRARIES(2geom blas gsl)
INSTALL(TARGETS 2geom
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES ${files} DESTINATION include)

ADD_SUBDIRECTORY (toys)
ADD_SUBDIRECTORY (tests)
ADD_SUBDIRECTORY (python)
