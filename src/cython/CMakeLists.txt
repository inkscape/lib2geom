#TODO - rewrite to use ALLCAPS?

OPTION(2GEOM_CYTHON_BINDINGS
  "Build a python binding with Cython."
  OFF)
OPTION(2GEOM_CYTHON_BUILD_SHARED
  "Build cython shared libraries."
  ON)
IF(2GEOM_CYTHON_BUILD_SHARED)
    SET(LIB_TYPE SHARED)
    SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -shared")
ELSE(2GEOM_CYTHON_BUILD_SHARED)
    SET(LIB_TYPE STATIC)
ENDIF(2GEOM_CYTHON_BUILD_SHARED)


IF(2GEOM_CYTHON_BINDINGS)

        include( UseCython )

        # With CMake, a clean separation can be made between the source tree and the
        # build tree.  When all source is compiled, as with pure C/C++, the source is
        # no-longer needed in the build tree.  However, with pure *.py source, the
        # source is processed directly.  To handle this, we reproduce the availability
        # of the source files in the build tree.
        #add_custom_target( ReplicatePythonSourceTree ALL ${CMAKE_COMMAND} -P
        #  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/ReplicatePythonSourceTree.cmake
        #  ${CMAKE_CURRENT_BINARY_DIR}
        #  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

        #include_directories( ${CYTHON_CMAKE_EXAMPLE_SOURCE_DIR}/include )

        # Process the CMakeLists.txt in the 'src' and 'bin' directory.

        set_source_files_properties(
          _common_decl.pxd
          _common_decl.pyx

          _cy_primitives.pxd
          _cy_primitives.pyx
          
          _cy_rectangle.pxd
          _cy_rectangle.pyx
          
          _cy_affine.pxd
          _cy_affine.pyx
          
          _cy_curves.pxd
          _cy_curves.pyx
          
          _cy_path.pxd
          _cy_path.pyx
          
          _cy_conicsection.pxd
          _cy_conicsection.pyx
          
          cy2geom.pyx

          PROPERTIES CYTHON_IS_CXX 1)

        # Multi-file cython modules do not appear to be working at the moment.
        cython_add_module( _common_decl _common_decl.pyx)


        cython_add_module( _cy_primitives _cy_primitives.pyx)

        cython_add_module( _cy_rectangle _cy_rectangle.pyx)

        cython_add_module( _cy_affine _cy_affine.pyx)

        cython_add_module( _cy_curves _cy_curves.pyx)

        cython_add_module( _cy_path _cy_path.pyx)

        #not finished for now
        #~ cython_add_module( _cy_shape _cy_shape.pyx)

        cython_add_module( _cy_conicsection _cy_conicsection.pyx)

        target_link_libraries(_cy_primitives
            #TODO! linking to static lib2geom.a gives -fPIC error, to compile 
            #you have to enable building dynamic library in cmake . -i
            gsl gslcblas 2geom
        )
        target_link_libraries(_cy_rectangle
            gsl gslcblas 2geom
        )

        target_link_libraries(_cy_affine
            gsl gslcblas 2geom
        )

        target_link_libraries(_cy_curves
            gsl gslcblas 2geom
        )

        target_link_libraries(_cy_path
            gsl gslcblas 2geom
        )

        #~ target_link_libraries(_cy_shape
            #~ gsl gslcblas 2geom
        #~ )

        target_link_libraries(_cy_conicsection
            gsl gslcblas 2geom
        )

        cython_add_module( cy2geom cy2geom.pyx)

        add_test(cython-primitives python2 test-primitives.py)
        add_test(cython-rectangle python2 test-rectangle.py)
        add_test(cython-affine python2 test-affine.py)
        add_test(cython-curves python2 test-curves.py)
        add_test(cython-path python2 test-path.py)
        add_test(cython-conicsection python2 test-conicsection.py)

        # stuff to install the cy2geom package in the Python site-packages directory
        FIND_PACKAGE(PythonLibs)
        IF (WIN32)
            GET_FILENAME_COMPONENT(PYTHON_LIB_INSTALL "${PYTHON_LIBRARY}" PATH)
            GET_FILENAME_COMPONENT(SITEPACKAGE "${PYTHON_LIB_INSTALL}/../Lib/site-packages" ABSOLUTE)
        ELSE (WIN32)
            SET(PYTHON_LIB_INSTALL "/usr/local/lib/python2.7/dist-packages" CACHE STRING "Where to install the cy2geom module?")
            SET(SITEPACKAGE ${PYTHON_LIB_INSTALL})
        ENDIF(WIN32)

        INSTALL(TARGETS _common_decl _cy_primitives  _cy_rectangle  _cy_affine _cy_curves _cy_path _cy_conicsection cy2geom 
                DESTINATION "${SITEPACKAGE}/cy2geom")

ENDIF(2GEOM_CYTHON_BINDINGS)
