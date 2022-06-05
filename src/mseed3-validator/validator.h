#ifndef __MSEED3VALIDATOR_VALIDATOR_H__
#define __MSEED3VALIDATOR_VALIDATOR_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "warnings.h"

#define MSEED3_FIXED_HEADER_LEN 40

bool check_file(struct extra_options_s *options, FILE *input, char *schema_file_name,
                char *file_name, uint32_t *records, uint8_t verbose);

bool check_header(struct extra_options_s *options, FILE *input_file, long file_len, int64_t *file_pos,
                  uint8_t *identifier_len, uint16_t *extra_header_len, uint32_t *payload_len,
                  uint8_t *payload_fmt, uint32_t recordNum, int8_t verbose);

bool check_identifier(struct extra_options_s *options, FILE *input, uint8_t identifier_len,
                      uint32_t recordNum, uint8_t verbose);

bool check_extra_headers(struct extra_options_s *options, char *schema, FILE *input,
                         uint16_t extra_header_len, uint32_t recordNum, uint8_t verbose);

bool
check_payloads(struct extra_options_s *options, FILE *input, uint32_t payload_len,
               uint8_t payload_fmt, char *file_name,
               uint32_t recordNum, bool print_data, uint8_t verbose);

bool check_payload_text(struct extra_options_s *options, uint32_t payload_len, char *buffer);

bool is_valid_gbl;

#endif /* __MSEED3VALIDATOR_VALIDATOR_H__ */
