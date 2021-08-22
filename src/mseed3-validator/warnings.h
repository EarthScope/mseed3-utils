#ifndef __MSEED3VALIDATOR_WARNINGS_H__
#define __MSEED3VALIDATOR_WARNINGS_H__

#include <stdbool.h>

/* Additional cmd line options:
 * treat_as_errors -> treats validation warnings as errors and halts program,
 * skip-payload -> skips payload validation */
struct extra_options_s
{
    bool treat_as_errors;
    bool skip_payload;
};

bool parse_extra_options(struct extra_options_s *extra_options, char *string_parse);

#endif /*__MSEED3VALIDATOR_WARNINGS_H__ */
