#include <string.h>
#include <stdio.h>

char *
mseed3_get_dirname (char *path)
{
  char *last;
  char *parent;
  //TODO need case for WIN
  last   = strrchr (path, '/');
  parent = strndup (path, strlen (path) - (strlen (last)));

  return parent;
}
