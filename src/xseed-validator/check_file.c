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
check_file (struct warn_options_s *options, FILE *input, char *schema_file_name,
            char *file_name, uint32_t *records, bool print_data, uint8_t verbose)
{
  bool valid_header       = false;
  bool valid_ident        = false;
  bool valid_extra_header = false;
  bool valid_payload      = false;

  uint32_t fail_count_rcd  = 0;
  uint32_t fail_count_file = 0;
  uint32_t recordNum       = 0;
  int64_t file_pos         = 0;
  int file_len             = xseed_file_length (input);

  if (verbose > 0)
  {
    printf("Reading file %s\n", file_name);
  }

  if (verbose > 1)
  {
    printf ("Record length of %d found, starting verification...\n", file_len);
  }

  if (file_len < 0)
  {
    printf ("Error! file %s could not read!\n", file_name);
    return false;
  }

  /* Loop through all records in the provided file and validate content */
  for (file_pos = lmp_ftell64 (input); file_len > file_pos; file_pos = lmp_ftell64 (input))
  {
    uint8_t identifier_len    = 0;
    uint16_t extra_header_len = 0;
    uint32_t payload_len      = 0;
    uint8_t payload_fmt       = 0;

    //----Check fixed header-----
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
      fail_count_rcd = fail_count_rcd + 1;
    }

    //----Check identifier-----
    valid_ident = check_identifier (options, input, identifier_len, recordNum, verbose);
    if (!valid_ident)
    {
      printf ("Error! Record: %d --- Error parsing identifier\n", recordNum);
      if (options->treat_as_errors)
      {
        return false;
      }
      fail_count_rcd = fail_count_rcd + 1;
    }

    //----Check extra headers-----
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
      fail_count_rcd = fail_count_rcd + 1;
    }

    if (verbose > 2)
    {
      printf ("--- Completed Extra Header verification for record: %d ---\n", recordNum);
    }

    //----Initial payload check: length only-----
    //-----Need to read off the buffer to keep things moving
    //TODO check payload via buffer, for now payloads are checked via libmseed after all headers are checked
    //Check payload length
    char *buffer = (char *)calloc (payload_len + 1, sizeof (char));

    if (buffer == NULL)
    {
      printf ("Fatal Error! Record: %d --- could not calloc buffer\n", recordNum);
      return false;
    }

    if (payload_len > fread (buffer, sizeof (char), payload_len, input))
    {
      printf ("Fatal Error! Record: %d ---  failed to read record data payload into buffer\n", recordNum);
      valid_payload = false;
      //if(options->treat_as_errors)
      //{
      return false;
      //}
    }

    //TODO validate payload using the buffer contains

    if (buffer)
    {
      free(buffer);
    }

    recordNum = recordNum + 1;
  } //End of buffer input for-loop

  //----Check Payload content via libmseed functions-----
  if (!options->skip_payload)
  {
    if (verbose > 2)
    {
      printf ("--- Started Data Payload Verification for %d records ---\n", recordNum);
    }

    valid_payload = check_payloads (options, input, 0, 0, file_name, recordNum, print_data, verbose);
    if (valid_payload)
    {
      if (verbose > 1)
        printf ("--- Payloads are valid! ---\n");
    }
    else
    {
      printf ("Error! --- Payloads are not valid!\n");
      if (options->treat_as_errors)
      {
        return false;
      }
      fail_count_file = fail_count_file + 1;
    }
  }
  else
  {
    if (verbose > 0)
    {
      printf("Payload validation skipped by user\n");
    }
  }

  if (verbose > 2)
  {
    printf ("--- Completed Data Payload Verification ---\n");
  }

  if (verbose > 1)
  {
    printf("Completed processing %d record(s)\n", recordNum);
    //TODO needs to be conditional (valid_file)
  }

  *records = recordNum;

  if (fail_count_rcd == 0 && fail_count_file == 0)
    return true;
  else
    return false;
}
