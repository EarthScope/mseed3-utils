add_sources(xseed-validator main.c parse_warn_options.c check_file.c
        check_header.c check_extra_headers.c check_payloads.c
        check_payload_text.c check_identifier.c)

ADD_EXECUTABLE(xseed-validator ${xseed-validator_SRCS})
TARGET_LINK_LIBRARIES(xseed-validator xseed-common)
add_test(xseed-validator ${CMAKE_BINARY_DIR}/bin/xseed-validator COMMAND xseed-validator
        ${CMAKE_SOURCE_DIR}/share/reference_datasets/reference-baseline-record-sinusoid-steim2_EH-FDSN-Full.xseed
        -j ${CMAKE_SOURCE_DIR}/share/json_schemas/ExtraHeaders-FDSN.schema.json -vvv)

INSTALL(TARGETS xseed-validator
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_EXECUTE WORLD_WRITE WORLD_READ
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
