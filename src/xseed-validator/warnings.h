#ifndef __XSEEDVALIDATOR_WARNINGS_H__
#define __XSEEDVALIDATOR_WARNINGS_H__

#include <stdbool.h>

struct warn_options_s
{
    bool treat_as_errors;
    bool ignore_header;
    bool warn_extra_headers;
    bool skip_payload;
};

bool parse_warn_options(struct warn_options_s *warn_options, char *string_parse);

#endif /*__XSEEDVALIDATOR_WARNINGS_H__ */
