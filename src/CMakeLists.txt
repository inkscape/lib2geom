#generate parser file with ragel
SET(SVG_PARSER_CPP "${CMAKE_CURRENT_SOURCE_DIR}/svg-path-parser.cpp")
SET(SVG_PARSER_TMP "${CMAKE_CURRENT_SOURCE_DIR}/svg-path-parser.tmp")
SET(SVG_PARSER_RL "${CMAKE_CURRENT_SOURCE_DIR}/svg-path-parser.rl")
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
svg-path.h
svg-path.cpp
svg-path-parser.h
svg-path-parser.cpp
#arc-length.cpp
#arc-length.h
bezier-to-sbasis.h
bezier-utils.cpp
bezier-utils.h
choose.h
circulator.h
conjugate_gradient.cpp
conjugate_gradient.h
convex-cover.cpp
convex-cover.h
coord.h
d2.h
d2.cpp
floating.h
geom.cpp
geom.h
macros.h
matrix.cpp
matrix.h
math-utils.h
maybe.h
sb-geometric.h
sb-math.h
sb-math.cpp
#nearestpoint.cpp
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
pw.cpp
quadtree.cpp
quadtree.h
refcounted.h
s-basis-2d.h
s-basis-2d.cpp
s-basis.cpp
s-basis.h
s-basis-roots.cpp
basic-intersection.h
basic-intersection.cpp
sb-geometric.cpp
sbasis-poly.cpp
sbasis-poly.h
sbasis-to-bezier.cpp
sbasis-to-bezier.h
solve-bezier-one-d.cpp
solve-bezier-parametric.cpp
solver.h
sturm.h
transforms.cpp
transforms.h
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
ADD_SUBDIRECTORY (py2geom)
