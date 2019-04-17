#include "cmd_opt.h"
#include <stdio.h>
#include <xseed-common/config.h>

void
display_help (char *program_name, char *usage, char *message, const struct xseed_option_s *opts)
{
  printf ("%s\n", message);
  printf ("\nUsage: %s%s\n", program_name, usage);

  printf ("\n         ## Options ##\n");

  const struct xseed_option_s *opt_ptr = opts;

  for (; opt_ptr != NULL && (0 != opt_ptr->short_option || 0 != opt_ptr->long_option || 0 != opt_ptr->description ||
                             0 != opt_ptr->variable_to_fill || 0 != opt_ptr->argument_type);
       opt_ptr++)
  {
    char *option_str_l = opt_ptr->long_option;
    char option_char   = opt_ptr->short_option;
    char *option_dis   = opt_ptr->description;
    printf ("\t -%c %s %s", option_char, option_str_l, option_dis);
    printf ("\n");
  }
}
