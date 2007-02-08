#generate parser file with ragel
SET(SVG_PARSER_CPP "${CMAKE_CURRENT_SOURCE_DIR}/svg_path_parser.cpp")
SET(SVG_PARSER_TMP "${CMAKE_CURRENT_SOURCE_DIR}/svg_path_parser.tmp")
SET(SVG_PARSER_RL "${CMAKE_CURRENT_SOURCE_DIR}/svg_path_parser.rl")
SET(GENERATE_SVG_PARSER NOT EXISTS "${SVG_PARSER_CPP}")
SET(REGENERATE_SVG_PARSER "${SVG_PARSER_CPP}" IS_NEWER_THAN "${SVG_PARSER_RL}")
IF(GENERATE_SVG_PARSER OR REGENERATE_SVG_PARSER)
  ADD_CUSTOM_COMMAND(OUTPUT "${SVG_PARSER_CPP}"
                     COMMAND ragel -o "${SVG_PARSER_TMP}" "${SVG_PARSER_RL}"
                     COMMAND rlcodegen -T0 -o "${SVG_PARSER_CPP}" "${SVG_PARSER_TMP}"
                     DEPENDS "${SVG_PARSER_RL}"
                     WORKING_DIRECTORY "${CURRENT_SOURCE_DIR}"
                     COMMENT "Generating svg_path_parser.cpp with ragel"
                     )
ENDIF(GENERATE_SVG_PARSER OR REGENERATE_SVG_PARSER)

SET(2GEOM_SRC
svg_path_parser.cpp
#arc-length.cpp
#arc-length.h
bezier-to-sbasis.h
bezier-utils.cpp
bezier-utils.h
#centroid.cpp
#centroid.h
choose.h
circulator.h
conjugate_gradient.cpp
conjugate_gradient.h
convex-cover.cpp
convex-cover.h
coord.h
floating.h
forward.h
geom.cpp
geom.h
macros.h
matrix.cpp
matrix.h
math-utils.h
maybe.h
multidim-sbasis.h
sb-geometric.h
#nearestpoint.cpp
#path-builder.h
path2-builder.h
#path-nearest.cpp
#path-nearest.h
#path-intersect.cpp
#path-intersect.h
#path-metric.cpp
#path-metric.h
#path-to-polyline.cpp
#path-to-polyline.h
#path.cpp
#path.h
path2.cpp
path2.h
point.h
point.cpp
point-l.h
poly.cpp
poly-dk-solve.cpp
poly-dk-solve.h
poly.h
poly-laguerre-solve.cpp
poly-laguerre-solve.h
quadtree.cpp
quadtree.h
#read-svgd.cpp
#read-svgd.h
rect.cpp
rect.h
refcounted.h
rotate.h
rotate.cpp
s-basis-2d.h
s-basis-2d.cpp
s-basis.cpp
s-basis.h
s-basis-roots.cpp
sb-geometric.cpp
sb-pw.cpp
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
INSTALL(FILES ${files} DESTINATION include/2geom/2geom)

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/2geom.pc.in
                ${CMAKE_BINARY_DIR}/2geom.pc @ONLY IMMEDIATE )
INSTALL(FILES "${CMAKE_BINARY_DIR}/2geom.pc" DESTINATION lib/pkgconfig)

ADD_SUBDIRECTORY (toys)
ADD_SUBDIRECTORY (tests)
ADD_SUBDIRECTORY (python)
