SET(2GEOM_DEPENDS gtk+-2.0 cairo gsl blas)
include(UsePkgConfig)

# Dependencies Packages

FOREACH(dep ${2GEOM_DEPENDS})
    PKGCONFIG_FOUND(${dep} "${dep}_FOUND")
    PKGCONFIG(${dep} "${dep}_INCLUDE_DIR" "${dep}_LINK_DIR" "${dep}_LINK_FLAGS" "${dep}_CFLAGS")
ENDFOREACH(dep)
# This is a hack due to a bug in Cmake vars system,temp fix until cmake 2.4.4 is out //verbalshadow
PKGCONFIG(gtk+-2.0 GTK2_INCLUDE_DIR GTK2_LINK_DIR GTK2_LINK_FLAGS GTK2_CFLAGS)
# end Dependencies

FOREACH(need ${2GEOM_DEPENDS})
    IF("${need}_FOUND")
            message(STATUS "${need} Includes, Compile and Link Flags: FOUND")
      ELSE("${need}_FOUND")
        message(STATUS "${need} Includes, Compile and Link Flags: NOT FOUND")
ENDIF("${need}_FOUND")
ENDFOREACH(need)

INCLUDE (CheckIncludeFiles)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )

#CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)
#CHECK_INCLUDE_FILES ("sys/param.h;sys/mount.h" HAVE_SYS_MOUNT_H)
#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
