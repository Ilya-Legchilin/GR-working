#ifndef TIC33_H
#define TIC33_H
#include "stm32l0xx_hal.h"
#include "main.h"

#define SetDin  HAL_GPIO_WritePin(DISPLAY_Q2_GPIO_Port, DISPLAY_Q2_Pin, GPIO_PIN_SET);
#define ClrDin  HAL_GPIO_WritePin(DISPLAY_Q2_GPIO_Port, DISPLAY_Q2_Pin, GPIO_PIN_RESET);
#define SetDclk HAL_GPIO_WritePin(DISPLAY_Q1_GPIO_Port, DISPLAY_Q1_Pin, GPIO_PIN_SET);
#define ClrDclk HAL_GPIO_WritePin(DISPLAY_Q1_GPIO_Port, DISPLAY_Q1_Pin, GPIO_PIN_RESET);
#define SClk    SetDclk;SetDclk;ClrDclk;ClrDclk;
#define SetLoad HAL_GPIO_WritePin(DISPLAY_Q3_GPIO_Port, DISPLAY_Q3_Pin, GPIO_PIN_SET);
#define ClrLoad HAL_GPIO_WritePin(DISPLAY_Q3_GPIO_Port, DISPLAY_Q3_Pin, GPIO_PIN_RESET);
#define SLoad   SetLoad;SetLoad;ClrLoad;ClrLoad;

#define DISPLAY_Q1_GPIO_Port GPIOB
#define DISPLAY_Q2_GPIO_Port GPIOB
#define DISPLAY_Q3_GPIO_Port GPIOB
#define SCLK_GPIO_Port GPIOB
#define DISPLAY_Q1_Pin GPIO_PIN_1
#define DISPLAY_Q2_Pin GPIO_PIN_11
#define DISPLAY_Q3_Pin GPIO_PIN_2
#define SCLK_Pin GPIO_PIN_10

void LCD_SCLK_SWITCH(void);
void LCD_WriteCharTIC33(uint8_t ch,uint8_t point);
void LCD_WriteTIC33(uint8_t ch);
//void LCD_Write_test(uint8_t __xdata *bcd);
void LCD_WriteInt(uint32_t num);
void LCD_WriteBCD(uint8_t *bcd);
void LCD_WaterDigits();
void LCD_HighAccuracy();
void LCD_SoftwareVersion();
void LCD_WriteNumber(const uint32_t num);
void LCD_WriteHex(uint32_t num);
void lcd_init_();
void LCD_Clear(void);
void LCD_WriteStr(const char *str);

#endif
