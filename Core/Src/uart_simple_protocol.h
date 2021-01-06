#ifndef UART_SIMPLE_PROTOCOL_H
#define UART_SIMPLE_PROTOCOL_H


#include "sim7020_phy.h"
#include "main.h"
#include "stm32l0xx_hal_uart.h"
#include "http.h"
#include "sim7020.h"
#include "schedule.h"
#include "wtimer.h"

#define USP_RX_BUFF_LEN 1024
#define NORMING_TO_USP_RX_BUFF_LEN(x) (x & 0x3FF)
#define USP_TX_BUFF_LEN 1024
#define NORMING_TO_USP_TX_BUFF_LEN(x) (x & 0x3FF)
#define USP_COMMAND_LEN 1024
#define USP_OPTION_LEN 1024
#define USP_BP_LEN 1024
#define USP_CALLBACK_LEN 1024


typedef enum {
  USP_OK = 0, 
  USP_ERROR,
  USP_NO_USP_COMMAND,
  USP_BUFF_OVERFLOW
} USP_StatusTypeDef;

void StartUSPTask(struct wtimer_desc *desc);
USP_StatusTypeDef USP_Init(UART_HandleTypeDef *screen_huart);
USP_StatusTypeDef USP_Deinit();
USP_StatusTypeDef USP_Enable();
USP_StatusTypeDef UART_SendString(const char* str);
void USP_Rx_Callback(uint16_t pointer);
//void HTTP_Rx_Callback(const char* str);

#endif //UART_SIMPLE_PROTOCOL_H
