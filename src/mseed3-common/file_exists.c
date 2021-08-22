#include <stdbool.h>
#include <stdio.h>

bool
mseed3_file_exists (char *pathname)
{
  FILE *file = NULL;
  if ((file = fopen (pathname, "r")))
  {
    fclose (file);
    return true;
  }
  return false;
}
