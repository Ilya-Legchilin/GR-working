#ifndef MD5_H
#define MD5_H

#include "misc.h"

#define const_string_t const char *

//WORK ONLY WITH SHORT SEQUENCES < 50 characters
void count_md5(const_string_t strin, char* strout);

#endif // MD5_H