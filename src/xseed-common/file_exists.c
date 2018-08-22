#include <stdio.h>
#include <stdbool.h>

bool xseed_file_exists(char *pathname)
{
    FILE *file = NULL;
    if ((file = fopen(pathname, "r")))
    {
        fclose(file);
        return true;
    }
    return false;
}
