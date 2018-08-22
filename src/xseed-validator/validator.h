#ifndef __XSEEDVALIDATOR_VALIDATOR_H__
#define __XSEEDVALIDATOR_VALIDATOR_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "warnings.h"

bool check_file(struct warn_options_s *options, FILE *input, char *schema_file_name, char *file_name, uint32_t *records,
                bool print_data, uint8_t verbose);

bool check_header(struct warn_options_s *options, FILE *input_file, long file_len, long *file_pos,
                  uint8_t *identifier_len, uint16_t *extra_header_len, uint32_t *payload_len, uint8_t *payload_fmt,
                  uint32_t recordNum, int8_t verbose);

bool check_identifier(struct warn_options_s *options, FILE *input, uint8_t identifier_len, uint32_t recordNum,
                      uint8_t verbose);

bool check_extra_headers(struct warn_options_s *options, char *schema, FILE *input, uint16_t extra_header_len,
                         uint32_t recordNum, uint8_t verbose);

bool
check_payloads(struct warn_options_s *options, FILE *input, uint32_t payload_len, uint8_t payload_fmt, char *file_name,
               uint32_t recordNum, bool print_data, uint8_t verbose);

bool check_payload_text(struct warn_options_s *options, uint32_t payload_len, char *buffer);

#endif /* __XSEEDVALIDATOR_VALIDATOR_H__ */
