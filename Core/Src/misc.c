#include "misc.h"


uint8_t strstr(const char *str1, const char *str2) 
{
  while( (*str1 != '\0') || (*str2 != '\0')) {
    if (*str1 != *str2)
      return 0;
    ++str1;
    ++str2;
  }
  return 1;
}


uint16_t atoi(const char* buff, uint8_t size)
{
  uint16_t result = 0;
  for (uint8_t i = 0; i < size; i++)
    result = result*10 + buff[i] - '0';
  return result;
}


void reverse_str(char* buff, uint8_t size)
{
  uint8_t temp = 0;
  for (uint8_t i = 0; i < (size / 2); i++){
    temp = buff[i];
    buff[i] = buff[size - i - 1];
    buff[size - i - 1] = temp;
  }
}


void itoa(uint16_t integer, char* result)
{
  uint8_t i = 0;
  for(; integer != 0; i++, integer = integer / 10)
    result[i] = (integer % 10) + '0';
  reverse_str(result, i);
  result[i] = '\0';
}


uint32_t power(uint32_t x, uint32_t n){
    uint32_t returnValue = x;
    if(n==0)return 1;
    if(n==1)return x;
    while(--n){
        returnValue *= x;
    }
    return returnValue;
}


int32_t str_to_int(const char* str){
    const char* endStr = str;
    int32_t returnValue=0;
    uint8_t counter=0;
    uint8_t minus = 0;
    if(*endStr == '-'){
        endStr++;
        str++;
        minus = 1;
    }

    while((*endStr >= '0')&&(*endStr <= '9')){counter++; endStr++;}
    if(counter > 10) return 0xFFFFFFFF;
    while(counter--){
        returnValue += (*str-'0')*power(10, counter);
        str++;
    }
    if(minus) returnValue *= (-1);
    return returnValue;
}

int16_t rssi_to_dbm(uint8_t rssi)
{
  if (rssi == 99) return 0;
      
  if (rssi == 0) return -110;
  
  if (rssi == 1) return -108;
  
  if (rssi == 2) return -106;
  
  return (uint8_t)(2.1*rssi) - 111.3;
}

void memncpy(const char* from, char* to)
{
  for (uint8_t i = 0; from[i] != '\0'; i++)
    to[i] = from[i];
}


void add_to_end(const char* append, char* to)
{
  uint8_t i = 0;
  for (; to[i] != '\0'; i++);
  for (uint8_t j = 0; append[j] != '\0'; j++){
    to[i] = append[j];
    i++;
  }
}


uint8_t str2uint8(const char * str)
{
    uint8_t p = 0;
    uint8_t res = 0;

    while( (str[p] < '0') || (str[p] > '9') ) ++p;

    while( (str[p] >= '0') && (str[p] <= '9') )
        res = res * 10  + (str[p++] - '0');


    return res;
}


uint8_t strnstr(const char * str1, const char * str2, uint16_t n)
{
    uint16_t i = 0;

    if (!n)
    {
        while( (str1[i] != '\0') && (str2[i] != '\0') )
        {
            if (str1[i] != str2[i]) return 0;
            i += 1;
        }
        return 1;
    }

    while (i < n)
    {
        if ( (str1[i] == '\0') && (str2[i] == '\0') ) return 1;
        if (str1[i] == '\0') return 0;
        if (str2[i] == '\0') return 0;
        if (str1[i] != str2[i]) return 0;
        i += 1;
    }
    return 1;
}


uint8_t stradd(const char * strin, char* strout)
{
    uint16_t i = 0;
    uint16_t start = 0;

    while(strout[start++] != '\0');
    start -= 1;

    while(strin[i] != '\0')
    {
        strout[start + i] = strin[i];
        i += 1;
    }
    strout[start + i] = '\0';

    return 0;
}


uint8_t strcp(const char * strin, char* strout)
{
    uint8_t i = 0;
    uint8_t max_len = 0xFE;//sizeof(strout);
    while(strin[i] != '\0')
    {
        strout[i] = strin[i];
        i += 1;
        if (i >= max_len) return 1;
    }
    strout[i] = '\0';
    return 0;
}

uint8_t str_is_ip(const_string_t str)
{
    uint8_t p = 0;
    uint16_t num = 0;

    while(str[p] != '.')
    {
        if ( (str[p] >= '0') && (str[p] <= '9') )
            num = num * 10 + (str[p] - '0');
        else
            return 0;
        if (num > 255) return 0;
        p += 1;
    }

    str += p + 1;
    num = 0;
    p = 0;

    while(str[p] != '.')
    {
        if ( (str[p] >= '0') && (str[p] <= '9') )
            num = num * 10 + (str[p] - '0');
        else
            return 0;
        if (num > 255) return 0;
        p += 1;
    }

    str += p + 1;
    num = 0;
    p = 0;

    while(str[p] != '.')
    {
        if ( (str[p] >= '0') && (str[p] <= '9') )
            num = num * 10 + (str[p] - '0');
        else
            return 0;
        if (num > 255) return 0;
        p += 1;
    }

    str += p + 1;
    num = 0;
    p = 0;

    while(str[p] != '.' && str[p] != '\n' && str[p] != '\r')
    {
        if ( (str[p] >= '0') && (str[p] <= '9') )
            num = num * 10 + (str[p] - '0');
        else
            return 0;
        if (num > 255) return 0;
        p += 1;
    }

    return 1;
}


static const float fpows10[] = {100000, 10000, 1000, 100, 10, 1, 0.1, 0.01, 0.001, 0.0001, 0.00001};
int8_t float_to_str(float f, char* out)
{
    uint8_t p = 0;
    if (f > 999999)
    {
        out[p++] = '0';
        return p;
    }

    uint8_t pow = 0;

    // only XXX XXX. XXX XX

    float tmp;

    if (f < 0)
    {
        out[p++] = '-';
        f = -f;
    }

    if (f < 1)
    {
        out[p++] = '0';
        out[p++] = '.';

        pow = 6;
        while (pow < (sizeof(fpows10)/sizeof(float)))
        {
            uint8_t digit = f / fpows10[pow];
            out[p++] = digit + '0';
            f -= digit * fpows10[pow];
            pow += 1;
        }
        return p;
    }

    uint8_t num_of_digits = 6;
    uint8_t started = 0;
    while(num_of_digits)
    {
        uint8_t digit = f / fpows10[pow];

        if (digit) started = 1;

        if (started)
        {
            out[p++] = digit + '0';
            f -= digit * fpows10[pow];
            num_of_digits -= 1;
        }

        pow += 1;
        if (pow == 6) out[p++] = '.';
    }

    return p;
}


static const uint32_t pows10[] =
{
    1000000000,
    100000000,
    10000000,
    1000000,
    100000,
    10000,
    1000,
    100,
    10,
    1
};

int8_t uint32_to_str(uint32_t num, char* out)
{
    uint8_t c, i = 0, p = 0;

    if (!num)
    {
        out[p++] = 0 + '0';
        return p;
    }

    while (num < pows10[i]) i += 1;

    while (i < sizeof(pows10)/sizeof(uint32_t))
    {
        c = num / pows10[i];
        num -= c * pows10[i];
        out[p++] = c + '0';
        i += 1;
    }

    return p;
}

int8_t uint32withoffset_to_str(uint32_t num, uint8_t offset, char* out)
{
    uint8_t c, i = 0, p = 0;

    if (!num)
    {
        out[p++] = 0 + '0';
        out[p++] = '.';
        out[p++] = 0 + '0';
        return p;
    }


    while (num < pows10[i]) i += 1;

    while (i < 10)
    {
        c = num / pows10[i];
        num -= c * pows10[i];
        out[p++] = c + '0';
        if (i == 9 - offset)
            out[p++] = '.';
        i += 1;
    }

    return p;
}

uint16_t spec_strlen(const_string_t str)
{
    uint16_t i = 0;
    while( (str[i] != '\0') && (str[i] != '\"') )
        i += 1;
    return i;
}

uint8_t spec_strcp(const_string_t strin, char* strout)
{
    uint16_t i = 0;
    while( (strin[i] != '\0') && (strin[i] != '\"') )
    {
        strout[i] = strin[i];
        i += 1;
    }
    strout[i] = '\0';
    return 0;
}

uint16_t strlen(const_string_t str)
{
    uint16_t i = 0;
    while(str[i] != '\0')
        i += 1;
    return i;
}

uint8_t strcp_safe(const_string_t strin, char* strout, uint16_t max_size)
{
    uint16_t i = 0;

    while(strin[i] != '\0')
    {
        strout[i] = strin[i];
        i += 1;
        if (i + 1 >= max_size) return 1;
    }

    strout[i] = '\0';

    return 0;
}

int16_t strfindc(const_string_t str, char c)
{
    uint16_t i = 0;
    while(str[i] != '\0')
    {
        if (str[i] == c)
            return i;
        i += 1;
    }
    return -1;
}

int32_t str2int32(const_string_t str)
{
    uint8_t p = 0;
    int32_t res = 0;

    while( (str[p] < '0') || (str[p] > '9') || (str[p] == '-') ) ++p;

    if (str[p] == '-')
    {
        p += 1;
        while( (str[p] >= '0') && (str[p] <= '9') )
            res = res * 10  - (str[p++] - '0');
    }
    else
    {
        while( (str[p] >= '0') && (str[p] <= '9') )
            res = res * 10  + (str[p++] - '0');
    }

    return res;
}


uint16_t str2uint16(const_string_t str)
{
    uint8_t p = 0;
    uint16_t res = 0;

    while( (str[p] < '0') || (str[p] > '9') ) ++p;

    while( (str[p] >= '0') && (str[p] <= '9') )
        res = res * 10  + (str[p++] - '0');

    return res;
}

int8_t str2int8(const_string_t str)
{
    uint8_t p = 0;
    int8_t res = 0;

    while( (str[p] < '0') || (str[p] > '9') || (str[p] == '-') ) ++p;

    if (str[p] == '-')
    {
        p += 1;
        while( (str[p] >= '0') && (str[p] <= '9') )
            res = res * 10  - (str[p++] - '0');
    }
    else
    {
        while( (str[p] >= '0') && (str[p] <= '9') )
            res = res * 10  + (str[p++] - '0');
    }

    return res;
}

uint16_t i = 0;
void add_back_slashes_to_quotes(char * str)
{
  //uint16_t i = 0;
  char temp_prev, temp_curr;
  while(str[i] != '\0'){
    if (str[i] == '\"'){
      uint16_t j = i + 1;
      temp_prev = str[i];
      while (str[j] != '\0'){
        temp_curr = str[j];
        str[j] = temp_prev;
        temp_prev = temp_curr;
        j++;
      }
      str[i] = '\\';
      str[j++] = '}';
      str[j] = '\0';
      i = i + 1;
    }
  i++;
  }
  i;
}