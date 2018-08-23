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




unset(MSEED_INCLUDE_DIR CACHE)
FIND_PATH(MSEED_INCLUDE_DIR
        NAMES libmseed.h
        PATHS /usr/
              /usr/local/
        ${CMAKE_CURRENT_BINARY_DIR}/libsrc/libmseed/src/MSEED_LIBRARY-build
        PATH_SUFFIXES include)


SET(MSEED_NAMES ${MSEED_NAMES} mseed)

unset(MSEED_LIBRARY CACHE)
FIND_LIBRARY(MSEED_LIBRARY
    NAMES ${MSEED_NAMES}
    PATHS /usr/
          /usr/local/
        ${CMAKE_CURRENT_BINARY_DIR}/libsrc/libmseed/src/MSEED_LIBRARY-build
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
    SET(MSEED_LIBRARIES ${MSEED_LIBRARY} "-lm")
    SET(MSEED_INCLUDE_DIRS ${MSEED_INCLUDE_DIR})
    MARK_AS_ADVANCED(MSEED_ROOT)
ELSE(MSEED_FOUND)
    MESSAGE("*NOTE*: Manually install libmseed to system directory \n                     ---OR--- ")
    MESSAGE("        Type 'make' to download a local copy of libmseed to the local project folder")
ENDIF (MSEED_FOUND)
