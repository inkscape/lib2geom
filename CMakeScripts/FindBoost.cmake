# - Find Boost libraries
# Go hunting for boost compoments
# Defines:
#  BOOST_INCLUDE_DIR


# To find boost on Windows, use DEVLIBS_PATH variable set by mingwenv.bat

FIND_PATH(BOOST_INCLUDE_DIR boost/weak_ptr.hpp 
                            /usr/include 
                            /usr/local/include
                            $ENV{BOOST_DIR} )


IF(MINGW) 	 
  SET (BOOST_ROOT C:\\Boost) 	 
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY 	 
                libboost_python-mgw 	 
                PATHS ${BOOST_ROOT}\\lib ) 	 
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY_DEBUG 	 
                libboost_python-mgw-d 	 
                PATHS ${BOOST_ROOT}\\lib ) 	 
ELSE(MINGW) 	 
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY NAMES boost_python 	 
                PATHS /usr/lib /usr/local/lib C:\\Boost\\lib ) 	 
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY_DEBUG NAMES boost_python-d 	 
                PATHS /usr/lib /usr/local/lib C:\\Boost\\lib ) 	 
ENDIF(MINGW)

IF (BOOST_INCLUDE_DIR)
  SET(BOOST_FOUND TRUE)
ENDIF (BOOST_INCLUDE_DIR)

IF (BOOST_FOUND)
     MESSAGE(STATUS "boost: FOUND  ( ${BOOST_INCLUDE_DIR} )")
ELSE(BOOST_FOUND)
     MESSAGE(FATAL_ERROR "boost: NOT FOUND")
ENDIF (BOOST_FOUND)

INCLUDE_DIRECTORIES( ${BOOST_INCLUDE_DIR} )

