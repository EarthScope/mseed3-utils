#ifndef __MSEED3_COMMON_CMD_OPT_H__
#define __MSEED3_COMMON_CMD_OPT_H__
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

struct option /* specification for a long form option...	*/
{
  const char *name; /* option name, without leading hyphens */
  int has_arg; /* does it take an argument?		*/
  int *flag; /* where to save its status, or NULL	*/
  int val; /* its associated status value		*/
};

#else

#include <unistd.h>
#include <getopt.h>

#endif

struct mseed3_option_s
{
    char short_option;
    char *long_option;
    char *description;
    int *variable_to_fill;
    char argument_type;
};

enum argument_type_e
{
    NO_OPTARG = 0,
    MANDATORY_OPTARG,
    OPTIONAL_OPTARG
};

typedef struct mseed3_option_s mseed3_option;

extern int mseed3_get_short_getopt_string(char **, const struct mseed3_option_s *);

extern int mseed3_get_long_getopt_array(struct option **, const struct mseed3_option_s *);

extern void display_help(char *program_name, char *usage, char *message, const struct mseed3_option_s *opts);

extern void display_version(char *program_name, char *message, int major, int minor, int patch);

#endif /* __MSEED3_COMMON_CMD_OPT_H__*/
