SET(SRCS main.c)

ADD_EXECUTABLE(xseed2text ${SRCS})
TARGET_LINK_LIBRARIES(xseed2text xseed-common)
add_test(xseed2text ${CMAKE_BINARY_DIR}/bin/xseed2text COMMAND xseed2text
        ${CMAKE_SOURCE_DIR}/share/ref_datasets/reference-baseline-record-sinusoid-steim2_EH-FDSN-Full.xseed -vvv)
INSTALL(TARGETS xseed2text
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_EXECUTE WORLD_WRITE WORLD_READ
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)