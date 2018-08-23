#include <stdint.h>
#include "constants.h"

int buffer_to_number(char *buffer, int len, char type, void *output)
{

    uint16_t temp_int16 = buffer[0] | buffer[1] << 8;
    uint32_t temp_int32 = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
    union
    {
        uint32_t i;
        float f;
    } u1;
    union
    {
        uint64_t i;
        double d;
    } u2;
    switch (type)
    {
        case XSEED_TEXT:
            *((char *) output) = buffer[1];
            break;
        case XSEED_UINT16:
            *((uint16_t *) output) = temp_int16;
            break;
        case XSEED_UINT32:
            *((uint32_t *) output) = temp_int32;
            break;
        case XSEED_FLOAT:
            u1.i = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
            *((float *) output) = u1.f;
            break;
        case XSEED_DOUBLE:
            u2.i = buffer[0] | (uint64_t) buffer[1] << 8 | (uint64_t) buffer[2] << 16 | (uint64_t) buffer[3] << 24 |
                   (uint64_t) buffer[4] << 32 | (uint64_t) buffer[5] << 40 | (uint64_t) buffer[6] << 48 |
                   (uint64_t) buffer[7] << 56;
            *((double *) output) = u2.d;
            break;
        case XSEED_STEIM1:
        case XSEED_STEIM2:
        case XSEED_STEIM3:
        case XSEED_COMPRESS_INT32:
        case XSEED_COMPRESS_FLOAT:
        case XSEED_COMPRESS_DOUBLE:
        case XSEED_OPAQUE:
            break;
        default:
            return XSEED_INVALID_DATATYPE;
    }
    return XSEED_SUCCESS;
}
