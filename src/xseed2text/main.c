#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unpack.h>

#include <libmseed.h>

#include <xseed-common/cmd_opt.h>
#include <xseed-common/files.h>
#include <xseed-common/xseed_string.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <xseed-common/vcs_getopt.h>
#else

#include <getopt.h>
#include <unistd.h>

#endif

//CMD line option structure
static const struct xseed_option_s args[] = {
    {'h', "help", "   Display usage information", NULL, NO_OPTARG},
    {'v', "verbose", "Verbosity level", NULL, OPTIONAL_OPTARG},
    {'d', "data", "   Print data payload", NULL, OPTIONAL_OPTARG},
    {0, 0, 0, 0, 0}};

/*! @brief Prints xSEED file contains in human readable format
 *
 */
int
main (int argc, char **argv)
{

  //vars for parsing xSEED file and payload
  MS3Record *msr    = NULL;
  MS3Record *msrOut = NULL;
  int ierr;
  uint32_t flags = 0;

  //vars to store command line options/args
  char *short_opt_string        = NULL;
  struct option *long_opt_array = NULL;
  int opt;
  int longindex;
  unsigned char display_usage = 0;
  uint8_t verbose             = 0;
  bool print_data             = false;
  char *file_name             = NULL;

  //parse command line args
  xseed_get_short_getopt_string (&short_opt_string, args);
  xseed_get_long_getopt_array (&long_opt_array, args);

  //Set flag to unpack data and check CRC
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
    display_help (argv[0], " [options] infile(s)", "Program to Print a xSEED file in text format", args);
    return display_usage < 2 ? EXIT_FAILURE : EXIT_SUCCESS;
  }

  free (long_opt_array);
  free (short_opt_string);

  while (argc > optind)
  {
    file_name = argv[optind++];

    if (!xseed_file_exists (file_name))
    {
      printf ("Error reading file: %s, File Not Found! \n", file_name);
      continue;
    }

    //Read in records
    //Add 1 to verbose level as verbose = 1 prints nothing extra
    while ((ms3_readmsr (&msr, file_name, 0, NULL, 0, verbose + 1) == MS_NOERROR))
    {
      //Print header info
      msr3_print (msr, verbose + 1);

      //The following code adapted from libmseed - msr3_parse(..)
      //Print payload if enabled
      if (print_data)
      {

        //If mimiSEEDv3
        if (msr->formatversion == 3)
        {
          if (verbose > 0)
            printf ("Unpacking data for verification\n");

          ierr = msr3_unpack_mseed3 (msr->record, msr->reclen, &msrOut, flags, verbose);
          //ierr = msr3_parse(msr->record, msr->reclen,&msrOut, flags, verbose);
          //ierr = ms_parse_raw3 (msr->record, msr->reclen, ppackets);
          if (ierr != MS_NOERROR)
          {
            //TODO more verbose error output
            printf ("Error: Format 3 payload parsing failed. ms_unpack_mseed3 returned: %d\n", ierr);
            return EXIT_FAILURE;
          }
          else
          {
            if (verbose > 0)
              printf ("Data unpacked successfully\n");
          }
        }
        else //If older miniSEED format
        {
          printf ("Error: Format version not version 3, read as version: %d\n", msr->formatversion);
          printf ("Attepting to parse as format 2");
          ierr = msr3_unpack_mseed2 (msr->record, msr->reclen, &msrOut, flags, verbose);
          if (ierr > 0)
          {
            printf ("Error: Format 2 payload parsing failed. ms_unpack_mseed2 returned: %d\n", ierr);
            return EXIT_FAILURE;
          }
        }

        //only attempt to print if data exists
        if (msrOut->numsamples > 0)
        {
          int line, col, cnt, samplesize;
          uint64_t lines = (msrOut->numsamples / 6) + 1;
          void *sptr;

          if ((samplesize = ms_samplesize (msrOut->sampletype)) == 0)
          {
            printf ("Unrecognized sample type: '%c'\n", msrOut->sampletype);
            return -1;
          }
          if (msrOut->sampletype == 'a')
          {
            char *ascii     = (char *)msrOut->datasamples;
            uint64_t length = msrOut->numsamples;

            printf ("ASCII Data:\n");

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
            //if samples are a number
          }
          else
          {
            for (cnt = 0, line = 0; line < lines; line++)
            {
              for (col = 0; col < 6; col++)
              {
                if (cnt < msrOut->numsamples)
                {
                  sptr = (char *)msrOut->datasamples + (cnt * samplesize);

                  if (msrOut->sampletype == 'i')
                    printf ("%10d  ", *(int32_t *)sptr);

                  else if (msrOut->sampletype == 'f')
                    printf ("%10.8g  ", *(float *)sptr);

                  else if (msrOut->sampletype == 'd')
                    printf ("%10.10g  ", *(double *)sptr);

                  cnt++;
                }
              }
              printf ("\n");
            }
          }
        }

        if (msrOut != NULL)
          msr3_free (&msrOut);
      }
    }

    //if(msr != NULL)
    //    msr3_free(&msr);
    //Required to cleanup globals
    ms3_readmsr (&msr, NULL, 0, 0, 0, 0);
  }

  return EXIT_SUCCESS;
}
