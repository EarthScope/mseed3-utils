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

#if 0
    //TODO incomplete, solution to verify data payload without libmseed functions
    char *buffer = (char *) calloc(payload_len +1, sizeof(char));
    if (payload_len > fread(buffer, sizeof(char), payload_len, input))
    {
        printf("Error! Record: %d ---  failed to read record data payload into buffer\n",recordNum);
        answer = false;
        if(options->treat_as_errors)
        {
            return answer;
        }
    }


    if(options->skip_payload)
    {
        return false;
    }

#if 0

    switch (payload_fmt)
    {
        case 0: /* text */
            /* if this was succesfull but already made false we need to keep */
            answer = check_payload_text(options, payload_len, buffer)? answer : false;
            break;
        case 1: /* 16-bit, integer, little-endian*/
        case 3: /* 32-bit, integer, little-endian*/
        case 4: /* IEEE 32-bit floats, little-endian */
        case 5: /* IEEE 64-bit floats (double), little-endian */
        case 10: /* Steim-1 integer compressin, big-endian */
        case 11: /* Steim-2 integer compressin, big-endian */
        case 19: /* Steim-3 integer compressin, big-endian */
        case 53: /* 32-bit integer, little-endian, general compressor */
        case 54: /* 32-bit IEEE floats, little-endian, general compressor */
        case 55: /* 64-bit IEEE floats, little-endian, general compressor */
            break;
        default:
            /* invalid payload type */
            answer = false;
            break;
    }
#endif
    if (buffer)
    {
        free(buffer);
    }
    //End of incomplete solution, see prev TODO

    //Solution using libmseed builtin functions to
#endif

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
    //msr3_print (msr, verbose);

    //TODO get status message
    if (msr->formatversion == 3)
    {
      if (verbose > 2)
        printf ("Record: %d --- Unpacking data for verification\n", recordNum);

      ierr = msr3_unpack_mseed3 (msr->record, msr->reclen, &msrOut, flags, verbose);
      //ierr = msr3_parse(msr->record, msr->reclen,&msrOut, flags, ppackets);
      //ierr = ms_parse_raw3 (msr->record, msr->reclen, ppackets);
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

  //    if(msr != NULL)
  //        msr3_free(&msr);
  //Required to cleanup globals
  ms3_readmsr (&msr, NULL, 0, 0, 0, 0);

  return answer;
}
