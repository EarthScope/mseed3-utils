#include <stdio.h>
#include "constants.h"


long
mseed3_file_length (FILE *file)
{
  if (NULL == file)
  {
    return 0;
  }

  long current_pos = ftell (file);

  if (0 > fseek (file, 0L, SEEK_END))
  {
    return MSEED3_SEEK_ERROR;
  }

  long file_size = ftell (file);

  if (0 > fseek (file, current_pos, SEEK_SET))
  {
    return MSEED3_SEEK_ERROR;
  }

  return file_size;
}
