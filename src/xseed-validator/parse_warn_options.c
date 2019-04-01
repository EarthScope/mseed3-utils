#include "warnings.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xseed-common/array.h>
#include <xseed-common/xseed_string.h>

bool
parse_warn_options (struct warn_options_s *warn_options, char *string_parse)
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
    // xseedvalidator --file myfile.mseeid -W error
    //split on equals
    char *flag = strtok (tokened[i], "=");
    //char *value = strtok(NULL,"=");

    //trim front and back whitespaces
    //set values
    if (0 == strncmp ("error", flag, strlen ("error")))
    {
      warn_options->treat_as_errors = true;
    }
    else if (0 == strncmp ("skip-header", flag, strlen ("skip-header")))
    {
      warn_options->ignore_header = true;
    }
    else if (0 == strncmp ("skip-extra-header", flag, strlen ("skip-extra-header")))
    {
      warn_options->warn_extra_headers = true;
    }
    else if (0 == strncmp ("skip-payload", flag, strlen ("skip-payload")))
    {
      warn_options->skip_payload = true;
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
