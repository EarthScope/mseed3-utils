#include <stdbool.h>
#include <stdlib.h>

#include <libmseed.h>
#include <yyjson.h>

#include "mseed3-json_config.h"
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
    {'d', "data", "   Include data payload, default is without", NULL, OPTIONAL_OPTARG},
    {'B', "bare", "   Omit top-level array wrapper", NULL, OPTIONAL_OPTARG},
    {'V', "version", "Print program version", NULL, OPTIONAL_OPTARG},
    {0, 0, 0, 0, 0}};

int print_mseed3_2_json (char *file_name, bool print_data, bool print_array, uint8_t verbose);

/*! @brief Program to Print a miniSEED file in JSON format
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
  unsigned char display_usage    = 0;
  unsigned char display_revision = 0;
  uint8_t verbose                = 0;
  char *file_name                = NULL;
  bool print_data                = false;
  bool print_array               = true;

  /* parse command line args */
  mseed3_get_short_getopt_string (&short_opt_string, args);
  mseed3_get_long_getopt_array (&long_opt_array, args);

  while (-1 != (opt = getopt_long (argc, argv, short_opt_string, long_opt_array, &longindex)))
  {
    switch (opt)
    {
    case 'd':
      print_data = true;
      break;
    case 'B':
      print_array = false;
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
    display_help (argv[0], " [options] infile(s)", "Program to print an miniSEED file in JSON format", args);
    return display_usage < 2 ? EXIT_FAILURE : EXIT_SUCCESS;
  }

  if (display_revision)
  {

    display_version (argv[0], "Program to print a miniSEED file in JSON format",
                     MSEED3JSON_VERSION_MAJOR,
                     MSEED3JSON_VERSION_MINOR,
                     MSEED3JSON_VERSION_PATCH);
    return EXIT_SUCCESS;
  }

  free (long_opt_array);
  free (short_opt_string);

  while (argc > optind)
  {
    file_name = argv[optind++];

    if (!mseed3_file_exists (file_name))
    {
      fprintf (stderr, "Error reading file: %s, File Not Found!\n", file_name);
      continue;
    }

    print_mseed3_2_json (file_name, print_data, print_array, verbose);
  }

  return 0;
}

int
print_mseed3_2_json (char *file_name, bool print_data, bool print_array, uint8_t verbose)
{
  MS3Record *msr = NULL;

  char string[1024];
  uint32_t flags   = 0;
  uint64_t records = 0;

  yyjson_mut_doc *mut_doc;
  yyjson_doc *ehdoc;
  yyjson_read_err rerr;
  bool rv = true;

  if (!mseed3_file_exists (file_name))
  {
    fprintf (stderr, "Error: input file %s not found!", file_name);
    return EXIT_FAILURE;
  }

  /* Set flags to check CRC and unpack data */
  flags |= MSF_VALIDATECRC;
  if (print_data)
    flags |= MSF_UNPACKDATA;

  if (print_array)
    printf ("[");

  /* Loop over all records in input file,
   * Add 1 to verbose level as verbose = 1 prints nothing extra */
  while ((ms3_readmsr (&msr, file_name, flags, verbose + 1) == MS_NOERROR))
  {
    mut_doc = yyjson_mut_doc_new (NULL);

    if (!mut_doc)
    {
      fprintf (stderr, "Cannot initialize JSON document, out of memory?\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/SID", yyjson_mut_str (mut_doc, msr->sid)))
    {
      fprintf (stderr, "Something went wrong generating JSON : SID\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/RecordLength", yyjson_mut_sint (mut_doc, msr->reclen)))
    {
      fprintf (stderr, "Something went wrong generating JSON : RecordLength\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/FormatVersion", yyjson_mut_sint (mut_doc, msr->formatversion)))
    {
      fprintf (stderr, "Something went wrong generating JSON : FormatVersion\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/Flags/RawUInt8", yyjson_mut_uint (mut_doc, msr->flags)))
    {
      fprintf (stderr, "Something went wrong generating JSON : Flags RawUint8\n");
      return EXIT_FAILURE;
    }

    /* Add boolean entries for each bit flag set */
    if (msr->flags)
    {
      if (bit (msr->flags, 0x01))
        rv = yyjson_mut_doc_ptr_set (mut_doc, "/Flags/CalibrationSignalsPresent", yyjson_mut_bool (mut_doc, true));
      if (rv && bit (msr->flags, 0x02))
        rv = yyjson_mut_doc_ptr_set (mut_doc, "/Flags/TimeTagQuestionable", yyjson_mut_bool (mut_doc, true));
      if (rv && bit (msr->flags, 0x04))
        rv = yyjson_mut_doc_ptr_set (mut_doc, "/Flags/ClockLocked", yyjson_mut_bool (mut_doc, true));
      if (rv && bit (msr->flags, 0x08))
        rv = yyjson_mut_doc_ptr_set (mut_doc, "/Flags/ReservedBit3", yyjson_mut_bool (mut_doc, true));
      if (rv && bit (msr->flags, 0x10))
        rv = yyjson_mut_doc_ptr_set (mut_doc, "/Flags/ReservedBit4", yyjson_mut_bool (mut_doc, true));
      if (rv && bit (msr->flags, 0x20))
        rv = yyjson_mut_doc_ptr_set (mut_doc, "/Flags/ReservedBit5", yyjson_mut_bool (mut_doc, true));
      if (rv && bit (msr->flags, 0x40))
        rv = yyjson_mut_doc_ptr_set (mut_doc, "/Flags/ReservedBit6", yyjson_mut_bool (mut_doc, true));
      if (rv && bit (msr->flags, 0x80))
        rv = yyjson_mut_doc_ptr_set (mut_doc, "/Flags/ReservedBit7", yyjson_mut_bool (mut_doc, true));

      if (rv == false)
      {
        fprintf (stderr, "Something went wrong generating JSON : Flags values\n");
        return EXIT_FAILURE;
      }
    }

    ms_nstime2timestr (msr->starttime, string, ISOMONTHDAY_Z, NANO);
    if (!yyjson_mut_doc_ptr_set (mut_doc, "/StartTime", yyjson_mut_strcpy (mut_doc, string)))
    {
      fprintf (stderr, "Something went wrong generating JSON : StartTime\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/EncodingFormat", yyjson_mut_sint (mut_doc, msr->encoding)))
    {
      fprintf (stderr, "Something went wrong generating JSON : EncodingFormat\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/SampleRate", yyjson_mut_real (mut_doc, msr3_sampratehz (msr))))
    {
      fprintf (stderr, "Something went wrong generating JSON : SampleRate\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/SampleCount", yyjson_mut_sint (mut_doc, msr->samplecnt)))
    {
      fprintf (stderr, "Something went wrong generating JSON : SampleCount\n");
      return EXIT_FAILURE;
    }

    sprintf (string, "0x%0X", msr->crc);
    if (!yyjson_mut_doc_ptr_set (mut_doc, "/CRC", yyjson_mut_strcpy (mut_doc, string)))
    {
      fprintf (stderr, "Something went wrong generating JSON : CRC\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/PublicationVersion", yyjson_mut_sint (mut_doc, msr->pubversion)))
    {
      fprintf (stderr, "Something went wrong generating JSON : PublicationVersion\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/ExtraLength", yyjson_mut_sint (mut_doc, msr->extralength)))
    {
      fprintf (stderr, "Something went wrong generating JSON : ExtraLength\n");
      return EXIT_FAILURE;
    }

    if (!yyjson_mut_doc_ptr_set (mut_doc, "/DataLength", yyjson_mut_sint (mut_doc, msr->datalength)))
    {
      fprintf (stderr, "Something went wrong generating JSON : DataLength\n");
      return EXIT_FAILURE;
    }

    if (msr->extralength > 0 && msr->extra)
    {
      ehdoc = yyjson_read_opts (msr->extra, msr->extralength, 0, NULL, &rerr);

      if (!yyjson_mut_doc_ptr_set (mut_doc, "/ExtraHeaders",
                                   yyjson_mut_doc_get_root (yyjson_doc_mut_copy(ehdoc, NULL))))
      {
        fprintf (stderr, "Something went wrong generating JSON : ExtraHeaders\n");
        return EXIT_FAILURE;
      }
    }

    /* Build array of data samples if present */
    if (msr->numsamples > 0)
    {
      yyjson_mut_val *array;
      int samplesize;
      void *sptr;

      if ((samplesize = ms_samplesize (msr->sampletype)) == 0)
      {
        fprintf (stderr, "Unrecognized sample type: '%c'\n", msr->sampletype);
        return EXIT_FAILURE;
      }

      if (msr->sampletype == 't')
      {
        rv = yyjson_mut_doc_ptr_set (mut_doc, "/Data",
                                     yyjson_mut_strn (mut_doc,
                                                      (const char *)msr->datasamples,
                                                      msr->numsamples));

        if (rv == false)
        {
          fprintf (stderr, "Something went wrong generating JSON : Data (Text)\n");
          return EXIT_FAILURE;
        }
      }
      else
      {
        if ((array = yyjson_mut_arr (mut_doc)) == NULL ||
            !yyjson_mut_doc_ptr_set (mut_doc, "/Data", array))
        {
          fprintf (stderr, "Something went wrong generating JSON : Data array\n");
          return EXIT_FAILURE;
        }

        for (int i = 0; i < msr->numsamples && rv != false; i++)
        {
          sptr = (char *)msr->datasamples + (i * samplesize);

          if (msr->sampletype == 'i')
          {
            rv = yyjson_mut_arr_append (array, yyjson_mut_sint (mut_doc, *(int32_t *)sptr));
          }
          else if (msr->sampletype == 'f')
          {
            rv = yyjson_mut_arr_append (array, yyjson_mut_real (mut_doc, *(float *)sptr));
          }
          else if (msr->sampletype == 'd')
          {
            rv = yyjson_mut_arr_append (array, yyjson_mut_real (mut_doc, *(double *)sptr));
          }
        }

        if (rv == false)
        {
          fprintf (stderr, "Something went wrong generating JSON : Data (numeric)\n");
          return EXIT_FAILURE;
        }
      }
    }

    yyjson_write_err werr;
    char *serialized = yyjson_mut_write_opts (mut_doc, YYJSON_WRITE_PRETTY, NULL, NULL, &werr);

    if (serialized == NULL)
    {
      fprintf (stderr, "Something went wrong generating JSON : %s\n", werr.msg);
      return EXIT_FAILURE;
    }

    printf ("%s%s", (records == 0) ? "" : ",", serialized);

    yyjson_mut_doc_free (mut_doc);

    records += 1;
  } /* End of loop over records */

  if (print_array)
    printf ("]");

  if (msr)
    ms3_readmsr (&msr, NULL, flags, verbose + 1);

  return EXIT_SUCCESS;
}
