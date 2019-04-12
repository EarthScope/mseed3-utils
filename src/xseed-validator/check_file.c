#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <xseed-common/files.h>

#include <libmseed.h>

#include "validator.h"
#include "warnings.h"

/*! @brief Top level function to perform all verification tests on input xSEED file
 *
 *  @param[in] options -W cmd line warn options (currently not implemented)
 *  @param[in] input file pointer to xSEED file
 *  @param[in] schema_file_name json file path parsed from cmd line
 *  @param[in] file_name xSEED file path parsed from cmd line
 *
 */
bool
check_file (struct extra_options_s *options, FILE *input, char *schema_file_name,
            char *file_name, uint32_t *records, bool print_data, uint8_t verbose)
{
  bool valid_header       = false;
  bool valid_ident        = false;
  bool valid_extra_header = false;
  bool valid_payload      = false;

  uint32_t fail_count_rcd  = 0;
  uint32_t recordNum       = 0;
  int64_t file_pos         = 0;
  int file_len             = xseed_file_length (input);

  MS3Record *msr = NULL;
  char recordbuffer[MAXRECLEN];

  if (verbose > 0)
  {
    printf("Reading file %s\n", file_name);
  }

  if (file_len < 0)
  {
    printf ("Error! file %s could not read!\n", file_name);
    return false;
  }

  if (verbose > 1)
  {
    printf ("Record length of %d found, starting verification...\n", file_len);
  }


  /* Loop through all records in the provided file and validate content */
  for (file_pos = lmp_ftell64 (input); file_len > file_pos; file_pos = lmp_ftell64 (input))
  {
    uint8_t identifier_len    = 0;
    uint16_t extra_header_len = 0;
    uint32_t payload_len      = 0;
    uint8_t payload_fmt       = 0;
    uint64_t record_len       = 0;
    bool can_check_payload    = false;

    /* ----Check fixed header----- */
    if (verbose > 2)
    {
      printf ("--- Starting Fixed Header verification for record: %d ---\n", recordNum);
    }

    valid_header = check_header (options, input, file_len, &file_pos, &identifier_len, &extra_header_len,
                                 &payload_len, &payload_fmt, recordNum, verbose);

    if (valid_header && verbose > 1)
    {
      printf ("Record: %d --- Fixed Header is valid!\n", recordNum);
    }
    else if (!valid_header)
    {
      printf ("Error! Record: %d --- Fixed Header is not valid!\n", recordNum);
      if (options->treat_as_errors)
      {
        return false;
      }
      fail_count_rcd += 1;
    }

    /* ----Check identifier----- */
    valid_ident = check_identifier (options, input, identifier_len, recordNum, verbose);
    if (!valid_ident)
    {
      printf ("Error! Record: %d --- Error parsing identifier\n", recordNum);
      if (options->treat_as_errors)
      {
        return false;
      }
      fail_count_rcd += 1;
    }

    /* ----Check extra headers----- */
    if (verbose > 2)
    {
      printf ("--- Completed Header verification for record: %d ---\n", recordNum);
      printf ("--- Starting Extra Header verification for record: %d ---\n", recordNum);
    }

    valid_extra_header = check_extra_headers (options, schema_file_name, input, extra_header_len, recordNum,
                                              verbose);
    if (valid_extra_header && schema_file_name != NULL && extra_header_len > 0 && verbose > 1)
    {
      printf ("Record: %d --- Extra Header is valid!\n", recordNum);
    }
    if (!valid_extra_header)
    {
      printf ("Error! Record: %d --- Extra Header not valid under provided schema!\n", recordNum);
      if (options->treat_as_errors)
      {
        return false;
      }
      fail_count_rcd += 1;
    }

    if (verbose > 2)
    {
      printf ("--- Completed Extra Header verification for record: %d ---\n", recordNum);
    }

    /* ----Check data payload headers----- */
    if (payload_len > 0)
    {
      /* Calculate record length and check that it is within libmseed limits */
      record_len        = XSEED_FIXED_HEADER_LEN + identifier_len + extra_header_len + payload_len;
      can_check_payload = (record_len <= MAXRECLEN);

      if (!options->skip_payload && can_check_payload)
      {
        if (verbose > 2)
        {
          printf ("--- Starting Data Payload verification for record: %d ---\n", recordNum);
        }

        /* Reposition file position to beginning of record and read into buffer */
        lmp_fseek64 (input, file_pos, SEEK_SET);

        if (record_len != fread (recordbuffer, sizeof (char), record_len, input))
        {
          printf ("Fatal Error! Record: %d --- File size mismatch, check input record\n", recordNum);
          fail_count_rcd += 1;
        }

        /* Parse record with libmseed */
        else if (msr3_parse (recordbuffer, MAXRECLEN, &msr, 0, verbose))
        {
          printf ("Fatal Error! Record: %d --- [libmseed] Could not parse record\n", recordNum);
          fail_count_rcd += 1;
        }

        /* Unpack data samples, aka payload */
        else
        {
          int samples = msr3_unpack_data (msr, verbose);

          valid_payload = (samples <= 0) ? false : true;

          if (valid_payload)
          {
            if (verbose > 1)
              printf ("Record: %d --- Data Payload is valid!\n", recordNum);


            if (print_data)
            {
              int line, col, cnt, samplesize;
              uint64_t lines = (msr->numsamples / 6) + 1;
              void *sptr;

              if ((samplesize = ms_samplesize(msr->sampletype)) == 0)
              {
                printf("Error! Record: %d --- Unrecognized sample type: '%c'\n", recordNum, msr->sampletype);
                if (options->treat_as_errors)
                {
                  return false;
                }
                fail_count_rcd += 1;
                continue;
              }
              if (msr->sampletype == 'a')
              {
                char *ascii = (char *) msr->datasamples;
                uint64_t length = msr->numsamples;

                printf("Record: %d --- ASCII Data:\n", recordNum);

                /* Print maximum log message segments */
                while (length > (MAX_LOG_MSG_LENGTH - 1))
                {
                  printf("%.*s", (MAX_LOG_MSG_LENGTH - 1), ascii);
                  ascii += MAX_LOG_MSG_LENGTH - 1;
                  length -= MAX_LOG_MSG_LENGTH - 1;
                }

                /* Print any remaining ASCII and add a newline */
                if (length > 0)
                {
                  printf("%.*s\n", (int) length, ascii);
                } else
                {
                  printf("\n");
                }
              } else
              {
                for (cnt = 0, line = 0; line < lines; line++)
                {
                  for (col = 0; col < 6; col++)
                  {
                    if (cnt < msr->numsamples)
                    {
                      sptr = (char *) msr->datasamples + (cnt * samplesize);

                      if (msr->sampletype == 'i')
                        printf("%10d  ", *(int32_t *) sptr);

                      else if (msr->sampletype == 'f')
                        printf("%10.8g  ", *(float *) sptr);

                      else if (msr->sampletype == 'd')
                        printf("%10.10g  ", *(double *) sptr);

                      cnt++;
                    }
                  }
                  printf("\n");
                }
              }
            }
          }
          else
          {
            printf ("Error! Record: %d --- Data Payload is not valid!\n", recordNum);
            if (options->treat_as_errors)
            {
              return false;
            }
            fail_count_rcd += 1;
          }
        }
      }
      else
      {
        /* Skip payload if not checking it */
        lmp_fseek64 (input, payload_len, SEEK_CUR);

        if (verbose > 0)
        {
          if (!can_check_payload)
            printf ("Cannot check payload of record length %" PRId64 "\n", record_len);
          else
            printf ("Payload validation skipped by user\n");
        }
      }

      if (verbose > 2)
      {
        printf ("--- Completed Data Payload verification for record: %d ---\n", recordNum);
      }
    } /* End of payload check */

    recordNum = recordNum + 1;
  } /* End of buffer input for-loop */

  if (msr)
  {
    msr3_free (&msr);
  }

  if (verbose > 1)
  {
    printf("Completed processing %d record(s)\n", recordNum);
  }

  *records = recordNum;

  if (fail_count_rcd == 0)
    return true;
  else
    return false;
}
