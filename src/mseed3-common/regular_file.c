#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef S_IFDIR
#define S_IFDIR 0040000
#endif

bool
mseed3_regular_file (char *pathname)
{
  struct stat info;
  if (stat (pathname, &info) != 0)
  {
    printf ("cannot access %s\n", pathname);
    return false;
  }
  else if (info.st_mode & S_IFDIR)
  {
    return false;
  }
  else
  {
    return true;
  }
}
