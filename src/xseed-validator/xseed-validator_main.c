#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <libmseed.h>

#include <xseed-common/cmd_opt.h>
#include <xseed-common/files.h>
#include <xseed-common/xseed_string.h>

#include "xseed-validator_config.h"
#include "warnings.h"
#include "validator.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
  #include <xseed-common/vcs_getopt.h>
#else
  #include <unistd.h>
  #include <getopt.h>
#endif

#define MAX_FILE_SIZE 1024

//CMD line option structure
static const struct xseed_option_s args[] = {
    {'h', "help", "   Display usage information", NULL, NO_OPTARG},
    {'j', "schema", "   File containing JSON Schema", NULL, MANDATORY_OPTARG},
    {'v', "verbose", "Verbosity level", NULL, OPTIONAL_OPTARG},
    {'d', "data", "   Print data payload", NULL, OPTIONAL_OPTARG},
    {'W', "       ", "Option flag  *e.g* -W error,skip-payload ", NULL, MANDATORY_OPTARG},
    {'V', "version", "Print program version", NULL, OPTIONAL_OPTARG},
    {0, 0, 0, 0, 0}};

/*! @brief Program to Validate xSEED format files
 *
 */
int
main (int argc, char **argv)
{
  uint8_t verbose        = 0;
  bool print_data        = false;
  char *file_name        = NULL;
  char *schema_file_name = NULL;
  int32_t fail_cnt       = 0;

  //vars to store command line options/args
  char *short_opt_string        = NULL;
  struct option *long_opt_array = NULL;
  int opt;
  struct warn_options_s warn_options[1];
  unsigned char display_usage    = 0;
  unsigned char display_revision = 0;

  bool valid;
  FILE *file        = NULL;
  uint32_t file_cnt = 0;
  uint32_t record_cnt;
  uint64_t record_total = 0;

  char **files = malloc (argc * sizeof (char *));

  //For warning options - TODO Need payload skipping option
  memset (warn_options, 0, sizeof (struct warn_options_s));

  //parse command line args
  xseed_get_short_getopt_string (&short_opt_string, args);
  xseed_get_long_getopt_array (&long_opt_array, args);

  //Get usage options TODO in progress
  int longindex;
  while (-1 != (opt = getopt_long (argc, argv, short_opt_string, long_opt_array, &longindex)))
  {
    switch (opt)
    {
    case 'd':
      print_data = true;
      break;
    case 'v':
      if (0 == optarg)
      {
        verbose++;
      }
      else
      {
        verbose = (uint8_t)strlen (optarg) + 1;
      }
      break;
    case 'W':
      parse_warn_options (warn_options, optarg);
      break;
    case 'h':
      display_usage = 1;
      break;
    case 'V':
      display_revision = 1;
      break;
    case 'j':
      schema_file_name = strndup (optarg, MAX_FILE_SIZE);

      if (!xseed_file_exists (schema_file_name))
      {
        printf ("Error! Cannot read JSON schema file: %s, File Not Found!\n", schema_file_name);
        return EXIT_FAILURE;
      }

      break;
    default:
      // display_usage++;
      break;
    }
    if (display_usage > 0)
    {
      break;
    }
  }

  if (display_usage > 0 || (argc == 1))
  {
    display_help (argv[0], " [options] infile(s)", "Program to validate xSEED format files", args);
    return display_usage < 2 ? EXIT_FAILURE : EXIT_SUCCESS;
  }

  if (display_revision)
  {
    display_version (argv[0], "Program to validate xSEED format files",
                     XSEEDVALIDATOR_VERSION_MAJOR,
                     XSEEDVALIDATOR_VERSION_MINOR,
                     XSEEDVALIDATOR_VERSION_PATCH);
    return EXIT_SUCCESS;
  }

  free (long_opt_array);
  free (short_opt_string);

  while (argc > optind)
  {
    record_cnt = 0;
    file_name  = argv[optind++];

    if (!xseed_file_exists (file_name))
    {
      printf ("Error! Cannot read file: %s, File Not Found! \n", file_name);
      continue;
    }

    if (!xseed_regular_file (file_name))
    {
      printf ("Error! %s, is not a regular file...skipping \n", file_name);
      continue;
    }

    // Open ms file as binary
    file = fopen (file_name, "rb");

    if (file == NULL)
    {
      printf ("Error reading file: %s, fopen failure \n", file_name);
      continue;
    }
    // run verification tests
    // TODO keep tally of failures
    valid = check_file (warn_options, file, schema_file_name, file_name, &record_cnt, print_data, verbose);
    fclose (file);
    record_total = record_total + (uint64_t)record_cnt;
    file_cnt++;

    if (valid)
    {
      if (verbose > 0)
      {
        printf ("xseed-validator RESULT - file %s is VALID xSEED\n", file_name);
      }
    }
    else
    {
      printf ("xseed-validator RESULT - file %s is **NOT** VALID xSEED\n", file_name);
      files[fail_cnt] = strndup (file_name, MAX_FILE_SIZE);
      fail_cnt++;
    }
  }

  if (schema_file_name)
  {
    free (schema_file_name);
  }

  //Final program output
  if (verbose > 0)
  {
    printf ("\n----------------------------------------------------------\n");
  }
#if defined(_WIN32) || defined(__APPLE__)
  printf ("*xseed-validator COMPLETE - %lld record(s) processed in %d file(s)*\n", record_total, file_cnt);
#else
  printf ("xseed-validator COMPLETE - %ld record(s) processed in %d file(s)\n", record_total, file_cnt);
#endif
  if (fail_cnt != 0)
  {
    printf ("xseed-validator FAILED to validate %d file(s) out of the %d file(s) processed\n", fail_cnt, file_cnt);

    printf ("Offending file(s):\n");
    for (int i = 0; i < fail_cnt; i++)
    {
      printf ("%s\n", files[i]);
      free (files[i]);
    }
    free (files);

    printf ("\n");

    return EXIT_FAILURE;
  }
  //return valid ? EXIT_SUCCESS : EXIT_FAILURE;
  return EXIT_SUCCESS;
}
