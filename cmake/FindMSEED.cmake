# - Find the native MSEED includes and library
#

# This module defines
#  MSEED_INCLUDE_DIR, where to find libmseed.h, etc.
#  MSEED_LIBRARIES, the libraries to link against to use libmseed.
#  MSEED_DEFINITIONS, You should ADD_DEFINITONS(${MSEED_DEFINITIONS}) before compiling code that includes libmseed library files.
#  MSEED_FOUND, If false, do not try to use fftw3.
#  MSEED_VERSION, The found libmseed version

#valid for cmake 2.8.3

INCLUDE(CheckFunctionExists)

IF(NOT DEFINED MSEED_BUILD)
   SET(MSEED_BUILD ON)
ENDIF(NOT DEFINED MSEED_BUILD)
   
UNSET(MSEED_SEARCH_PATHS CACHE)
IF(MSEED_BUILD)
    UNSET(MSEED_VERSION CACHE)
    UNSET(MSEED_INCLUDE_DIR CACHE)
    UNSET(MSEED_FOUND CACHE)
    UNSET(MSEED_LIBRARY CACHE)
    SET (MSEED_SEARCH_PATHS
        "${CMAKE_CURRENT_BINARY_DIR}/libsrc/libmseed/src/MSEED_LIBRARY-build"
        "/usr/\n"
        "/usr/local/")
ELSE(MSEED_BUILD)
   SET (MSEED_SEARCH_PATHS
        "/usr/"
        "/usr/local/")
ENDIF(MSEED_BUILD)


FIND_PATH(MSEED_INCLUDE_DIR
        NAMES libmseed.h
        PATHS ${MSEED_SEARCH_PATHS} NO_DEFAULT_PATH
        PATH_SUFFIXES include)

SET(MSEED_NAMES ${MSEED_NAMES} mseed)


FIND_LIBRARY(MSEED_LIBRARY
    NAMES ${MSEED_NAMES}
    PATHS ${MSEED_SEARCH_PATHS} NO_DEFAULT_PATH
    PATH_SUFFIXES lib)


IF (MSEED_LIBRARY AND MSEED_INCLUDE_DIR)
    SET(CMAKE_REQUIRED_INCLUDES ${MSEED_INCLUDE_DIR})

    #Get version information from libmseed
    FILE(STRINGS "${MSEED_INCLUDE_DIR}/libmseed.h" TEST_VERSION
        REGEX "^[ \t]*#define[ \t\n]+LIBMSEED_VERSION"
        #REGEX "^[ \t]*#define[ \t\n]+LIBMSEED[ \t]+(.*)$"
        )
    STRING(REGEX MATCH "[0-9]+(\\.[0-9]+(\\.[0-9]+)?)?" MSEED_VERSION "${TEST_VERSION}")

ENDIF (MSEED_LIBRARY AND MSEED_INCLUDE_DIR)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MSEED FOUND_VAR MSEED_FOUND
    REQUIRED_VARS MSEED_LIBRARY MSEED_INCLUDE_DIR
    VERSION_VAR MSEED_VERSION)

MARK_AS_ADVANCED(MSEED_LIBRARY MSEED_INCLUDE_DIR)
IF (MSEED_FOUND)
    MESSAGE("Using MSEED library FOUND: " ${MSEED_LIBRARY})
    SET(MSEED_LIBRARIES ${MSEED_LIBRARY} "-lm")
    SET(MSEED_INCLUDE_DIRS ${MSEED_INCLUDE_DIR})
    MARK_AS_ADVANCED(MSEED_ROOT)
ELSE(MSEED_FOUND)
   MESSAGE("  Install valid libmseed to system directory")
   IF(MSEED_BUILD)
       MESSAGE("--OR--\n  Type 'make' to download a local copy of libmseed to the local project folder")
   ENDIF(MSEED_BUILD)
ENDIF (MSEED_FOUND)
