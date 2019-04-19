#include "validator.h"
#include "warnings.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*! @brief Read identifier length into buffer
 *
 *  @param[in] options -W cmd line warn options (currently not implemented)
 *  @param[in] input file pointer to xSEED file
 *  @param[in] identifier_len
 *
 */
bool
check_identifier (struct extra_options_s *options, FILE *input, uint8_t identifier_len,
                  uint32_t recordNum, uint8_t verbose)
{
  bool output  = true;
  char *buffer = (char *)calloc (identifier_len + 1, sizeof (char));

  if (identifier_len > fread (buffer, sizeof (char), identifier_len, input))
  {
    printf ("Fatal Error: EOF reached reading identifier_len into buffer, please double check input record\n");
    free (buffer);
    output = false;
    return output;
  }

  if (verbose > 2)
    printf ("Record: %d --- Checking source identifier URN: %s\n", recordNum, buffer);

  //TODO test value

  if (buffer)
  {
    free (buffer);
  }
  return output;
}
