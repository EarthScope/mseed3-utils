#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libmseed.h>

#include "validator.h"
#include "warnings.h"

bool
check_payloads (struct warn_options_s *options, FILE *input, uint32_t payload_len,
                uint8_t payload_fmt, char *file_name,
                uint32_t recordNum, bool print_data, uint8_t verbose)
{
  MS3Record *msr = NULL;
  uint32_t flags = 0;
  bool answer    = true;
  int ierr;

  /* Set flag to check CRC */
  flags |= MSF_VALIDATECRC;

  recordNum = 0;
  while ((ms3_readmsr (&msr, file_name, 0, NULL, 0, verbose) == MS_NOERROR))
  {
    if (verbose > 2)
      printf ("Record: %d --- Unpacking data for verification\n", recordNum);

    ierr = msr3_unpack_data (msr, verbose);

    if (ierr != MS_NOERROR)
    {
      printf ("Error! Record: %d ---  Payload parsing failed. msr3_unpack_data returned: %d\n",
              recordNum, ierr);
      answer = false;
      if (options->treat_as_errors)
      {
        return answer;
      }
    }
    else if (verbose > 2)
    {
      printf ("Record: %d --- Data unpacked successfully\n", recordNum);
    }

    if (msr->numsamples > 0 && print_data)
    {
      int line, col, cnt, samplesize;
      uint64_t lines = (msr->numsamples / 6) + 1;
      void *sptr;

      if ((samplesize = ms_samplesize (msr->sampletype)) == 0)
      {
        printf ("Error! Record: %d --- Unrecognized sample type: '%c'\n", recordNum, msr->sampletype);
        answer = false;
        if (options->treat_as_errors)
        {
          return answer;
        }
      }
      if (msr->sampletype == 'a')
      {
        char *ascii     = (char *)msr->datasamples;
        uint64_t length = msr->numsamples;

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

    recordNum = recordNum + 1;
  }

  /* Required to cleanup globals */
  if (msr != NULL)
    ms3_readmsr (&msr, NULL, 0, 0, 0, 0);

  return answer;
}
