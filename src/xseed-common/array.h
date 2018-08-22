#ifndef __XSEED_COMMON_ARRAY_H__
#define __XSEED_COMMON_ARRAY_H__

//#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
//#define strnlen _strnlen
//#endif

int expand_array(void **ptr, int current_len, size_t obj_size);

int buffer_to_number(char *buffer, int len, char type, void *output);

#endif /* __XSEED_COMMON_ARRAY_H__ */
