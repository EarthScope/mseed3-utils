PROJECT(xseed-utils)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
ENABLE_LANGUAGE(C)
ENABLE_TESTING()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

IF (UNIX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -D_GNU_SOURCE")
ENDIF(UNIX)

SET(XSEED-UTILS_VERSION_MAJOR 1)
SET(XSEED-UTILS_VERSION_MINOR 0)
SET(XSEED-UTILS_VERSION_PATCH 0)
SET(XSEED-UTILS_VERSION ${XSEED-UTILS_VERSION_MAJOR}.${XSEED-UTILS_VERSION_MINOR}.${XSEED-UTILS_VERSION_PATCH})
MESSAGE("Configuring xseed-utils version: " ${XSEED-UTILS_VERSION})


IF (MSVC)
#Warning, if this is not set WJElement breaks in windows builds
set(CMAKE_BUILD_TYPE Debug)
ENDIF (MSVC)


# Add modules for finding requisites
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
SET(CMAKE_TESTING_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Testing)
SET(CMAKE_BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/bin)



#make setup, include cmake funtions
IF (UNIX)
    INCLUDE(GNUInstallDirs)
ENDIF(UNIX)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
#Built in CMake funtion to check for funtions
INCLUDE(CheckFunctionExists)
#Hand build macros to make adding source collections easily
INCLUDE(AddSources)

#TODO: check for thirdparty libraries (tinycbor)

# Some users Might want to build xseed as a static library
IF (NOT MSVC)
SET(XSEED_BUILD_SHARED ON CACHE BOOL "Build libxseed as a shared library
vs static")
# Tells ADD_LIBRARY to default to static or shared depending on
# XSEED_BUILD_SHARED
OPTION(BUILD_SHARED_LIBS "Build the shared library" ${XSEED_BUILD_SHARED})
ENDIF (NOT MSVC)


#calls Find_libmseed
FIND_PACKAGE(MSEED 3.0)
#Check for these functions
CHECK_FUNCTION_EXISTS(strnlen HAS_STRNLEN)
CHECK_FUNCTION_EXISTS(strndup HAS_STRNDUP)



#Debug for windows building against a static runtime (libs instead of dlls)
#IF (MSVC)
#    foreach(flag_var
#            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
#            CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
#            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
#            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
#        if(${flag_var} MATCHES "/MD")
#            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
#        endif(${flag_var} MATCHES "/MD")
#    endforeach(flag_var)
#ENDIF(MSVC)

#add xseed-utils/libsrc
ADD_SUBDIRECTORY(libsrc)
#add xseed-utils/src
ADD_SUBDIRECTORY(src)



#------------------------------------------------------------------------------#
#                                   CPACK Packaging                            #
#------------------------------------------------------------------------------#
#SET(CPACK_PACKAGE_NAME "xseed-utils")
#SET(CPACK_PACKAGE_VENDOR "ISTI")
#SET(CPACK_PACKAGE_CONTACT "jeffreyleifer@isti.com")
#SET(CPACK_PACKAGE_LICENSE "Apache 2")
#SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Utilites for the xSEED data format")
#SET(CPACK_PACKAGE_VERSION_MAJOR ${XSEED-UTILS_VERSION_MAJOR})
#SET(CPACK_PACKAGE_VERSION_MINOR ${XSEED-UTILS_VERSION_MINOR})
#SET(CPACK_PACKAGE_VERSION_PATCH ${XSEED-UTILS_VERSION_PATCH})
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
#INCLUDE(CPack)

#------------------------------------------------------------------------------#
#                              Configuration Details                           #
#------------------------------------------------------------------------------#
SET(INCLUDE_INSTALL_DIR include/)
SET(SYSCONFIG_INSTALL_DIR etc/xseed-utils/)
INCLUDE(CMakePackageConfigHelpers)

