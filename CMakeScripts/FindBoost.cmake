# - Find Boost libraries
# Go hunting for boost compoments
# Defines:
#  BOOST_INCLUDE

FIND_PATH(BOOST_INCLUDE_DIR boost/weak_ptr.hpp 
                            /usr/include 
                            /usr/local/include
                            C:\\Boost\\Include
                            C:\\Boost\\include\\boost-1_33_1 )


IF(MINGW)
  SET (BOOST_ROOT C:\\Boost)
  FIND_LIBRARY( BOOST_FILESYSTEM_LIBRARY 
                libboost_filesystem-mgw-s
                PATHS ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_FILESYSTEM_LIBRARY_DEBUG 
                libboost_filesystem-mgw-sd
                PATHS ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_PROGRAM_OPTIONS_LIBRARY 
                libboost_program_options-mgw-s
                PATHS ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_PROGRAM_OPTIONS_LIBRARY_DEBUG 
                libboost_program_options-mgw-sd
                PATHS ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY 
                libboost_python-mgw
                PATHS ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY_DEBUG 
                libboost_python-mgw-d
                PATHS ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_SERIALIZATION_LIBRARY 
                libboost_serialization-mgw
                PATHS ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_SERIALIZATION_LIBRARY_DEBUG 
                libboost_python-mgw-d
                PATHS ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_UNIT_TEST_LIBRARY 
                libboost_unit_test_framework-mgw-s
                ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_UNIT_TEST_LIBRARY_DEBUG 
                libboost_unit_test_framework-mgw-s
                PATHS ${BOOST_ROOT}\\lib )
ELSE(MINGW)


FIND_LIBRARY( BOOST_FILESYSTEM_LIBRARY NAMES boost_filesystem
              PATHS /usr/lib /usr/local/lib C:\\Boost\\lib )
FIND_LIBRARY( BOOST_PROGRAM_OPTIONS_LIBRARY NAMES boost_program_options
              PATHS /usr/lib /usr/local/lib C:\\Boost\\lib )
FIND_LIBRARY( BOOST_PYTHON_LIBRARY NAMES boost_python
              PATHS /usr/lib /usr/local/lib C:\\Boost\\lib )
FIND_LIBRARY( BOOST_PYTHON_LIBRARY_DEBUG NAMES boost_python-d
              PATHS /usr/lib /usr/local/lib C:\\Boost\\lib )
FIND_LIBRARY( BOOST_SERIALIZATION_LIBRARY NAMES boost_serialization
              PATHS /usr/lib /usr/local/lib C:\\Boost\\lib )
FIND_LIBRARY( BOOST_SERIALIZATION_LIBRARY_DEBUG NAMES boost_serialization-d
              PATHS /usr/lib /usr/local/lib C:\\Boost\\lib )
FIND_LIBRARY( BOOST_UNIT_TEST_LIBRARY NAMES boost_unit_test_framework 
              PATHS /usr/lib /usr/local/lib C:\\Boost\\lib )

ENDIF(MINGW)

IF (BOOST_INCLUDE_DIR)
  SET(BOOST_FOUND TRUE)
ENDIF (BOOST_INCLUDE_DIR)

IF (BOOST_FOUND)
  IF (NOT Boost_FIND_QUIETLY)
     MESSAGE(STATUS "Found Boost: ${BOOST_INCLUDE_DIR}")
  ENDIF (NOT Boost_FIND_QUIETLY)
ELSE(BOOST_FOUND)
  IF (Boost_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find Boost")
  ENDIF (Boost_FIND_REQUIRED)
ENDIF (BOOST_FOUND)


