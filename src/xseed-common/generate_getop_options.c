#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "cmd_opt.h"
#include "constants.h"

int
xseed_get_short_getopt_string (char **short_opt_string, const struct xseed_option_s *options)
{
  if (NULL == options)
  {
    return XSEED_BAD_INPUT;
  }
  char *temp    = NULL;
  int total_len = 0, alloc_len = 0;
  for (const struct xseed_option_s *option_ptr = options;
       option_ptr != NULL && 0 != option_ptr->short_option; option_ptr++)
  {
    if (option_ptr->short_option < 0 || option_ptr->argument_type < 0 || option_ptr->argument_type > 2)
    {
      continue;
    }
    int len;
    char *str;
    switch (option_ptr->argument_type)
    {
    case MANDATORY_OPTARG:
      len = 1;
      str = ":";
      break;
    case OPTIONAL_OPTARG:
      len = 2;
      str = "::";
      break;
    default:
      len = 0;
      str = "";
      break;
    }
    while (total_len + 2 + len > alloc_len)
    {
      alloc_len = expand_array ((void **)&temp, alloc_len, sizeof (char));
    }
    snprintf (temp + total_len, alloc_len - total_len, "%c%s", option_ptr->short_option, str);
    total_len += 1 + len;
  }
  *short_opt_string = temp;

  return total_len;
}

int
xseed_get_long_getopt_array (struct option **long_opt_array, const struct xseed_option_s *options)
{
  if (NULL == options)
  {
    return XSEED_BAD_INPUT;
  }
  struct option *temp = NULL;
  int total_len = 0, alloc_len = 0;

  for (const struct xseed_option_s *option_ptr = options;
       option_ptr != NULL && 0 != option_ptr->short_option; option_ptr++)
  {
    if (option_ptr->long_option == NULL || option_ptr->argument_type < 0 || option_ptr->argument_type > 2)
    {
      continue;
    }
    while (total_len + 2 > alloc_len)
    {
      alloc_len = expand_array ((void **)&temp, alloc_len, sizeof (struct option));
    }
    (temp + total_len)->name    = option_ptr->long_option;
    (temp + total_len)->has_arg = option_ptr->argument_type;
    (temp + total_len)->flag    = option_ptr->variable_to_fill;
    (temp + total_len)->val     = option_ptr->short_option;
    total_len++;
  }
  memset (temp + total_len, 0, sizeof (struct option));
  *long_opt_array = temp;
  return total_len;
}
