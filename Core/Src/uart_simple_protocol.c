#include "uart_simple_protocol.h"
#include "schedule.h"
#include "wtimer.h"


#define SET_PIN(x) HAL_GPIO_WritePin(x.port, x.pin, GPIO_PIN_SET)
#define RESET_PIN(x) HAL_GPIO_WritePin(x.port, x.pin, GPIO_PIN_RESET)
#define READ_PIN(x) HAL_GPIO_ReadPin(x.port, x.pin)

typedef struct {
  GPIO_TypeDef* port;
  uint16_t pin;
} DigOut;

static DigOut led;
static uint8_t usp_enabled;
static uint8_t  usp_rx_buff[USP_RX_BUFF_LEN + 1];
static uint16_t usp_rx_buff_pointer = 0;
static uint8_t  usp_tx_buff[USP_TX_BUFF_LEN + 1];
static uint16_t usp_tx_buff_pointer = 0;  
static UART_HandleTypeDef *screen_huart = NULL;
static char bp_tx_buffer[USP_BP_LEN];
static uint8_t uart_rx_buff[USP_RX_BUFF_LEN + 1];
char option[USP_OPTION_LEN];
char command[USP_COMMAND_LEN];
uint8_t error_screen = 0;
uint16_t old_len;
uint16_t new_len;

USP_StatusTypeDef ECHO_Send(const char* str);
USP_StatusTypeDef USP_Disable();
USP_StatusTypeDef LED_Handle(const char* str);
USP_StatusTypeDef BYPASS_Send(const char* str);
USP_StatusTypeDef LED_Handler(const char* str);
USP_StatusTypeDef UART_SendString(const char* str);
void StartUSPTask(struct wtimer_desc *desc);
USP_StatusTypeDef UART_SendString(const char* str);

/**
  * @brief Comapares two stings
  * @param String1 and String2
  * @retval 0 - different, 1 - equal
  */
static uint8_t strstr(const char *str1, const char *str2) 
{
  while( (*str1 != '\0') && (*str2 != '\0')) {
    if (*str1 != *str2)
      return 0;
    ++str1;
    ++str2;
  }
  return 1;
}

/**
  * @brief Parses massage that comes from the screen
  * @param Strings option and command and pointer to a part of a buffer where 
  * @param the message to be handled is placed now
  * @retval Success code, No command code or overflow code
  */
USP_StatusTypeDef USP_Parse_String(char* option, char* command, 
                                   uint16_t pointer) 
{      
  uint16_t i;
  for(i = 0; usp_rx_buff[pointer] != ':'; ++i) {
    if (i >= USP_OPTION_LEN) 
      return USP_BUFF_OVERFLOW;
    option[i] = usp_rx_buff[pointer];
    pointer = NORMING_TO_USP_RX_BUFF_LEN(pointer + 1);
    if (i >= USP_RX_BUFF_LEN)
      return USP_NO_USP_COMMAND;
  }
  if (i >= USP_OPTION_LEN) 
      return USP_BUFF_OVERFLOW;
  option[i] = '\0';
  pointer = NORMING_TO_USP_RX_BUFF_LEN(pointer + 1); //ingore ':'
  for(i = 0; usp_rx_buff[pointer] != '\0'; ++i) {
    if (i >= USP_COMMAND_LEN) 
      return USP_BUFF_OVERFLOW;
    command[i] = usp_rx_buff[pointer];
    pointer = NORMING_TO_USP_RX_BUFF_LEN(pointer + 1);
  }
  if (i >= USP_COMMAND_LEN) 
      return USP_BUFF_OVERFLOW;
  command[i] = '\0';
  return USP_OK;
} 

/**
  * @brief Call echo, bypass or led depending on pointer
  * @param Pointer to a part of receive buffer where is message to be handled now
  * @retval Success code
  */
USP_StatusTypeDef USP_MessageHandler(uint16_t pointer)
{    
  USP_StatusTypeDef res = USP_Parse_String(option, command, pointer);
  if (!res) {
    if (strstr(option, "echo")) {
      UART_SendString(command);
    } else if (strstr(option, "bp")) {
      BYPASS_Send(command);
    } else if (strstr(option, "led")) {
      LED_Handler(command);
    }
  } else {
    return res;
  }
  return USP_OK;
}

/**
  * @brief Enables USP application
  * @param None
  * @retval Success code
  */
USP_StatusTypeDef USP_Enable(struct wtimer_desc *desc)
{
  usp_enabled = 1;  
  ScheduleTask(desc, StartUSPTask, RELATIVE, SECONDS(1));
  return USP_OK;
}

/**
  * @brief Function tries to capture message form screen and compare length of new
  * @brief message with length of old one, if they are equal than it was a proper
  * @brief message and it has to be handled via USP_MessageHandler
  * @param Unused parameter
  * @retval None
  */
void StartUSPTask(struct wtimer_desc *desc)
{ 
  new_len = USP_RX_BUFF_LEN + 1 - screen_huart->RxXferCount;
  if ( (screen_huart->ErrorCode != HAL_UART_ERROR_NONE) || 
       (screen_huart->RxXferCount == 0)) {
    if ((screen_huart->RxXferCount == 0) && !error_screen) {
      UART_SendString("ERROR: rxBuffer Overflow\n\r");
      error_screen = 1;
    }
    HAL_UART_AbortReceive_IT(screen_huart);
    HAL_UART_Receive_IT(screen_huart, uart_rx_buff, USP_RX_BUFF_LEN + 1);
    usp_rx_buff_pointer = 0;
    new_len = old_len = 0;
  }    
  
  if (new_len && (new_len == old_len)) {
    HAL_UART_AbortReceive_IT(screen_huart);
    uint16_t pointer = usp_rx_buff_pointer;
    for (uint16_t i = 0; i < new_len; ++i) {
      usp_rx_buff[usp_rx_buff_pointer] = uart_rx_buff[i];
      usp_rx_buff_pointer = NORMING_TO_USP_RX_BUFF_LEN(usp_rx_buff_pointer+1);
    }
    usp_rx_buff[usp_rx_buff_pointer] = '\0';
    usp_rx_buff_pointer = NORMING_TO_USP_RX_BUFF_LEN(usp_rx_buff_pointer + 1);
    USP_MessageHandler(pointer);
    error_screen = 0;
    new_len = old_len = 0;
    usp_rx_buff_pointer = 0;
    HAL_UART_Receive_IT(screen_huart, uart_rx_buff, USP_RX_BUFF_LEN + 1);
  } else {
    old_len = new_len;
  }
  ScheduleTask(desc, 0, RELATIVE, 10);
}

/**
  * @brief Initializes USP apptication
  * @param Pointer to huart instance
  * @retval Success code
  */
USP_StatusTypeDef USP_Init(UART_HandleTypeDef *new_huart) 
{
  screen_huart = new_huart;
  led.pin = led_Pin;
  led.port = led_GPIO_Port;
  HAL_UART_Receive_IT(screen_huart, uart_rx_buff, USP_RX_BUFF_LEN + 1);
  old_len = 0;
  return USP_OK;
}

/**
  * @brief Deinitializes USP apptication
  * @param None
  * @retval Success code
  */
USP_StatusTypeDef USP_Deinit() 
{ 
  screen_huart = NULL;
  USP_Disable();
  return USP_OK;
}

/**
  * @brief Disables USP apptication
  * @param None
  * @retval Success code
  */
USP_StatusTypeDef USP_Disable()
{  
  usp_enabled = 0;
  return USP_OK;
}

/**
  * @brief Bypasses the message to simcom through STM32
  * @param String that contains message
  * @retval Succes code or overflow code
  */
USP_StatusTypeDef BYPASS_Send(const char* str) 
{
  uint16_t len = 0;
  while(str[len] != '\0') {
    if (len >= USP_BP_LEN)
      return USP_BUFF_OVERFLOW;
    bp_tx_buffer[len] = str[len];
    ++len;
  }
  if (len >= USP_BP_LEN)
      return USP_BUFF_OVERFLOW;
  bp_tx_buffer[len++] = '\n';
   if (len >= USP_BP_LEN)
      return USP_BUFF_OVERFLOW;
  bp_tx_buffer[len++] = '\r';
  if (len >= USP_BP_LEN)
      return USP_BUFF_OVERFLOW;
  bp_tx_buffer[len++] = '\0';  
  sim7020_phy_send(bp_tx_buffer);
  return USP_OK;
}

/**
  * @brief Prints message on the screen
  * @param String that contains message
  * @retval Success code or overflow code
  */
USP_StatusTypeDef UART_SendString(const char* str) 
{
  while( screen_huart->gState == HAL_UART_STATE_BUSY_TX);
  uint16_t i;
  for(i = 0; str[i] != '\0'; ++i) {
    usp_tx_buff[i] = str[i];
    if (usp_tx_buff_pointer >= USP_TX_BUFF_LEN) {
      usp_tx_buff_pointer = 0;
      return USP_BUFF_OVERFLOW;
    }
  }
  if (usp_tx_buff_pointer >= USP_TX_BUFF_LEN) {
      usp_tx_buff_pointer = 0;
      return USP_BUFF_OVERFLOW;
  }
  usp_tx_buff[i++] = '\n';
  HAL_UART_Transmit_IT(screen_huart, usp_tx_buff, i);
  return USP_OK;
}

/**
  * @brief Prints message on the screen
  * @param String that contains message
  * @retval Success code
  */
USP_StatusTypeDef ECHO_Send(const char* str)
{
  UART_SendString(str);
  return USP_OK;
}

/**
  * @brief Enabled, disables or gets led status
  * @param Command
  * @retval Success code
  */
USP_StatusTypeDef LED_Handler(const char* str)
{
  if (strstr(str, "on")){
    UART_SendString("LED ON");
    SET_PIN(led);
  }
  if (strstr(str, "off")){
    UART_SendString("LED OFF");
    RESET_PIN(led);
  }
  if (strstr(str, "status")){
    UART_SendString("STATUS:");
    if (READ_PIN(led) == GPIO_PIN_SET) {
      UART_SendString("led is on\n");
    } else {
      UART_SendString("led is off\n");
    }
  }
  return USP_OK;
}

/**
  * @brief Callback for showing something on the screen
  * @param Pointer to a part of buffer where is our message to be handled
  * @retval None
  */

/*
void USP_Rx_Callback(uint16_t pointer) 
{
  static char str[USP_CALLBACK_LEN];
  uint16_t len = 0;  
  str[len++] = 'b'; str[len++] = 'p'; str[len++] = ':';
  for(;sim7020_phy_rx_buff[pointer] != '\0'; ++len) {
    str[len] = sim7020_phy_rx_buff[pointer];
    pointer = NORM_SIM7020_PHY(pointer + 1);
  }
  str[len++] = '\n'; 
  str[len++] = '\0';
  UART_SendString(str);
}*/
