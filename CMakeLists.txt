PROJECT(lib2geom CXX)
SET(2GEOM_VERSION 0.1.0)
SET(CMAKE_BACKWARDS_COMPATIBILITY 2.4)
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)

SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
SET(CMAKE_SKIP_RPATH:BOOL OFF)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts)
INCLUDE(Config2GeomDepends)

INCLUDE_DIRECTORIES (src/2geom)
LINK_DIRECTORIES (src/2geom)

IF(gsl_FOUND)
# must have GSL
    SET(NEEDS_GSL 
    differential-constraint
    root-finder-comparer
#   contour
    sb-to-bez
    )
    SET(LINK_GSL ${gsl_LINK_FLAGS})
    SET(gsl_CFLAGS "${gsl_CFLAGS} -DHAVE_GDL")
ENDIF(gsl_FOUND) 

# Set Compiler Flags
SET(CXX_WARNINGS_FLAGS "-Wall -Wformat -Werror=format-security -W -Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch")
SET(CMAKE_CXX_FLAGS "${GTK2_CFLAGS} ${cairo_CFLAGS} ${gsl_CFLAGS} ${CXX_WARNINGS_FLAGS}")
SET(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j2")

SET(CMAKE_MAJOR_MINOR "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}")
IF("${CMAKE_MAJOR_MINOR}" STREQUAL "2.2")
    SET(CMAKE_AR "ar")
    SET(CMAKE_RANLIB "ranlib")
ENDIF("${CMAKE_MAJOR_MINOR}" STREQUAL "2.2")

OPTION(2GEOM_BUILD_SHARED
  "Build lib2geom and libtoy as shared libraries."
  OFF)
IF(2GEOM_BUILD_SHARED)
    SET(LIB_TYPE SHARED)
ELSE(2GEOM_BUILD_SHARED)
    SET(LIB_TYPE STATIC)
ENDIF(2GEOM_BUILD_SHARED)

#make dist target
SET(2GEOM_DIST_PREFIX "${PROJECT_NAME}-${2GEOM_VERSION}")
ADD_CUSTOM_TARGET(dist svn export --force -q "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/${2GEOM_DIST_PREFIX}" 
		COMMAND tar -czf "${CMAKE_BINARY_DIR}/${2GEOM_DIST_PREFIX}.tar.gz" -C "${CMAKE_BINARY_DIR}" --exclude=".hidden" ${2GEOM_DIST_PREFIX}
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

#make unistall target
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

ADD_SUBDIRECTORY (src/2geom)
