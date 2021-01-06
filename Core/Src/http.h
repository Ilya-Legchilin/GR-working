#ifndef HTTP_H
#define HTTP_H

#include "main.h"
#include "sim7020.h"
#include "sim7020_phy.h"

typedef struct{
  char host[32];
  char path[32];
  char api_key[64];
  uint32_t http_status;
} http_settings_t;

typedef enum{
  HTTP_OK,
  HTTP_ERROR
} HTTP_StatusTypeDef;


#define __HTTP_SET_STATUS(ST) do{ http->http_status |= (ST); }while(0)
#define __HTTP_GET_STATUS(ST) ((http->http_status & (ST)) == (ST))
#define __HTTP_CLEAR_STATUS(ST) do{ http->http_status &= ~(ST); }while(0) 


enum {
  HTTP_HOST_CREATED = 0x00000001,
  HTTP_CONNECTED    = 0x00000002,
  HTTP_INITIALIZED  = 0x00000004,
};


extern char content_http[256];

HTTP_StatusTypeDef HTTP_Init(http_settings_t* ptr);
HTTP_StatusTypeDef HTTP_Send(const char* message);
uint32_t HTTP_Get_UTC(void);
HTTP_StatusTypeDef HTTP_Deinit(void);
#endif