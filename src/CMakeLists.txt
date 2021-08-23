#Build common code for all mseed3-utils
#init common dir
init_target_source_dir(mseed3-common)
ADD_SUBDIRECTORY(mseed3-common)


INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}" ${WJELEMENT_INCLUDE_DIRS} ${MSEED_INCLUDE_DIRS})

ADD_LIBRARY(mseed3-common STATIC ${mseed3-common_SRCS} mseed3-common/regular_file.c)
target_link_libraries(mseed3-common ${MSEED_LIBRARIES} ${WJELEMENT_LIBRARIES})

#check for older linux for defualting to c89, force to c99
IF (${CMAKE_VERSION} VERSION_LESS 3.1)
    IF (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        SET(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
    ENDIF (CMAKE_C_COMPILER_ID STREQUAL "GNU")
ELSE (${CMAKE_VERSION} VERSION_LESS 3.1)
    SET_PROPERTY(TARGET mseed3-common PROPERTY C_STANDARD 99)
ENDIF (${CMAKE_VERSION} VERSION_LESS 3.1)

ADD_SUBDIRECTORY(mseed3-json)
ADD_SUBDIRECTORY(mseed3-text)
ADD_SUBDIRECTORY(mseed3-validator)
