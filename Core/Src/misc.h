#include "stdint.h"

#define const_string_t const char *


uint8_t strstr(const char *str1, const char *str2);
//uint16_t atoi(const char *buff, uint8_t size);
void itoa(uint16_t integer, char* result);
int32_t str_to_int(const char* str);
int16_t rssi_to_dbm(uint8_t rssi);
void memncpy(const char* from, char* to);
void add_to_end(const char* append, char* to);
uint8_t str2uint8(const char * str);
uint8_t strnstr(const char * str1, const char * str2, uint16_t n);
uint8_t stradd(const char * strin, char* strout);
uint8_t strcp(const char * strin, char* strout);



uint8_t strfind(const_string_t str1, const_string_t str2);
int16_t strfindc(const_string_t str, char c);
uint16_t strlen(const_string_t str);
uint16_t spec_strlen(const_string_t str);
uint8_t spec_strcp(const_string_t strin, char* strout);
uint8_t strcp_safe(const_string_t strin, char* strout, uint16_t max_size);

int32_t str2int32(const_string_t str);
uint16_t str2uint16(const_string_t str);
int8_t  str2int8(const_string_t str);

int8_t uint32withoffset_to_str(uint32_t num, uint8_t offset, char* out);
int8_t float_to_str(float f, char* out);
uint8_t str_is_ip(const_string_t str);
void add_back_slashes_to_quotes(char * str);
