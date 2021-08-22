#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char * mseed3_cat_strings(char *str1,char* str2)
{
      char * cat_str = (char *) malloc(strlen(str1)+ strlen(str2) + 1);
      strcpy(cat_str, str1);
      strcat(cat_str, str2);
      
      return cat_str;
}
