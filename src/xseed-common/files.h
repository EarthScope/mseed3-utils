#ifndef __XSEED_COMMON_FILES_H__
#define __XSEED_COMMON_FILES_H__

#include <stdbool.h>

bool xseed_file_exists(char *pathname);

bool xseed_regular_file(char *pathname);

long xseed_file_length(FILE *file);

#endif /* __XSEED_COMMON_FILES_H__ */
