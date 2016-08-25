cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)
PROJECT(lib2geom CXX C) # C is required by CHECK_SYMBOL_EXISTS
SET(2GEOM_MINOR_VERSION 3)
SET(2GEOM_VERSION 0.${2GEOM_MINOR_VERSION})
SET(2GEOM_ABI_VERSION ${2GEOM_MINOR_VERSION}.0)

SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
SET(CMAKE_SKIP_RPATH:BOOL OFF)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts)
ENABLE_TESTING()
INCLUDE(Config2GeomDepends)

INCLUDE_DIRECTORIES (src src/googletest/googletest/include ${CMAKE_CURRENT_BINARY_DIR})
LINK_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/src/2geom)

IF(gsl_FOUND)
# must have GSL
    SET(NEEDS_GSL 
    differential-constraint
    root-finder-comparer
#   contour
    sb-to-bez
    )
    SET(LINK_GSL ${gsl_LINK_FLAGS})
    SET(gsl_CFLAGS "${gsl_CFLAGS} -DHAVE_GSL")
ENDIF(gsl_FOUND) 
IF(pycairo_FOUND)
    SET(pycairo_CFLAGS "${pycairo_CFLAGS} -DHAVE_PYCAIRO")
ENDIF(pycairo_FOUND) 

# Set Compiler Flags
SET(CXX_WARNINGS_FLAGS "-Wall -Wformat -Wformat-security -W -Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch -Werror=return-type")
# set the general CXX flags, note that subproject folders may add to this list for themselves
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GTK2_CFLAGS} ${cairo_CFLAGS} ${gsl_CFLAGS} ${pycairo_CFLAGS} ${CXX_WARNINGS_FLAGS} -O2")

# suppress the very annoying "#pragma ms_struct" Clang warning, caused by -mms-bitfield required for GTK
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (WIN32)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes")
    endif()
endif()

# the required C++11 flag depends on the compiler brand:
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")  # force colored output through make/ninja
    SET(CPP11_FLAG "-std=c++11")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CPP11_FLAG "-std=gnu++0x")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    SET(CPP11_FLAG "-std=c++11")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

OPTION(USE_CPP11
  "Build lib2geom with C++11 enabled"
  OFF)
if (USE_CPP11)
#if (1) 
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPP11_FLAG} -DCPP11")
    # if we want to enable some C++11 code by #ifdefs, add the defines here
    # add_definitions(-DHAS_MOVESEMANTICS)
endif()

OPTION(COMPILE_WITH_PROFILER
  "Build lib2geom with profiling enabled"
  OFF)
if (COMPILE_WITH_PROFILER)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
  SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -pg")
endif()

SET(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM}")

OPTION(2GEOM_BUILD_SHARED
  "Build lib2geom and libtoy as shared libraries."
  OFF)
IF(2GEOM_BUILD_SHARED)
    SET(LIB_TYPE SHARED)
ELSE(2GEOM_BUILD_SHARED)
    SET(LIB_TYPE STATIC)
ENDIF(2GEOM_BUILD_SHARED)

OPTION(2GEOM_USE_GPL_CODE
  "Build lib2geom with GPL licensed Code."
  ON)
IF(2GEOM_USE_GPL_CODE)
  ADD_DEFINITIONS(-DGPL_TAINT)
ELSE(2GEOM_USE_GPL_CODE)
    # Do Nothing
ENDIF(2GEOM_USE_GPL_CODE)

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

ADD_SUBDIRECTORY(src)

# install config.h
FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES ${files} DESTINATION include/2geom-${2GEOM_VERSION}/2geom)
