SET(2GEOM_BOOST_PYTHON_SRC
etc.cpp
point.cpp
interval.cpp
transforms.cpp
rect.cpp
line.cpp
circle.cpp
ellipse.cpp
conic.cpp
crossing.cpp
sbasis.cpp
bezier.cpp
linear.cpp
pw.cpp
d2.cpp
parser.cpp
path.cpp
ray.cpp
shape.cpp
convexcover.cpp
py2geom.cpp
# curves
#curve.cpp
#bezier-curve.cpp
)

SET(BUILD_BOOST_PYTHON_STATIC FALSE)
IF (BUILD_BOOST_PYTHON_STATIC)
  SET(BOOST_PYTHON_SRC "C:/boost/libs/python/src")
  #define BOOST_PYTHON_STATIC_LIB
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_PYTHON_STATIC_LIB")
  SET(2GEOM_BOOST_PYTHON_SRC
    ${2GEOM_BOOST_PYTHON_SRC}
    ${BOOST_PYTHON_SRC}/dict.cpp
    ${BOOST_PYTHON_SRC}/errors.cpp
    ${BOOST_PYTHON_SRC}/exec.cpp
    ${BOOST_PYTHON_SRC}/import.cpp
    ${BOOST_PYTHON_SRC}/list.cpp
    ${BOOST_PYTHON_SRC}/long.cpp
    ${BOOST_PYTHON_SRC}/module.cpp
    ${BOOST_PYTHON_SRC}/numeric.cpp
    ${BOOST_PYTHON_SRC}/object_operators.cpp
    ${BOOST_PYTHON_SRC}/object_protocol.cpp
    ${BOOST_PYTHON_SRC}/slice.cpp
    ${BOOST_PYTHON_SRC}/str.cpp
    ${BOOST_PYTHON_SRC}/tuple.cpp
    ${BOOST_PYTHON_SRC}/wrapper.cpp

    ${BOOST_PYTHON_SRC}/converter/arg_to_python_base.cpp
    ${BOOST_PYTHON_SRC}/converter/builtin_converters.cpp
    ${BOOST_PYTHON_SRC}/converter/from_python.cpp
    ${BOOST_PYTHON_SRC}/converter/registry.cpp
    ${BOOST_PYTHON_SRC}/converter/type_id.cpp

    ${BOOST_PYTHON_SRC}/object/class.cpp
    ${BOOST_PYTHON_SRC}/object/enum.cpp
    ${BOOST_PYTHON_SRC}/object/function.cpp
    ${BOOST_PYTHON_SRC}/object/inheritance.cpp
    ${BOOST_PYTHON_SRC}/object/iterator.cpp
    ${BOOST_PYTHON_SRC}/object/life_support.cpp
    ${BOOST_PYTHON_SRC}/object/pickle_support.cpp
    ${BOOST_PYTHON_SRC}/object/stl_iterator.cpp
  )
ENDIF (BUILD_BOOST_PYTHON_STATIC)

IF(pycairo_FOUND)
    SET(2GEOM_BOOST_PYTHON_SRC
    ${2GEOM_BOOST_PYTHON_SRC}
    cairo-helpers.cpp
    )
ENDIF(pycairo_FOUND)


OPTION(2GEOM_BOOST_PYTHON
  "Build a python binding with Boost.Python"
  OFF)
IF(2GEOM_BOOST_PYTHON)
        FIND_PACKAGE(Boost)
        FIND_PACKAGE(PythonLibs)
        GET_FILENAME_COMPONENT(PYTHON_LIB_INSTALL "${PYTHON_LIBRARY}" PATH)
        SET(PYDEXTENSION FALSE)
        IF (WIN32)
            GET_FILENAME_COMPONENT(SITEPACKAGE "${PYTHON_LIB_INSTALL}/../Lib/site-packages" ABSOLUTE)
            SET(PYDEXTENSION TRUE)
        ELSE (WIN32)
            GET_FILENAME_COMPONENT(SITEPACKAGE "${PYTHON_LIB_INSTALL}/../dist-packages" ABSOLUTE)
        ENDIF(WIN32)

        INCLUDE_DIRECTORIES( src/ ${PYTHON_INCLUDE_PATH}
                             ${BOOST_INCLUDE_DIR} )

        ADD_LIBRARY(py2geom SHARED ${2GEOM_BOOST_PYTHON_SRC})

        SET_TARGET_PROPERTIES(py2geom PROPERTIES PREFIX "_")
        IF(PYDEXTENSION)
            SET_TARGET_PROPERTIES(py2geom PROPERTIES SUFFIX ".pyd")
        ENDIF(PYDEXTENSION)

        IF (BUILD_BOOST_PYTHON_STATIC)
            TARGET_LINK_LIBRARIES(py2geom 2geom ${PYTHON_LIBRARIES})
        ELSE (BUILD_BOOST_PYTHON_STATIC)
            TARGET_LINK_LIBRARIES(py2geom 2geom ${BOOST_PYTHON_LIBRARY} ${PYTHON_LIBRARIES})
        ENDIF (BUILD_BOOST_PYTHON_STATIC)

        IF(pycairo_FOUND)
            TARGET_LINK_LIBRARIES(py2geom ${cairo_LINK_FLAGS})
        ENDIF(pycairo_FOUND)

        INSTALL(TARGETS py2geom DESTINATION "${SITEPACKAGE}/py2geom")
        INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py" DESTINATION "${SITEPACKAGE}/py2geom")
        IF (WIN32 AND NOT BUILD_BOOST_PYTHON_STATIC)
            # on windows, statically linking boost.python does not work :-(  So we have to copy that DLL to py2geom's dir too
            GET_FILENAME_COMPONENT(BOOST_PYTHON_NAME ${BOOST_PYTHON_LIBRARY} NAME_WE)
            # if the dll name has "lib" in front of it, remove it.
            STRING( REPLACE "lib" "" BOOST_PYTHON_NAME_WO_LIB ${BOOST_PYTHON_NAME})
            GET_FILENAME_COMPONENT(BOOST_PYTHON_PATH ${BOOST_PYTHON_LIBRARY} PATH)
            INSTALL(FILES "${BOOST_PYTHON_PATH}/${BOOST_PYTHON_NAME}.dll" DESTINATION "${SITEPACKAGE}/py2geom" RENAME "${BOOST_PYTHON_NAME_WO_LIB}.dll")
        ENDIF(WIN32 AND NOT BUILD_BOOST_PYTHON_STATIC)

ENDIF(2GEOM_BOOST_PYTHON)



