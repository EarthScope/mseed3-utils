#include <stdio.h>

long xseed_file_length(FILE *file)
{
    if (NULL == file)
    {
        return 0;
    }

    long current_pos = ftell(file);

    if (0 > fseek(file, 0L, SEEK_END))
    {
        return -1;//TODO should be an error from constants
    }

    long file_size = ftell(file);

    if (0 > fseek(file, current_pos, SEEK_SET))
    {
        return -1;//TODO should be an error from constants
    }


    return file_size;
}

