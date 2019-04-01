#include <stdlib.h>

#include "array.h"
#include "constants.h"

int
expand_array (void **ptr, int current_len, size_t obj_size)
{
  int ret_len;

  if (0 == current_len)
  {
    ret_len = 1;
  }
  else
  {
    ret_len = (int)(1.5f * (double)current_len);
  }

  if (ret_len == current_len)
  {
    ret_len++;
  }

  *ptr = realloc (*ptr, ret_len * obj_size);
  if (NULL == *ptr)
  {
    return XSEED_MALLOC_ERROR;
  }
  return ret_len;
}
