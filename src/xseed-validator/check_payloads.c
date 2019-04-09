#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mseedformat.h>
#include <unpack.h>

#include <libmseed.h>

#include "validator.h"
#include "warnings.h"

bool
check_payloads (struct warn_options_s *options, FILE *input, uint32_t payload_len,
                uint8_t payload_fmt, char *file_name,
                uint32_t recordNum, bool print_data, uint8_t verbose)
{
  bool answer = true;

  //Decode and check using libmseed's functions
  MS3Record *msr    = NULL;
  MS3Record *msrOut = NULL;

  int ierr;
  uint32_t flags = 0;

  //unpack data payload, check CRC
  flags |= MSF_UNPACKDATA;
  flags |= MSF_VALIDATECRC;

  recordNum = 0;
  while ((ms3_readmsr (&msr, file_name, 0, NULL, 0, verbose) == MS_NOERROR))
  {
    //TODO get status message
    if (msr->formatversion == 3)
    {
      if (verbose > 2)
        printf ("Record: %d --- Unpacking data for verification\n", recordNum);

      ierr = msr3_unpack_mseed3 (msr->record, msr->reclen, &msrOut, flags, verbose);
      if (ierr != MS_NOERROR)
      {
        //TODO more verbose error output
        printf ("Error! Record: %d ---  Format 3 payload parsing failed. ms_unpack_mseed3 returned: %d\n",
                recordNum, ierr);
        answer = false;
        if (options->treat_as_errors)
        {
          return answer;
        }
      }
      else
      {
        if (verbose > 2)
          printf ("Record: %d --- Data unpacked successfully\n", recordNum);
      }
    }
    else
    {
      printf ("Error! Record: %d ---  Format version not version 3, read as version: %d", recordNum,
              msr->formatversion);
      printf ("Attepting to parse as format 2");
      ierr = msr3_unpack_mseed2 (msr->record, msr->reclen, &msrOut, flags, verbose);
      if (ierr > 0)
      {
        printf ("Error! Record: %d ---  Format 2 payload parsing failed. ms_unpack_mseed2 returned: %d\n",
                recordNum, ierr);
        answer = false;
        if (options->treat_as_errors)
        {
          return answer;
        }
      }
    }

    if (msrOut->numsamples > 0 && print_data)
    {
      int line, col, cnt, samplesize;
      uint64_t lines = (msrOut->numsamples / 6) + 1;
      void *sptr;

      if ((samplesize = ms_samplesize (msrOut->sampletype)) == 0)
      {
        printf ("Error! Record: %d --- Unrecognized sample type: '%c'\n", recordNum, msrOut->sampletype);
        answer = false;
        if (options->treat_as_errors)
        {
          return answer;
        }
      }
      if (msrOut->sampletype == 'a')
      {
        char *ascii     = (char *)msrOut->datasamples;
        uint64_t length = msrOut->numsamples;

        printf ("Record: %d --- ASCII Data:\n", recordNum);

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

    recordNum = recordNum + 1;

    if (msrOut != NULL)
      msr3_free (&msrOut);
  }

  /* Required to cleanup globals */
  if (msr != NULL)
    ms3_readmsr (&msr, NULL, 0, 0, 0, 0);

  return answer;
}
