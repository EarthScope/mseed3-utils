#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <libmseed.h>

#include <mseed3-common/cmd_opt.h>
#include <mseed3-common/files.h>
#include <mseed3-common/mseed3_string.h>

#include "mseed3-validator_config.h"
#include "warnings.h"
#include "validator.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
  #include <mseed3-common/vcs_getopt.h>
#else
  #include <unistd.h>
  #include <getopt.h>
#endif

#define MAX_FILE_SIZE 1024

/* CMD line option structure */
static const struct mseed3_option_s args[] = {
    {'h', "help", "   Display usage information", NULL, NO_OPTARG},
    {'j', "schema", "File containing JSON Schema", NULL, MANDATORY_OPTARG},
    {'v', "verbose", "Verbosity level", NULL, OPTIONAL_OPTARG},
    {'W', "       ", "Extra Options\n"
                      "                       "
                      "Usage: -W {option},{option},...\n"
                      "                       "
                      "Options:\n "
                      "                         "
                      "error - Halt processing on validation failure\n"
                      "                          "
                      "skip-payload - Skip payload validation", NULL, MANDATORY_OPTARG},
    {'V', "version", "Print program version", NULL, OPTIONAL_OPTARG},
    {0, 0, 0, 0, 0}};

/*! @brief Program to Validate miniSEED format files
 *
 */
int
main (int argc, char **argv)
{
  uint8_t verbose        = 0;
  char *file_name        = NULL;
  char *schema_file_name = NULL;
  int32_t fail_cnt       = 0;

  /* vars to store command line options/args */
  char *short_opt_string        = NULL;
  struct option *long_opt_array = NULL;
  int opt;
  struct extra_options_s extra_options[1];
  unsigned char display_usage    = 0;
  unsigned char display_revision = 0;

  bool valid;
  FILE *file        = NULL;
  uint32_t file_cnt = 0;
  uint32_t record_cnt;
  uint64_t record_total = 0;

  char **files = malloc (argc * sizeof (char *));

  /* For warning options */
  memset (extra_options, 0, sizeof (struct extra_options_s));

  /* parse command line args */
  mseed3_get_short_getopt_string (&short_opt_string, args);
  mseed3_get_long_getopt_array (&long_opt_array, args);

  /* Get usage options */
  int longindex;
  while (-1 != (opt = getopt_long (argc, argv, short_opt_string, long_opt_array, &longindex)))
  {
    switch (opt)
    {
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
      parse_extra_options (extra_options, optarg);
      break;
    case 'h':
      display_usage = 1;
      break;
    case 'V':
      display_revision = 1;
      break;
    case 'j':
      schema_file_name = strndup (optarg, MAX_FILE_SIZE);

      if (!mseed3_file_exists (schema_file_name))
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
    display_help (argv[0], " [options] infile(s)", "Program to validate miniSEED 3 format files", args);
    return display_usage < 2 ? EXIT_FAILURE : EXIT_SUCCESS;
  }

  if (display_revision)
  {
    display_version (argv[0], "Program to validate miniSEED 3 format files",
                     MSEED3VALIDATOR_VERSION_MAJOR,
                     MSEED3VALIDATOR_VERSION_MINOR,
                     MSEED3VALIDATOR_VERSION_PATCH);
    return EXIT_SUCCESS;
  }

  free (long_opt_array);
  free (short_opt_string);

  while (argc > optind)
  {
    record_cnt = 0;
    file_name  = argv[optind++];

    if (!mseed3_file_exists (file_name))
    {
      printf ("Error! Cannot read file: %s, File Not Found! \n", file_name);
      continue;
    }

    if (!mseed3_regular_file (file_name))
    {
      printf ("Error! %s, is not a regular file...skipping \n", file_name);
      continue;
    }

    /* Open ms file as binary */
    file = fopen (file_name, "rb");

    if (file == NULL)
    {
      printf ("Error reading file: %s, fopen failure \n", file_name);
      continue;
    }

    /* run verification tests */
    valid = check_file (extra_options, file, schema_file_name, file_name, &record_cnt, verbose);
    fclose (file);
    record_total = record_total + (uint64_t)record_cnt;
    file_cnt++;

    if (valid)
    {
      if (verbose > 0)
      {
        printf ("mseed3-validator RESULT - file %s is VALID miniSEED 3\n", file_name);
      }
    }
    else
    {
      printf ("mseed3-validator RESULT - file %s is **NOT** VALID miniSEED 3\n", file_name);
      files[fail_cnt] = strndup (file_name, MAX_FILE_SIZE);
      fail_cnt++;
    }
  }

  if (schema_file_name)
  {
    free (schema_file_name);
  }

  /* Final program output */
  if (verbose > 0)
  {
    printf ("\n----------------------------------------------------------\n");
  }

  printf ("mseed3-validator COMPLETE - %" PRId64 " record(s) processed in %d file(s)\n", record_total, file_cnt);

  if (fail_cnt != 0)
  {
    printf ("mseed3-validator FAILED to validate %d file(s) out of the %d file(s) processed\n", fail_cnt, file_cnt);

    printf ("Offending file(s):\n");

    for (int i = 0; i < fail_cnt; i++)
    {
      printf ("%s\n", files[i]);
      free (files[i]);
    }
    printf ("\n");

  }

  return fail_cnt ? EXIT_FAILURE : EXIT_SUCCESS;
}
