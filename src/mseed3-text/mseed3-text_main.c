#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <libmseed.h>
#include "mseed3-text_config.h"
#include <mseed3-common/cmd_opt.h>
#include <mseed3-common/files.h>
#include <mseed3-common/mseed3_string.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <mseed3-common/vcs_getopt.h>
#else

#include <getopt.h>
#include <unistd.h>

#endif

/* CMD line option structure */
static const struct mseed3_option_s args[] = {
    {'h', "help", "   Display usage information", NULL, NO_OPTARG},
    {'v', "verbose", "Verbosity level", NULL, OPTIONAL_OPTARG},
    {'d', "data", "   Print data payload", NULL, OPTIONAL_OPTARG},
    {'V', "version", "Print program version", NULL, OPTIONAL_OPTARG},
    {0, 0, 0, 0, 0}};

/*! @brief Prints miniSEED file contents in human readable format
 *
 */
int
main (int argc, char **argv)
{
  MS3Record *msr = NULL;
  int ierr;
  uint32_t flags = 0;

  char *short_opt_string        = NULL;
  struct option *long_opt_array = NULL;
  int opt;
  int longindex;
  unsigned char display_usage    = 0;
  unsigned char display_revision = 0;
  uint8_t verbose                = 0;
  bool print_data                = false;
  char *file_name                = NULL;

  /* parse command line args */
  mseed3_get_short_getopt_string (&short_opt_string, args);
  mseed3_get_long_getopt_array (&long_opt_array, args);

  /* Set flags to unpack data and check CRC */
  flags |= MSF_UNPACKDATA;
  flags |= MSF_VALIDATECRC;

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
    case 'h':
      display_usage = 1;
      break;
    case 'V':
      display_revision = 1;
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
    display_help (argv[0], " [options] infile(s)", "Program to print an miniSEED file in text format", args);
    return display_usage < 2 ? EXIT_FAILURE : EXIT_SUCCESS;
  }

  if (display_revision)
  {

    display_version (argv[0], "Program to print a miniSEED file in text format",
                     MSEED3TEXT_VERSION_MAJOR,
                     MSEED3TEXT_VERSION_MINOR,
                     MSEED3TEXT_VERSION_PATCH);
    return EXIT_SUCCESS;
  }

  free (long_opt_array);
  free (short_opt_string);

  while (argc > optind)
  {
    file_name = argv[optind++];

    if (!mseed3_file_exists (file_name))
    {
      fprintf (stderr, "Error reading file: %s, File Not Found! \n", file_name);
      continue;
    }

    /* loop over all records in intput file,
     * Add 1 to verbose level as verbose = 1 prints nothing extra */
    while ((ms3_readmsr (&msr, file_name, 0, NULL, 0, verbose + 1) == MS_NOERROR))
    {
      msr3_print (msr, 2);

      if (print_data)
      {
        ierr = msr3_unpack_data (msr, verbose);

        if (ierr < 0)
        {
          fprintf (stderr, "Error: Payload parsing failed: %s\n", ms_errorstr (ierr));
          return EXIT_FAILURE;
        }

        if (verbose > 1)
          fprintf (stderr, "Data unpacked successfully\n");

        if (msr->numsamples > 0)
        {
          int line, col, cnt, samplesize;
          uint64_t lines = (msr->numsamples / 6) + 1;
          void *sptr;

          printf ("Data:\n");

          if ((samplesize = ms_samplesize (msr->sampletype)) == 0)
          {
            fprintf (stderr, "Unrecognized sample type: '%c'\n", msr->sampletype);
            return EXIT_FAILURE;
          }
          if (msr->sampletype == 'a')
          {
            char *ascii     = (char *)msr->datasamples;
            uint64_t length = msr->numsamples;

            /* Print maximum log message segments */
            while (length > (MAX_LOG_MSG_LENGTH - 1))
            {
              printf ("%.*s", (MAX_LOG_MSG_LENGTH - 1), ascii);
              ascii += MAX_LOG_MSG_LENGTH - 1;
              length -= MAX_LOG_MSG_LENGTH - 1;
            }

            /* Print any remaining ASCII and add a newline */
            if (length > 0)
            {
              printf ("%.*s\n", (int)length, ascii);
            }
            else
            {
              printf ("\n");
            }
          }
          else /* If samples are non-ASCII, i.e. numbers */
          {
            for (cnt = 0, line = 0; line < lines; line++)
            {
              for (col = 0; col < 6; col++)
              {
                if (cnt < msr->numsamples)
                {
                  sptr = (char *)msr->datasamples + (cnt * samplesize);

                  if (msr->sampletype == 'i')
                    printf ("%10d  ", *(int32_t *)sptr);

                  else if (msr->sampletype == 'f')
                    printf ("%10.8g  ", *(float *)sptr);

                  else if (msr->sampletype == 'd')
                    printf ("%10.10g  ", *(double *)sptr);

                  cnt++;
                }
              }
              printf ("\n");
            }
          }
        }
      }
    } /* End of loop over records */

    if (msr)
      ms3_readmsr (&msr, NULL, 0, 0, 0, 0);
  }

  return EXIT_SUCCESS;
}
