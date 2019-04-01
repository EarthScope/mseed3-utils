#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <xseed-common/array.h>
#include <xseed-common/constants.h>

#include <libmseed.h>

#include "validator.h"
#include "warnings.h"

#define XSEED_STATIC_HEADER_LEN 40

/*!

@brief Validates a xSEED Version 3 Header

 Specification:
A record is composed of a header followed by a time series data payload. The byte order of binary
fields in the header must be least significant byte first (little endian).
The total length of a record is variable and is the sum of 40 (length of fixed section of header),
field 10 (length of identifier), field 11 (length of extra headers), field 12 (length of payload).
Field  Field name                            Type       Length    Offset  Content
1      Record header indicator (‘MS’)        CHAR       2         0       ASCII ‘MS’
2      Format version (3)                    UINT8      1         2
3      Flags                                 UINT8      1         3
       Record start time
4a       Nanosecond (0 - 999999999)          UINT32     4         4
4b       Year (0 - 65535)                    UINT16     2         8
4c       Day-of-year  (1 - 366)              UINT16     2         10
4d       Hour (0 - 23)                       UINT8      1         12
4e       Minute (0 - 59)                     UINT8      1         13
4f       Second (0 - 60)                     UINT8      1         14
5      Data encoding format                  UINT8      1         15
6      Sample rate/period                    FLOAT64    8         16
7      Number of samples                     UINT32     4         24
8      CRC of the record                     UINT32     4         28
9      Data publication version              UINT8      1         32
10     Length of identifier in bytes         UINT8      1         33
11     Length of extra headers in bytes      UINT16     2         34
12     Length of data payload in bytes       UINT32     2         36
13     Time series identifier                CHAR       V         40      URN identifier
14     Extra header fields                   JSON       V         40 + field 10
15     Data payload                          encoded    V         40 + field 10 + field 11
All length values are specified in bytes, which are assumed to be 8-bits in length
​*/

static bool parse_header (struct warn_options_s *options, char *buffer, uint8_t *identifier_len,
                          uint16_t *extra_header_len, uint32_t *payload_len, uint8_t *payload_fmt,
                          uint32_t recordNum, uint8_t verbose);

/*! @brief main validate header routine
 *
 *  @param[in] options -W cmd line warn options (currently not implemented)
 *  @param[in] input file pointer to xSEED file
 *  @param[in] file_len overall binary length
 *  @param[in,out] file_pos pointer to current position in the file
 *  @param[out] identifier_len length of identifier
 *  @param[out] extra_header_len length of extra headers
 *  @param[out] payload_len length of payload
 *  @param[out] payload_fmt format of payload
 *  @param[in] recordNum record currently being processed
 *  @param[in] verbose verbosity level
 *
 *
 *  @todo test big endian case, having two fns was clunky, converted to not using non-conventional bit shifting
 */

bool
check_header (struct warn_options_s *options, FILE *input_file, long file_len, long *file_pos,
              uint8_t *identifier_len, uint16_t *extra_header_len, uint32_t *payload_len,
              uint8_t *payload_fmt, uint32_t recordNum, int8_t verbose)
{

  bool header_valid;
  char buffer[XSEED_STATIC_HEADER_LEN];

  if (XSEED_STATIC_HEADER_LEN != fread ((void *)buffer, sizeof (char), XSEED_STATIC_HEADER_LEN, input_file))
  {
    printf ("Fatal Error! Record: %d --- File size mismatch, check input record\n", recordNum);
    header_valid = false;
    return header_valid;
  }

  if (ms_bigendianhost())
  {
    if (verbose > 2)
      printf ("host is Big Endian, *Wanring* untested\n");
  }
  else
  {
    if (verbose > 2)
      printf ("host is Little Endian\n");
  }

  header_valid = parse_header (options, buffer, identifier_len, extra_header_len,
                               payload_len, payload_fmt, recordNum, verbose);

  return header_valid;
}

/*! @brief Parse fixed header information from an input buffer
 *
 *  @param[in] options -W cmd line warn options (currently not implemented)
 *  @param[in] buffer current buffer content
 *  @param[out] identifier_len length of identifier
 *  @param[out] extra_header_len length of extra headers
 *  @param[out] payload_len length of payload
 *  @param[out] payload_fmt format of payload
 *  @param[in] recordNum record currently being processed
 *  @param[in] verbose verbosity level
 *
 */

bool
parse_header (struct warn_options_s *options, char *buffer, uint8_t *identifier_len, uint16_t *extra_header_len,
              uint32_t *payload_len, uint8_t *payload_fmt, uint32_t recordNum, uint8_t verbose)
{
  char h_flag[3];
  h_flag[0] = (char)buffer[0];
  h_flag[1] = (char)buffer[1];

  bool header_valid = true;

  if (verbose > 1)
    printf ("Record: %d --- Checking Header Flag value: %c%c\n", recordNum, h_flag[0], h_flag[1]);

  if (!(h_flag[0] == 'M' && h_flag[1] == 'S'))
  {
    printf ("Error! Record: %d --- Header Flag Value Incorrect ('MS' is only valid flag)\n", recordNum);
    header_valid = false;
    if (options->treat_as_errors)
    {
      return header_valid;
    }
  }

  //---Check MS version---
  uint8_t msVersion = (uint8_t)buffer[2];
  if (verbose > 1)
    printf ("Record: %d --- Checking File Version value: %d\n", recordNum, msVersion);

  if (3 != msVersion)
  {
    printf ("Error! Record: %d --- Header Version Value Incorrect ('3' currently is the only supported MS version)\n",
            recordNum);
    header_valid = false;
    if (options->treat_as_errors)
    {
      return header_valid;
    }
  }

  //---Check valid year---
  //uint16_t year = (uint8_t )buffer[8] | (uint8_t)buffer[9] << 8;
  uint16_t year = (uint8_t)buffer[8] + ((uint8_t)buffer[9] * (0xFF + 1));
  if (verbose > 1)
    printf ("Record: %d --- Checking Year value: %d\n", recordNum, year);

  if (year < 0 || year > 65535)
  {
    printf ("Error! Record: %d --- Year value out of range (0-65535)\n", recordNum);
    header_valid = false;
    if (options->treat_as_errors)
    {
      return header_valid;
    }
  }

  //TODO Warn for data in future

  //---Check valid Day-of-Year---
  //uint16_t doy = (uint8_t )buffer[10] | (uint8_t)buffer[11] << 8;
  uint16_t doy = (uint8_t)buffer[10] + ((uint8_t)buffer[11] * (0xFF + 1));
  if (verbose > 1)
    printf ("Record: %d --- Checking Day of Year value: %d\n", recordNum, doy);

  if (366 < doy || 1 > doy)
  {

    printf ("Error! Record: %d --- Day Of Year value out of range (1-366)\n", recordNum);
    header_valid = false;
    if (options->treat_as_errors)
    {
      return header_valid;
    }
  }

  //---Check valid hour range---
  uint8_t hours = (uint8_t)buffer[12];
  if (verbose > 1)
    printf ("Record: %d --- Checking Hours value: %d\n", recordNum, hours);

  if (hours < 0 || hours > 23)
  {
    printf ("Error! Record: %d --- Hours value out of range (0-23)\n", recordNum);
    header_valid = false;
    if (options->treat_as_errors)
    {
      return header_valid;
    }
  }

  //---Check valid min range---
  uint8_t mins = (uint8_t)buffer[13];
  if (verbose > 1)
    printf ("Record: %d --- Checking Mins value: %d\n", recordNum, mins);

  if (mins < 0 || mins > 59)
  {
    printf ("Error! Record: %d --- Mins value out of range (0-59)\n", recordNum);
    header_valid = false;
    if (options->treat_as_errors)
    {
      return header_valid;
    }
  }

  //---Check valid seconds range---
  uint8_t secs = (uint8_t)buffer[14];
  if (verbose > 1)
    printf ("Record: %d --- Checking Secs value: %d\n", recordNum, secs);

  if (secs < 0 || secs > 60)
  {
    printf ("Error! Record: %d --- Secs value out of range (1-366)\n", recordNum);
    header_valid = false;
    if (options->treat_as_errors)
    {
      return header_valid;
    }
  }

  //---Check valid nanoseconds---
  uint32_t nanoseconds =
      (uint8_t)buffer[4] + ((uint8_t)buffer[5] * (0xFF + 1)) + ((uint8_t)buffer[6] * (0xFFFF + 1)) +
      ((uint8_t)buffer[7] * (0xFFFFFF + 1));
  //uint32_t nanoseconds = ((uint8_t)buffer[4] | ((uint8_t)buffer[5] << 8) | ((uint8_t)buffer[6] << 16) | ((uint8_t)buffer[7] << 24));
  if (verbose > 1)
    printf ("Record: %d --- Checking Nanoseconds value: %d\n", recordNum, nanoseconds);

  if (999999999 < nanoseconds)
  {
    printf ("\"Error! Record: %d --- nanoseconds out of range\n", recordNum);
    header_valid = false;
    if (options->treat_as_errors)
    {
      return header_valid;
    }
  }

  //---Check for Payload type---
  uint8_t payload = (uint8_t)buffer[15];
  *payload_fmt    = payload;
  if (verbose > 1)
  {
    printf ("Record: %d --- Checking Payload Flag: %d\n", recordNum, payload);
    printf ("Record: %d --- Payload Type: ", recordNum);
  }

  switch (payload)
  {
  case 0:
    if (verbose > 1)
      printf ("Payload flag indicates TEXT payload\n");
    break;
  case 1: /* 16-bit, integer, little-endian*/
    if (verbose > 1)
      printf ("Payload flag indicates 16-bit, integer, little-endian payload\n");
    break;
  case 3: /* 32-bit, integer, little-endian*/
    if (verbose > 1)
      printf ("Payload flag indicates 32-bit, integer, little-endian payload\n");
    break;
  case 4: /* IEEE 32-bit floats, little-endian */
    if (verbose > 1)
      printf ("Payload flag indicates IEEE 32-bit floats, little-endian payload\n");
    break;
  case 5: /* IEEE 64-bit floats (double), little-endian */
    if (verbose > 1)
      printf ("Payload flag indicates IEEE 64-bit floats (double), little-endian payload\n");
    break;
  case 10: /* Steim-1 integer compressin, big-endian */
    if (verbose > 1)
      printf ("Steim-1 integer compression, big-endian\n");
    break;
  case 11: /* Steim-2 integer compressin, big-endian */
    if (verbose > 1)
      printf ("Steim-2 integer compression, big-endian\n");
    break;
  case 14: /* Steim-2 integer compressin, big-endian */
    if (verbose > 1)
      printf ("Steim-2 integer compression, big-endian\n");
    break;
    //TODO verify this with chad
  case 16: /* GEOSCOPE Muxed 16/4 bit gain/exp ,little endian(?) */
    if (verbose > 1)
      printf ("GEOSCOPE Muxed 16/4 bit gain/exp, little endian(?)\n");
    break;
  case 19: /* Steim-3 integer compressin, big-endian */
    if (verbose > 1)
      printf ("Steim-3 integer compression, big-endian\n");
    break;
    //TODO verify this with chad
  case 30: /* SRO Gain Ranged Format, ?-endian */
    if (verbose > 1)
      printf ("SRO Gain Ranged Format, ?-endian\n");
    break;
    //TODO verify this with chad
  case 32: /* DWWSSN Format, ?-endian */
    if (verbose > 1)
      printf ("DWWSSN Format, ?-endian\n");
    break;
  case 53: /* 32-bit integer, little-endian, general compressor */
    if (verbose > 1)
      printf ("32-bit integer, little-endian, general compressor\n");
    break;
  case 54: /* 32-bit IEEE floats, little-endian, general compressor */
    if (verbose > 1)
      printf ("32-bit IEEE floats, little-endian, general compressor\n");
    break;
  case 55: /* 64-bit IEEE floats, little-endian, general compressor */
    if (verbose > 1)
      printf ("64-bit IEEE floats, little-endian, general compressor\n");
    break;
  default: /* invalid payload type */
    printf ("Error! Record: %d --- Payload Type Flag is Invalid!\n", recordNum);
    header_valid = false;
    if (options->treat_as_errors)
    {
      return header_valid;
    }
    break;
  };

  //Get Sample Rate
  double sample_rate;
  //TODO convenience function for parsing buffer, in progress
  //buffer_to_number(buffer+16, sizeof(double), XSEED_DOUBLE, &sample_rate);
  //TODO need check for valid sample rate
  sample_rate = *((double *)((uint8_t *)buffer + 16));
  if (sample_rate < 0)
  {
    sample_rate = sample_rate * (-.01); //TODO ?????
  }

  if (verbose > 1)
    printf ("Record: %d --- Checking sample rate value: %f\n", recordNum, sample_rate);

  //Get Number of Samples
  //TODO need check for valid number_samples
  uint32_t number_samples =
      (uint8_t)buffer[24] + ((uint8_t)buffer[25] * (0xFF + 1)) + ((uint8_t)buffer[26] * (0xFFFF + 1)) +
      ((uint8_t)buffer[27] * (0xFFFFFF + 1));
  //uint32_t number_samples = ((uint8_t)buffer[24] | ((uint8_t)buffer[25] << 8) | ((uint8_t)buffer[26] << 16) | ((uint8_t)buffer[27] << 24));

  if (verbose > 1)
    printf ("Record: %d --- Checking number of samples value: %d\n", recordNum, number_samples);

  //Get CRC Value
  uint32_t CRC = (uint8_t)buffer[28] + ((uint8_t)buffer[29] * (0xFF + 1)) + ((uint8_t)buffer[30] * (0xFFFF + 1)) +
                 ((uint8_t)buffer[31] * (0xFFFFFF + 1));
  //uint32_t CRC = ((uint8_t)buffer[28]) | ((uint8_t)buffer[29] << 8) | ((uint8_t)buffer[30] << 16) | ((uint8_t)buffer[31]) << 24 ;

  if (verbose > 1)
    printf ("Record: %d --- CRC value: 0x%0X\n", recordNum, CRC);

  //Get dataPubVersion
  //TODO Check for valid dataPubVersion
  uint8_t dataPubVersion = (uint8_t)buffer[32];
  if (verbose > 1)
    printf ("Record: %d --- Data Publication Version value: %d\n", recordNum, dataPubVersion);

  uint8_t identifier_l = (uint8_t)buffer[33];
  if (verbose > 1)
    printf ("Record: %d --- Identifier Length value: %d\n", recordNum, identifier_l);

  //Get lengths for extra header and payload
  uint16_t extra_header_l = (uint8_t)buffer[34] + ((uint8_t)buffer[35] * (0xFF + 1));
  //uint16_t extra_header_l = (uint8_t )buffer[34] | (uint8_t)buffer[35] << 8;
  if (verbose > 1)
    printf ("Record: %d --- Extra Header Length value: %d\n", recordNum, extra_header_l);
  //uint32_t payload_l = ((uint8_t)buffer[36] | ((uint8_t)buffer[37] << 8) | ((uint8_t)buffer[38] << 16) | ((uint8_t)buffer[39] << 24));
  uint32_t payload_l =
      (uint8_t)buffer[36] + ((uint8_t)buffer[37] * (0xFF + 1)) + ((uint8_t)buffer[38] * (0xFFFF + 1)) +
      ((uint8_t)buffer[39] * (0xFFFFFF + 1));
  if (verbose > 1)
    printf ("Record: %d --- Payload Length value: %d\n", recordNum, payload_l);

  //assign to output values
  *payload_fmt      = payload;
  *extra_header_len = extra_header_l;
  *payload_len      = payload_l;
  *identifier_len   = identifier_l;

  return header_valid;
  //fflush(stdout);
}
