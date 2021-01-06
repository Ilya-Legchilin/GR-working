#include "sim7020_phy.h"
#include "main.h"
#include "uart_simple_protocol.h"

static UART_HandleTypeDef *sim_huart;


typedef struct {
  GPIO_TypeDef* port;
  uint16_t pin;
} DigOut;

uint8_t error_sim = 0;


static DigOut on_sim;
static DigOut on_cap;
static DigOut pwrkey;
static DigOut modem_reset;


static uint8_t sim7020_phy_rx_enabled;

#define SET_PIN(x) HAL_GPIO_WritePin(x.port, x.pin, GPIO_PIN_SET)
#define RESET_PIN(x) HAL_GPIO_WritePin(x.port, x.pin, GPIO_PIN_RESET)


uint8_t sim7020_phy_rx_buff[SIM7020_PHY_RX_BUFF_LEN + 1];
static uint8_t uart_rx_buffer[SIM7020_PHY_RX_BUFF_LEN + 1];
uint8_t sim7020_tra_rx_buff[256] = {0};
uint8_t sim7020_tra_rx_buff_p = 0;
uint8_t sim7020_tra_tx_buff[512] = {0};



static struct wtimer_desc reset_on_cap_desc;
static struct wtimer_desc reset_on_sim_desc;
static struct wtimer_desc reset_pwrkey_desc;
static struct wtimer_desc set_on_cap_desc;
static struct wtimer_desc set_on_sim_desc;
static struct wtimer_desc set_pwrkey_desc;
static struct wtimer_desc sim7020_phy_desc;


/**
  * @brief Initializes simcom
  * @param None
  * @retval None
  */
void sim7020_phy_init(UART_HandleTypeDef *new_huart)
{                                                     
  on_sim.port = on_sim_GPIO_Port;
  on_cap.port = on_cap_GPIO_Port;
  pwrkey.port = pwrkey_GPIO_Port;
  modem_reset.port = modem_reset_GPIO_Port;
  on_sim.pin = on_sim_Pin;
  on_cap.pin = on_cap_Pin;
  pwrkey.pin = pwrkey_Pin;
  modem_reset.pin = modem_reset_Pin;
  sim_huart = new_huart;   
  SET_PIN(pwrkey);
  SET_PIN(on_cap);
  SET_PIN(on_sim);
}
  
/**
  * @brief Deinitializes simcom
  * @param None
  * @retval None
  */
void sim7020_phy_deinit(void) 
{
  on_sim.port = NULL;
  on_cap.port = NULL;
  pwrkey.port = NULL;
  on_sim.pin = -1;
  on_cap.pin = -1;
  pwrkey.pin = -1;
  sim_huart = NULL;
}

/**
  * @brief Enable simcom
  * @param None
  * @retval None
  */
void sim7020_phy_power_on(void)
{
  RESET_PIN(pwrkey); 
  SET_PIN(on_cap);
  RESET_PIN(on_sim);
  ScheduleTask(&reset_on_cap_desc, sim7020_reset_on_cap, RELATIVE, SECONDS(1));
  ScheduleTask(&set_pwrkey_desc, sim7020_set_pwrkey, RELATIVE, MILLISECONDS(1010));
  
  sim7020_phy_rx_enabled = 1;
  ScheduleTask(&sim7020_phy_desc, sim7020_phy_processor, RELATIVE, MILLISECONDS(100));
}

/**
  * @brief Disable simcom
  * @param None
  * @retval None
  */
void sim7020_phy_power_off(void)
{
  sim7020_phy_rx_enabled = 0;
  ScheduleTask(&reset_pwrkey_desc, sim7020_reset_pwrkey, RELATIVE, MILLISECONDS(1));
  ScheduleTask(&set_on_cap_desc, sim7020_set_on_cap, RELATIVE, MILLISECONDS(11));
  ScheduleTask(&set_on_sim_desc, sim7020_set_on_sim, RELATIVE, MILLISECONDS(21));
}

/**
  * @brief Send message to simcom
  * @param string containing the message
  * @retval None
  */
void sim7020_phy_send(const char* str)
{
  uint16_t i = 0;
  for (; str[i] != '\0'; i++);
  HAL_UART_Transmit_IT(sim_huart, (uint8_t*)str, i);
  UART_SendString("@@@@");
  UART_SendString(str);
  sim7020_tra_tx_buff[0] = '\0';
  sim7020_tra_rx_buff_p = 0;
}

/**
  * @brief Abort receive session
  * @param None
  * @retval None
  */
void Sim7020_Phy_Abort_Receive(void)
{
  HAL_UART_AbortReceive(sim_huart);
}

/**
  * @brief Reset simcom
  * @param None
  * @retval None
  */
void Sim7020_Phy_Reset(void)
{
  RESET_PIN(modem_reset);
  //osDelay(20);
  SET_PIN(modem_reset);
}


void sim7020_set_on_cap(struct wtimer_desc * desc)
{
  SET_PIN(on_cap);
}


void sim7020_set_on_sim(struct wtimer_desc * desc)
{
  SET_PIN(on_sim);
}


void sim7020_set_pwrkey(struct wtimer_desc * desc)
{
  SET_PIN(pwrkey);
}


void sim7020_set_modem_reset(struct wtimer_desc * desc)
{
  SET_PIN(modem_reset);
}


void sim7020_reset_on_cap(struct wtimer_desc * desc)
{
  RESET_PIN(on_cap);
}


void sim7020_reset_on_sim(struct wtimer_desc * desc)
{
  RESET_PIN(on_sim); 
}


void sim7020_reset_pwrkey(struct wtimer_desc * desc)
{
  RESET_PIN(pwrkey); 
}


void sim7020_reset_modem_reset(struct wtimer_desc * desc)
{
  RESET_PIN(modem_reset);
}


void sim7020_phy_processor(struct wtimer_desc * desc)
{
  static uint16_t old_len = 0;
  static uint16_t new_len = 0;
  HAL_UART_Receive_IT(sim_huart, uart_rx_buffer, SIM7020_PHY_RX_BUFF_LEN + 1);
  new_len = SIM7020_PHY_RX_BUFF_LEN + 1 - sim_huart->RxXferCount;
  if ( (sim_huart->ErrorCode != HAL_UART_ERROR_NONE) || (sim_huart->RxXferCount == 0) ){
    if (sim_huart->RxXferCount == 0)
       error_sim = 1;
    HAL_UART_AbortReceive_IT(sim_huart);
    HAL_UART_Receive_IT(sim_huart, uart_rx_buffer, SIM7020_PHY_RX_BUFF_LEN + 1);
    new_len = old_len = 0;
    error_sim = 0;
  }        
  if (new_len && (new_len == old_len)) {
    uint8_t i = 0;
    uint8_t length = SIM7020_PHY_RX_BUFF_LEN + 1 - sim_huart->RxXferCount;
    for (; i < length; i++)
      sim7020_tra_rx_buff[i] = uart_rx_buffer[i];
    HAL_UART_AbortReceive(sim_huart);
    HAL_UART_Receive_IT(sim_huart, uart_rx_buffer, SIM7020_PHY_RX_BUFF_LEN + 1);
    Sim7020_Phy_Rx_Callback(i);
    new_len = old_len = 0;
  } else {
    old_len = new_len;
  }
  if(sim7020_phy_rx_enabled) 
    ScheduleTask(desc, 0, RELATIVE, MILLISECONDS(20));
}


void Sim7020_Phy_Rx_Callback(uint8_t i) 
{
  sim7020_tra_rx_buff[i] = 0;
  sim7020_tra_rx_buff_p = i;
  UART_SendString("->");
  UART_SendString((char *)sim7020_tra_rx_buff);
  //USP_Rx_Callback(pointer);
}