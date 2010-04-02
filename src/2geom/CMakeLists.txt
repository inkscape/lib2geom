#generate parser file with ragel
SET(SVG_PARSER_CPP "${CMAKE_CURRENT_SOURCE_DIR}/svg-path-parser.cpp")
SET(SVG_PARSER_TMP "${CMAKE_CURRENT_SOURCE_DIR}/svg-path-parser.tmp")
SET(SVG_PARSER_RL "${CMAKE_CURRENT_SOURCE_DIR}/svg-path-parser.rl")
SET(GENERATE_SVG_PARSER NOT EXISTS "${SVG_PARSER_CPP}")
SET(REGENERATE_SVG_PARSER "${SVG_PARSER_CPP}" IS_NEWER_THAN "${SVG_PARSER_RL}")
IF( EXISTS "/usr/bin/ragel" OR EXISTS "/usr/local/bin/ragel" )
IF(GENERATE_SVG_PARSER OR REGENERATE_SVG_PARSER)
  EXECUTE_PROCESS(COMMAND ragel --version	OUTPUT_VARIABLE CMD_OUTPUT ERROR_QUIET)
  STRING(REGEX MATCH "version [0-9]+[.]*[0-9]*" RALEG_VERSION ${CMD_OUTPUT})
  STRING(REGEX REPLACE "version ([0-9]+).*" "\\1" RALEG_MAJOR ${RALEG_VERSION})
  STRING(REGEX MATCH "[0-9]+$" RALEG_MINOR ${RALEG_VERSION})
  IF( ${RALEG_MAJOR} LESS 6 AND ${RALEG_MINOR} LESS 18 )
    SET(RLCODEGEN_CMD  rlcodegen)
  ELSE( ${RALEG_MAJOR} LESS 6 AND ${RALEG_MINOR} LESS 18 )
    SET(RLCODEGEN_CMD rlgen-cd)
  ENDIF( ${RALEG_MAJOR} LESS 6 AND ${RALEG_MINOR} LESS 18 )
  ADD_CUSTOM_COMMAND(OUTPUT "${SVG_PARSER_CPP}"
                     COMMAND ragel -o "${SVG_PARSER_TMP}" "${SVG_PARSER_RL}"
                     COMMAND ${RLCODEGEN_CMD} -T0 -o "${SVG_PARSER_CPP}" "${SVG_PARSER_TMP}"
                     DEPENDS "${SVG_PARSER_RL}"
                     WORKING_DIRECTORY "${CURRENT_SOURCE_DIR}"
                     COMMENT "Generating svg_path_parser.cpp with ragel"
                     )
ENDIF(GENERATE_SVG_PARSER OR REGENERATE_SVG_PARSER)
ENDIF( EXISTS "/usr/bin/ragel" OR EXISTS "/usr/local/bin/ragel" )

SET(2GEOM_SRC

svg-path.h
svg-path.cpp
svg-path-parser.h
svg-path-parser.cpp

ord.h

#nearestpoint.cpp
nearest-point.cpp
nearest-point.h
bezier-clipping.cpp
bezier-curve.cpp
bezier-curve.h
circle.cpp
circle.h
conicsec.h
conicsec.cpp
conic_section_clipper.cpp
conic_section_clipper.h
conic_section_clipper_cr.h
#conic_section_clipper_impl.cpp
conic_section_clipper_impl.h

curve.h
curve.cpp
curves.h

ellipse.cpp
ellipse.h
elliptical-arc.cpp
elliptical-arc.h
hvlinesegment.h

line.cpp
line.h
ray.h
sbasis-curve.h
path.cpp
path.h
path-intersection.cpp
path-intersection.h
pathvector.cpp
pathvector.h

forward.h

shape.cpp
shape.h
region.cpp
region.h
crossing.h
crossing.cpp
sweep.cpp
sweep.h

toposweep.cpp
toposweep.h

poly.cpp
poly.h

quadtree.cpp
quadtree.h

redblacktree.cpp
redblacktree.h

rtree.cpp
rtree.h

affine.cpp
affine.h
transforms.cpp
transforms.h

point.h
point.cpp
point-l.h

coord.h

d2.h
d2-sbasis.h
d2-sbasis.cpp
rect.h

piecewise.h
piecewise.cpp

sbasis-of.h
sbasis.cpp
sbasis.h
sbasis-2d.h
sbasis-2d.cpp
sbasisN.h
sbasis-geometric.cpp
sbasis-geometric.h
sbasis-math.h
sbasis-math.cpp
sbasis-poly.cpp
sbasis-poly.h
#chebyshev.cpp # requires gsl, not useful, I think
#chebyshev.h
sbasis-roots.cpp
sbasis-to-bezier.cpp
sbasis-to-bezier.h

bezier-to-sbasis.h

basic-intersection.h
basic-intersection.cpp
recursive-bezier-intersection.cpp
#intersection-by-bezier-clipping.cpp
intersection-by-smashing.h
intersection-by-smashing.cpp

geom.cpp
geom.h

math-utils.h
utils.cpp
utils.h
exception.h
angle.h

bezier-utils.cpp
bezier-utils.h
choose.h
circulator.h
conjugate_gradient.cpp
conjugate_gradient.h
convex-cover.cpp
convex-cover.h
solve-bezier-one-d.cpp
solve-bezier-parametric.cpp
solver.h
svg-elliptical-arc.cpp
svg-elliptical-arc.h

#arc-length.cpp
#arc-length.h

numeric/matrix.cpp

)

# make lib for 2geom
ADD_LIBRARY(2geom ${LIB_TYPE} ${2GEOM_SRC})
TARGET_LINK_LIBRARIES(2geom "${LINK_GSL} ${GTK2_LINK_FLAGS}")
SET_TARGET_PROPERTIES(2geom PROPERTIES SOVERSION "${2GEOM_ABI_VERSION}")
INSTALL(TARGETS 2geom
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES ${files} DESTINATION include/2geom-${2GEOM_VERSION}/2geom)
FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/numeric/*.h")
INSTALL(FILES ${files} DESTINATION include/2geom-${2GEOM_VERSION}/2geom/numeric)

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/2geom.pc.in
                ${CMAKE_BINARY_DIR}/2geom.pc @ONLY IMMEDIATE )
INSTALL(FILES "${CMAKE_BINARY_DIR}/2geom.pc" DESTINATION lib/pkgconfig)
ADD_SUBDIRECTORY (toys)
ADD_SUBDIRECTORY (tests)
ADD_SUBDIRECTORY (py2geom)

