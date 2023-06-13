PROJECT(mseed3-validator)
SET(MSEED3VALIDATOR_VERSION_MAJOR 1)
SET(MSEED3VALIDATOR_VERSION_MINOR 0)
SET(MSEED3VALIDATOR_VERSION_PATCH 5)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mseed3-validator_config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/mseed3-validator_config.h)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

add_sources(mseed3-validator mseed3-validator_main.c parse_extra_options.c check_file.c
        check_header.c check_extra_headers.c
        check_identifier.c)

ADD_EXECUTABLE(mseed3-validator ${mseed3-validator_SRCS})
TARGET_LINK_LIBRARIES(mseed3-validator mseed3-common)
add_test(mseed3-validator ${CMAKE_BINARY_DIR}/bin/mseed3-validator COMMAND mseed3-validator
        ${CMAKE_SOURCE_DIR}/share/reference_datasets/reference-baseline-record-sinusoid-steim2_EH-FDSN-Full.mseed3
        -j ${CMAKE_SOURCE_DIR}/share/json_schemas/ExtraHeaders-FDSN.schema.json -vvv)

INSTALL(TARGETS mseed3-validator
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_EXECUTE WORLD_WRITE WORLD_READ
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
