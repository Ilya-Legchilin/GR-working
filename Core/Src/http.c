#include "http.h"
#include "uart_simple_protocol.h"

static http_settings_t* http;
char begin_http[256] = "AT+CHTTPSEND=0,0,\"";
char content_http[256];

/**
  * @brief Function that intializes http module
  * @param pointer to a struct with http settings from main
  * @retval Success code
  */
HTTP_StatusTypeDef HTTP_Init(http_settings_t* ptr)
{
  http = ptr;
  while(__HTTP_GET_STATUS(HTTP_HOST_CREATED) == 0){
    if (Sim7020_Check_Created_HTTP_Host() == SIM7020_OK)
      __HTTP_SET_STATUS(HTTP_HOST_CREATED);
    else{
      Sim7020_Set_Host(http->host);
      //osDelay(1000);
    }
  }
  while(__HTTP_GET_STATUS(HTTP_CONNECTED) == 0){
    if (Sim7020_Check_HTTP_Connected() == SIM7020_OK)
      __HTTP_SET_STATUS(HTTP_CONNECTED);
    else {
      Sim7020_Connect();
      //osDelay(1000);
    }
  }
  __HTTP_SET_STATUS(HTTP_INITIALIZED);
  UART_SendString("Connected successfully!!!");
  //add_to_end(http->path, content_http);
  //add_to_end(http->api_key, content_http);
  return HTTP_OK;
}

/**
  * @brief Function that deinitialize http module
  * @param None
  * @retval Success code
  */
HTTP_StatusTypeDef HTTP_Deinit()
{
  while (__HTTP_GET_STATUS(HTTP_CONNECTED) == 1){
    if (Sim7020_Check_HTTP_Disconnected() == SIM7020_OK)
      __HTTP_CLEAR_STATUS(HTTP_CONNECTED);
    else {
      Sim7020_Disconnect();
      //osDelay(1000);
    }
  }
  while (__HTTP_GET_STATUS(HTTP_HOST_CREATED) == 1){
    if (Sim7020_Check_Destroyed_HTTP_Host() == SIM7020_OK)
      __HTTP_CLEAR_STATUS(HTTP_HOST_CREATED);
    else {
      Sim7020_Destroy();
      //osDelay(1000);
    }
  }
  __HTTP_CLEAR_STATUS(HTTP_INITIALIZED);
  http = 0;
  UART_SendString("Disconnected successfully!!!");
  return HTTP_OK;
}

/**
  * @brief Send a messsage to host
  * @param Message
  * @retval Success code
  */
HTTP_StatusTypeDef HTTP_Send(const char* message)
{
  Sim7020_Send(message);
  return HTTP_OK;
}