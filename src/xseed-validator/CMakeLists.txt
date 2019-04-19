PROJECT(xseed-validator)
SET(XSEEDVALIDATOR_VERSION_MAJOR 1)
SET(XSEEDVALIDATOR_VERSION_MINOR 0)
SET(XSEEDVALIDATOR_VERSION_PATCH 1)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/xseed-validator_config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/xseed-validator_config.h)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

add_sources(xseed-validator xseed-validator_main.c parse_extra_options.c check_file.c
        check_header.c check_extra_headers.c
        check_identifier.c)

ADD_EXECUTABLE(xseed-validator ${xseed-validator_SRCS})
TARGET_LINK_LIBRARIES(xseed-validator xseed-common)
add_test(xseed-validator ${CMAKE_BINARY_DIR}/bin/xseed-validator COMMAND xseed-validator
        ${CMAKE_SOURCE_DIR}/share/reference_datasets/reference-baseline-record-sinusoid-steim2_EH-FDSN-Full.xseed
        -j ${CMAKE_SOURCE_DIR}/share/json_schemas/ExtraHeaders-FDSN.schema.json -vvv)

INSTALL(TARGETS xseed-validator
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_EXECUTE WORLD_WRITE WORLD_READ
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
