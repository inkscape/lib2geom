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

affine.cpp
affine.h
angle.h

basic-intersection.cpp
basic-intersection.h
bezier-clipping.cpp
bezier-curve.cpp
bezier-curve.h
bezier-to-sbasis.h
bezier-utils.cpp
bezier-utils.h

choose.h
circle.cpp
circle.h
circulator.h
conicsec.cpp
conicsec.h
conic_section_clipper.h
conic_section_clipper_cr.h
conic_section_clipper_impl.cpp
conic_section_clipper_impl.h
conjugate_gradient.cpp
conjugate_gradient.h
convex-cover.cpp
convex-cover.h
coord.h
crossing.cpp
crossing.h
curve.cpp
curve.h
curves.h

d2-sbasis.cpp
d2-sbasis.h
d2.h

ellipse.cpp
ellipse.h
elliptical-arc.cpp
elliptical-arc.h
exception.h

forward.h

geom.cpp
geom.h

hvlinesegment.h

intersection-by-smashing.cpp
intersection-by-smashing.h

line.cpp
line.h

math-utils.h

nearest-point.cpp
nearest-point.h

numeric/matrix.cpp

ord.h

path-intersection.cpp
path-intersection.h
path.cpp
path.h
pathvector.cpp
pathvector.h
piecewise.cpp
piecewise.h
point-l.h
point.cpp
point.h
poly.cpp
poly.h

quadtree.cpp
quadtree.h

ray.h
rect.h
recursive-bezier-intersection.cpp
redblacktree.cpp
redblacktree.h
region.cpp
region.h
rtree.cpp
rtree.h

sbasis-2d.cpp
sbasis-2d.h
sbasis-curve.h
sbasis-geometric.cpp
sbasis-geometric.h
sbasis-math.cpp
sbasis-math.h
sbasis-of.h
sbasis-poly.cpp
sbasis-poly.h
sbasis-roots.cpp
sbasis-to-bezier.cpp
sbasis-to-bezier.h
sbasis.cpp
sbasis.h
sbasisN.h
shape.cpp
shape.h
solve-bezier-one-d.cpp
solve-bezier-parametric.cpp
solve-bezier-convex-hull.cpp
solver.h
svg-elliptical-arc.cpp
svg-elliptical-arc.h
svg-path-parser.cpp
svg-path-parser.h
svg-path.cpp
svg-path.h
sweep.cpp
sweep.h

toposweep.cpp
toposweep.h
transforms.cpp
transforms.h

utils.cpp
utils.h
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

