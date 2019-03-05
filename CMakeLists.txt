cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0054 NEW)

project(lib2geom CXX C) # C is required by CHECK_SYMBOL_EXISTS
set(2GEOM_MAJOR_VERSION 1)
set(2GEOM_MINOR_VERSION 0)
set(2GEOM_PATCH_VERSION 0)
set(2GEOM_VERSION ${2GEOM_MAJOR_VERSION}.${2GEOM_MINOR_VERSION}.${2GEOM_PATCH_VERSION})
set(2GEOM_ABI_VERSION ${2GEOM_MAJOR_VERSION}.${2GEOM_MINOR_VERSION}.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_SKIP_RPATH:BOOL OFF)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts)
set(THREADS_PREFER_PTHREAD_FLAG ON)
enable_testing()

include(CheckCXXSourceCompiles)

# Find dependencies
find_package(Boost 1.40 REQUIRED)
find_package(DoubleConversion REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Cython)
find_package(Threads)
pkg_check_modules(GTK3 gtk+-3.0)
pkg_check_modules(GLIB glib-2.0)
pkg_check_modules(CAIRO cairo)
pkg_check_modules(GSL gsl)

# Add global include and link directories
include_directories(src src/googletest/googletest/include ${CMAKE_CURRENT_BINARY_DIR})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/2geom)

check_cxx_source_compiles("#include <math.h>\nint main() { double a=0.5,b=0.5,c=0.5; sincos(a, &b, &c); return 0; }" HAVE_SINCOS)
if(HAVE_SINCOS)
	add_definitions(-DHAVE_SINCOS)
endif()
if(GSL_FOUND)
	add_definitions(-DHAVE_GSL)
endif()
if(PYCAIRO_FOUND)
    add_definitions(-DHAVE_PYCAIRO)
endif()


#    SET(NEEDS_GSL
#        differential-constraint
#        root-finder-comparer
#        # contour
#        sb-to-bez
#    )

add_compile_options(-Wall -Wformat-security -Woverloaded-virtual -Wpointer-arith -Werror=return-type)
add_compile_options(-O2)

# suppress the very annoying "#pragma ms_struct" Clang warning, caused by -mms-bitfield required for GTK
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if(WIN32)
    	add_compile_options(-Wno-ignored-attributes)
    endif()
    add_compile_options(-Wno-unused-local-typedef)
endif()

option(WITH_PROFILING
    "Build lib2geom with profiling enabled"
    OFF)
  
if(WITH_PROFILING)
    add_compile_options(-pg)
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -pg")
endif()

option(2GEOM_BUILD_SHARED
    "Build lib2geom and libtoy as shared libraries."
    OFF)

if(2GEOM_BUILD_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

option(2GEOM_USE_GPL_CODE
    "Build lib2geom with GPL licensed Code."
    ON)

if(2GEOM_USE_GPL_CODE)
    add_definitions(-DGPL_TAINT)
endif()

include(CMakeScripts/Dist.cmake)

# make unistall target
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

add_subdirectory(src)

# install headers
file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES ${files} DESTINATION include/2geom-${2GEOM_VERSION}/2geom)
