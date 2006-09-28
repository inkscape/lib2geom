SET(2GEOM_DEPENDS gtk+-2.0 cairo gsl blas)
include(UsePkgConfig)

# Dependencies Packages

FOREACH(dep ${2GEOM_DEPENDS})
    # This is a hack due to a bug in Cmake vars system,temp fix until cmake 2.4.4 is out //verbalshadow
    IF("${dep}" MATCHES "gtk\\+-2.0")
        SET(dep_name "GTK2")
    ELSE("${dep}" MATCHES "gtk\\+-2.0")
        SET(dep_name "${dep}")
    ENDIF("${dep}" MATCHES "gtk\\+-2.0")
    
    PKGCONFIG_FOUND(${dep} "${dep}_FOUND")
    PKGCONFIG(${dep} "${dep_name}_INCLUDE_DIR" "${dep_name}_LINK_DIR" "${dep_name}_LINK_FLAGS" "${dep_name}_CFLAGS")
#    PKGCONFIG_VERSION(${dep} "${dep}_VERSION")
    IF("${dep}_FOUND")
        message(STATUS "${dep} Includes, Compile and Link Flags: FOUND")
    ELSE("${dep}_FOUND")
        message(STATUS "${dep} Includes, Compile and Link Flags: NOT FOUND")
    ENDIF("${dep}_FOUND")
ENDFOREACH(dep)
# end Dependencies


INCLUDE (CheckIncludeFiles)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )

#CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)
#CHECK_INCLUDE_FILES ("sys/param.h;sys/mount.h" HAVE_SYS_MOUNT_H)
#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
