#include <stdbool.h>
#include <stdlib.h>

#include <libmseed.h>
#include <parson.h>

#include <xseed-common/cmd_opt.h>
#include <xseed-common/files.h>
#include <xseed-common/xseed_string.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <xseed-common/vcs_getopt.h>
#else

#include <getopt.h>
#include <unistd.h>

#endif

/* CMD line option structure */
static const struct xseed_option_s args[] = {
    {'h', "help", "   Display usage information", NULL, NO_OPTARG},
    {'v', "verbose", "Verbosity level", NULL, OPTIONAL_OPTARG},
    {'d', "data", "   Print data payload", NULL, OPTIONAL_OPTARG},
    {0, 0, 0, 0, 0}};

int print_xseed_2_json (char *file_name, bool print_data, uint8_t verbose);

/*! @brief Program to Print a xSEED file in JSON format
 *
 */
int
main (int argc, char **argv)
{

  //vars to store command line options/args
  char *short_opt_string        = NULL;
  struct option *long_opt_array = NULL;
  int opt;
  int longindex;
  unsigned char display_usage = 0;
  uint8_t verbose             = 0;
  char *file_name             = NULL;
  bool print_data             = false;

  /* parse command line args */
  xseed_get_short_getopt_string (&short_opt_string, args);
  xseed_get_long_getopt_array (&long_opt_array, args);

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
      //display_usage++;
      break;
    }
    if (display_usage > 0)
    {
      break;
    }
  }

  if (display_usage > 0 || (argc == 1))
  {
    display_help (argv[0], " [options] infile(s)", "Program to print an xSEED file in JSON format", args);
    return display_usage < 2 ? EXIT_FAILURE : EXIT_SUCCESS;
  }

  free (long_opt_array);
  free (short_opt_string);

  while (argc > optind)
  {
    file_name = argv[optind++];

    if (!xseed_file_exists (file_name))
    {
      fprintf (stderr, "Error reading file: %s, File Not Found!\n", file_name);
      continue;
    }

    print_xseed_2_json (file_name, print_data, verbose);
  }

  return 0;
}

int
print_xseed_2_json (char *file_name, bool print_data, uint8_t verbose)
{
  MS3Record *msr    = NULL;

  JSON_Status ierr;
  JSON_Value *val         = NULL;
  JSON_Object *jsonObj    = NULL;
  JSON_Value *extraVal    = NULL;
  JSON_Array *payload_arr = NULL;
  JSON_Object *flagsObj   = NULL;

  char string[1024];
  char databuffer[MAXRECLEN];
  uint32_t flags = 0;
  uint64_t records = 0;

  if (!xseed_file_exists (file_name))
  {
    fprintf (stderr, "Error: input file %s not found!", file_name);
    return EXIT_FAILURE;
  }

  /* Set flags to unpack data and check CRC */
  flags |= MSF_UNPACKDATA;
  flags |= MSF_VALIDATECRC;

  /* Loop over all records in input file,
   * Add 1 to verbose level as verbose = 1 prints nothing extra */
  while ((ms3_readmsr (&msr, file_name, 0, NULL, 0, verbose + 1) == MS_NOERROR))
  {
    val     = json_value_init_object ();
    jsonObj = json_value_get_object (val);

    ierr = json_object_set_string (jsonObj, "SID", msr->sid);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : SID\n");
      return EXIT_FAILURE;
    }

    ierr = json_object_set_number (jsonObj, "RecordLength", msr->reclen);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : RecordLength\n");
      return EXIT_FAILURE;
    }

    ierr = json_object_set_number (jsonObj, "FormatVersion", msr->formatversion);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : FormatVersion\n");
      return EXIT_FAILURE;
    }

    /* Add boolean entries for each big flag set */
    if (msr->flags)
    {
      ierr = json_object_set_value (jsonObj, "Flags", json_value_init_object ());

      if (ierr == JSONFailure)
      {
        fprintf (stderr, "Something went wrong generating JSON : Flags (Object)\n");
        return EXIT_FAILURE;
      }

      flagsObj = json_object_get_object (jsonObj, "Flags");

      if (ierr == JSONFailure)
      {
        fprintf (stderr, "Something went wrong getting JSON : Flags (Object)\n");
        return EXIT_FAILURE;
      }

      if (ierr != JSONFailure && bit (msr->flags, 0x01))
        ierr = json_object_set_boolean (flagsObj, "CalibrationSignalsPresent", 1);
      if (ierr != JSONFailure && bit (msr->flags, 0x02))
        ierr = json_object_set_boolean (flagsObj, "TimeTagQuestionable", 1);
      if (ierr != JSONFailure && bit (msr->flags, 0x04))
        ierr = json_object_set_boolean (flagsObj, "ClockLocked", 1);
      if (ierr != JSONFailure && bit (msr->flags, 0x08))
        ierr = json_object_set_boolean (flagsObj, "ReservedBit3", 1);
      if (ierr != JSONFailure && bit (msr->flags, 0x10))
        ierr = json_object_set_boolean (flagsObj, "ReservedBit4", 1);
      if (ierr != JSONFailure && bit (msr->flags, 0x20))
        ierr = json_object_set_boolean (flagsObj, "ReservedBit5", 1);
      if (ierr != JSONFailure && bit (msr->flags, 0x40))
        ierr = json_object_set_boolean (flagsObj, "ReservedBit6", 1);
      if (ierr != JSONFailure && bit (msr->flags, 0x80))
        ierr = json_object_set_boolean (flagsObj, "ReservedBit7", 1);

      if (ierr == JSONFailure)
      {
        fprintf (stderr, "Something went wrong generating JSON : Flags values\n");
        return EXIT_FAILURE;
      }
    }

    ms_nstime2timestrz (msr->starttime, string, SEEDORDINAL, NANO_MICRO_NONE);
    ierr = json_object_set_string (jsonObj, "StartTime", string);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : StartTime\n");
      return EXIT_FAILURE;
    }

    ierr = json_object_set_number (jsonObj, "EncodingFormat", msr->encoding);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : EncodingFormat\n");
      return EXIT_FAILURE;
    }

    ierr = json_object_set_number (jsonObj, "SampleRate", msr3_sampratehz (msr));

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : SampleRate\n");
      return EXIT_FAILURE;
    }

    ierr = json_object_set_number (jsonObj, "SampleCount", (double)msr->samplecnt);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : SampleCount\n");
      return EXIT_FAILURE;
    }

    sprintf (string, "0x%0X", msr->crc);
    ierr = json_object_set_string (jsonObj, "CRC", string);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : CRC\n");
      return EXIT_FAILURE;
    }

    ierr = json_object_set_number (jsonObj, "PublicationVersion", msr->pubversion);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : PublicationVersion\n");
      return EXIT_FAILURE;
    }

    ierr = json_object_set_number (jsonObj, "ExtraLength", msr->extralength);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : ExtraLength\n");
      return EXIT_FAILURE;
    }

    ierr = json_object_set_number (jsonObj, "DataLength", msr->datalength);

    if (ierr == JSONFailure)
    {
      fprintf (stderr, "Something went wrong generating JSON : DataLength\n");
      return EXIT_FAILURE;
    }

    if (msr->extralength > 0 && msr->extra)
    {
      extraVal = json_parse_string (msr->extra);
      ierr     = json_object_set_value (jsonObj, "ExtraHeaders", extraVal);

      if (ierr == JSONFailure)
      {
        fprintf (stderr, "Something went wrong generating JSON : ExtraHeaders\n");
        return EXIT_FAILURE;
      }
    }

    if (print_data)
    {
      ierr = msr3_unpack_data(msr, verbose);

      if (ierr < 0)
      {
        fprintf (stderr, "Error: Payload parsing failed: %s\n", ms_errorstr(ierr));
        return EXIT_FAILURE;
      }

      if (verbose > 0)
        fprintf (stderr, "Data unpacked successfully\n");

      if (msr->numsamples > 0)
      {
        int samplesize;
        void *sptr;

        if ((samplesize = ms_samplesize (msr->sampletype)) == 0)
        {
          fprintf (stderr, "Unrecognized sample type: '%c'\n", msr->sampletype);
          return EXIT_FAILURE;
        }
        if (msr->sampletype == 'a')
        {
          /* Text data payload is not NULL terminated, so copy and make a string */
          memcpy (databuffer, msr->datasamples, msr->numsamples);
          databuffer[msr->numsamples] = '\0';

          ierr = json_object_set_string (jsonObj, "Data", databuffer);

          if (ierr == JSONFailure)
          {
            fprintf (stderr, "Something went wrong generating JSON : Data (ASCII)\n");
            return EXIT_FAILURE;
          }
        }
        else
        {
          ierr = json_object_set_value (jsonObj, "Data", json_value_init_array ());

          if (ierr == JSONFailure)
          {
            fprintf (stderr, "Something went wrong generating JSON : Data (Array)\n");
            return EXIT_FAILURE;
          }

          payload_arr = json_object_get_array (jsonObj, "Data");

          for (int i = 0; i < msr->numsamples && ierr != JSONFailure; i++)
          {
            sptr = (char *)msr->datasamples + (i * samplesize);

            if (msr->sampletype == 'i')
            {
              ierr = json_array_append_number (payload_arr, *(int32_t *)sptr);
            }
            else if (msr->sampletype == 'f')
            {
              ierr = json_array_append_number (payload_arr, *(float *)sptr);
            }
            else if (msr->sampletype == 'd')
            {
              ierr = json_array_append_number (payload_arr, *(double *)sptr);
            }
          }

          if (ierr == JSONFailure)
          {
            fprintf (stderr, "Something went wrong generating JSON : Data (numeric)\n");
            return EXIT_FAILURE;
          }
        }
      }
    }

    char *full_string = json_serialize_to_string_pretty (val);
    printf ("%s%s", (records == 0) ? "" : ",", full_string);
    json_free_serialized_string (full_string);

    if (print_data)
    {
      json_array_clear (payload_arr);
    }

    json_value_free (val);
    records += 1;
  } /* End of loop over records */

  if (msr)
    ms3_readmsr (&msr, NULL, 0, 0, 0, 0);

  return EXIT_SUCCESS;
}
