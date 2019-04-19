#include "warnings.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xseed-common/array.h>
#include <xseed-common/xseed_string.h>

/*! @brief parsing additional validator options
 *
 *  @param[out] extra_options_s struct to contain additional options
 *  @param[in] string_parse string containg extra option from cmd line
 *
 */

 
 
bool
parse_extra_options (struct extra_options_s *extra_options, char *string_parse)
{
  char **tokened       = NULL;
  size_t tokened_len   = 0;
  size_t tokened_alloc = 0;
  bool bad_option      = false;
  //split on comma
  for (char *token = strtok (string_parse, ","); token != NULL; token = strtok (NULL, ","))
  {
    while (tokened_alloc <= tokened_len)
    {
      tokened_alloc = expand_array ((void **)&tokened, tokened_alloc, sizeof (char *));
    }
    size_t token_len       = strnlen (token, 1024);
    tokened[tokened_len++] = strndup (token, token_len);
  }
  for (size_t i = 0; i < tokened_len && !bad_option; i++)
  {
    char *flag = strtok (tokened[i], "=");

    if (0 == strncmp ("error", flag, strlen ("error")))
    {
      extra_options->treat_as_errors = true;
    }
    else if (0 == strncmp ("skip-payload", flag, strlen ("skip-payload")))
    {
      extra_options->skip_payload = true;
    }
    else
    {
      bad_option = true;
    }
  }

  for (size_t i = 0; i < tokened_len; i++)
  {
    free (tokened[i]);
    tokened[i] = NULL;
  }
  free (tokened);
  tokened = NULL;
  return !bad_option;
}
