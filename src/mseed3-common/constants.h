#ifndef __MSEED3_CONSTANTS_H__
#define __MSEED3_CONSTANTS_H__

enum error_codes_e
{
    MSEED3_SEEK_ERROR = -1,
    MSEED3_BAD_INPUT = -2,
    MSEED3_MALLOC_ERROR = -3
};

enum data_encodings_e
{
    MSEED3_TEXT = 0,
    MSEED3_UINT16 = 1,
    MSEED3_UINT32 = 3,
    MSEED3_FLOAT = 4,
    MSEED3_DOUBLE = 5,
    MSEED3_STEIM1 = 10,
    MSEED3_STEIM2 = 11,
    MSEED3_STEIM3 = 19,
    MSEED3_OPAQUE = 100
};

#endif /* __MSEED3_CONSTANTS_H__ */
