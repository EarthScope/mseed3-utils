#include <stdint.h>
#include <stdbool.h>
#include "validator.h"
#include "warnings.h"


//TODO remove this file, this case is handled in check_payloads.c
//When data encoding is ASCII
bool check_payload_text(struct warn_options_s *options, uint32_t payload_len, char *buffer)
{
    /*TODO do checks on text records */
    return true;
}
