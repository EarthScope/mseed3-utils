#include <xseed-common/config.h>
#include "cmd_opt.h"
#include <stdio.h>

void display_version(char *program_name, char *message, int major, int minor, int patch)
{
    printf("%s - %s\n", program_name, message);
    printf("Version: %d.%d.%d\n", major, minor, patch);

}