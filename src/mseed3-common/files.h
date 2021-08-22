#ifndef __MSEED3_COMMON_FILES_H__
#define __MSEED3_COMMON_FILES_H__

#include <stdbool.h>

bool mseed3_file_exists(char *pathname);

bool mseed3_regular_file(char *pathname);

long mseed3_file_length(FILE *file);

char * mseed3_get_dirname(char* path);

char * mseed3_cat_strings(char *str1,char* str2);

#endif /* __MSEED3_COMMON_FILES_H__ */
