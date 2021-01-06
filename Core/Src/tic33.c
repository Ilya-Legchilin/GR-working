#include <stdlib.h>
#include "tic33.h"

#define CLEAR_MODER(pin) ~(pin*pin*3)
#define MODER_SET(pin, state) (pin*pin*state)

void lcd_init_(){
  //RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
	HAL_GPIO_WritePin(DISPLAY_Q1_GPIO_Port, DISPLAY_Q1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(DISPLAY_Q2_GPIO_Port, DISPLAY_Q2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(DISPLAY_Q3_GPIO_Port, DISPLAY_Q3_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin( GPIOB, SCLK_Pin, GPIO_PIN_SET);
}

void lcd_pins_to_GND(){
	HAL_GPIO_WritePin(DISPLAY_Q1_GPIO_Port, DISPLAY_Q1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DISPLAY_Q2_GPIO_Port, DISPLAY_Q2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DISPLAY_Q3_GPIO_Port, DISPLAY_Q3_Pin, GPIO_PIN_RESET);
}

void LCD_SCLK_SWITCH(void)
{
	static uint8_t last_SCLK_state = 0;
	HAL_GPIO_WritePin(SCLK_GPIO_Port, SCLK_Pin, last_SCLK_state);
	last_SCLK_state ^= 1;
}

// TIC33
// dot = 1
// 0...9,-,deg,Space
// Segment bits:
//    _7_
//  6|_2_|1
//  5|_4_|3 .0
//
//     ___128
//  64|_4_|2
//  32|_16|8 .1
#define   minus   10
#define   degress 11
#define   space   12
#define   m1      13
#define   m2      14
#define   m3      15

const uint8_t DigitTIC33[]  = {
		2+8+16+32+64+128,
		2+8
		,128+2+4+32+16
		,2+8+128+4+16
		,64+4+2+8
		,128+64+4+8+16
		,128+64+4+8+16+32
		,128+2+8
		,2+4+8+16+32+64+128
		,2+4+8+16+64+128
		,4
		,2+4+64+128,
		0
		, /*H*/2+4+8+32+64
		, /*�*/2+8+32+64+128
		, /*O*/2+8+16+32+64+128
		, /*P*/32+64+2+4+128
};

const uint8_t SymbolsTIC33[] = {
		2+4+8+32+64+128, //a
		64+8+16+32+4, //b
		4+32+16, //c
		8+16+32+4+2,//d
		128+64+4+32+16, //e
		128+64+32+4,//f
		128+64+2+4+8+16,//g
		64+4+32+8,//h
		32,//i
		8+16+2,//j
		64+32+4,//k
		64+32+16,//l
		32+8,//m
		32+4+8,//n
		32+4+8+16,//o
		32+64+128+2+4,//p
		64+128+4+2+8,//q
		32+4,//r
		128+64+8+16+4,//s
		64+32+16+4,//t
		32+16+8+1,//u
		32+16+8,//v
		32+8+1,//w
		64+32+4+2+8,//x
		64+4+2+8,//y
		4+16//z
};
const uint8_t DigitHexTIC33[]  = {
    2+8+16+32+64+128,//0
    2+8,//1
    128+2+4+32+16,//2
    2+8+128+4+16,//3
    64+4+2+8,//4
    128+64+4+8+16,//5
    128+64+4+8+16+32,//6
    128+2+8,//7
    2+4+8+16+32+64+128,//8
    2+4+8+16+64+128,//9
    2+4+8+32+64+128,//a
    4+8+16+32+64,//b
    16+32+64+128,//c
    2+4+8+16+32,//d
    4+16+32+64+128,//e
    4+32+64+128,//f
};

const uint8_t tic33_H = 64 + 32 + 4 + 2 + 8;

void LCD_WriteCharTIC33(uint8_t ch,uint8_t point)
{
    uint8_t i;
    if(ch < 17) ch=DigitTIC33[ch];
    else if(ch < 47) ch = SymbolsTIC33[ch-20];
    else ch = 0;
    if(point) ch |= 1;
    // Output
    for(i=8; i; i--)
    {
        if(ch & 128)
        {
            SetDin;
            SetDin;
        }
        else
        {

            ClrDin;
            ClrDin;
        }

        ch=ch<<1;
        SClk;
    }
}

void LCD_WriteHexCharTIC33(uint8_t ch,uint8_t point)
{
    uint8_t i;
    ch=DigitHexTIC33[ch];
    if(point) ch |= 1;
    // Output
    for(i=8; i; i--)
    {
        if(ch & 128)
        {
            SetDin;
            SetDin;
        }
        else
        {
            ClrDin;
            ClrDin;
        }

        ch=ch<<1;
        SClk;
    }
}

void LCD_WriteTIC33(uint8_t ch)
{
    uint8_t i;
    // Output
    for(i=8; i; i--)
    {
        if(ch & 128)
        {
            SetDin;
            SetDin;
        }
        else
        {
            ClrDin;
            ClrDin;
        }

        ch=ch<<1;
        SClk;
    }
}

const uint32_t divs[]={
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000};

void LCD_WriteNumber(const uint32_t num)
{
	LCD_Clear();

	uint8_t i;
	uint8_t num_digits[9];
	for(i = 0; i < 9; i++)
		num_digits[i] = (num/divs[i]) % 10;

	LCD_WriteCharTIC33(13, 1);

	for(i = 0; i < 8; i++)
		LCD_WriteCharTIC33(num_digits[7-i], 0);

	SLoad;
}

void LCD_WriteInt(const uint32_t num)
{

	uint8_t i;
	uint8_t num_digits[9];

	for(i = 0; i < 9; i++)
		num_digits[i] = (num/divs[i]) % 10;

	for(i = 0; i < 9; i++)
		LCD_WriteCharTIC33(num_digits[8-i], 0);

	SLoad;

}

uint8_t get_tic33_symbol(char s)
{
	if( (s >= '0') && (s <= '9') )
		return s - '0';

	if( (s >= 'a') && (s <= 'z') )
		return s - 'a' + 20;

	if( (s >= 'A') && (s <= 'Z') )
		return s - 'A' + 20;

	return 12;
}

void LCD_WriteStr(const char *str)
{
	LCD_Clear();

	uint8_t i = 0;
	uint8_t end_of_str = 0;
	for(i = 0; i < 9; i++)
	{
		if(!end_of_str)
		{
			char s = str[i];
			if(s != '\0')
			{
				LCD_WriteCharTIC33(get_tic33_symbol(s), 0);
			}
			else
			{
				LCD_WriteCharTIC33(12, 0);
				end_of_str = 1;
			}
		}
		else LCD_WriteCharTIC33(12, 0);
	}

	SLoad;
}

void LCD_WriteHex(uint32_t num)
{
	LCD_Clear();

    LCD_WriteCharTIC33(13, 1);
    LCD_WriteCharTIC33(12, 0);
    LCD_WriteCharTIC33(12, 0);

    //LCD_WriteHexCharTIC33((num >> 28) & 0xF, 0);
    //LCD_WriteHexCharTIC33((num >> 24) & 0xF, 0);
    LCD_WriteHexCharTIC33((num >> 20) & 0xF, 0);
    LCD_WriteHexCharTIC33((num >> 16) & 0xF, 0);
    LCD_WriteHexCharTIC33((num >> 12) & 0xF, 0);
    LCD_WriteHexCharTIC33((num >>  8) & 0xF, 0);
    LCD_WriteHexCharTIC33((num >>  4) & 0xF, 0);
    LCD_WriteHexCharTIC33( num        & 0xF, 0);

    SLoad;
}

void LCD_WriteBCD(uint8_t *bcd) 
{   
  LCD_Clear();
  
  LCD_WriteCharTIC33(bcd[4] >> 4, 0);
  LCD_WriteCharTIC33(bcd[4] & 0x0F, 0);

  LCD_WriteCharTIC33(bcd[3] >> 4, 0);
  LCD_WriteCharTIC33(bcd[3] & 0x0F, 0);

  LCD_WriteCharTIC33(bcd[2] >> 4, 0);
  LCD_WriteCharTIC33(bcd[2] & 0x0F, 0);

  LCD_WriteCharTIC33(bcd[1] >> 4, 0);
  LCD_WriteCharTIC33(bcd[1] & 0x0F,0);

  LCD_WriteCharTIC33(bcd[0] >> 4, 0);
  LCD_WriteCharTIC33(bcd[0] & 0x0F, 0);
  
  SLoad;
  
  lcd_pins_to_GND();
}

extern uint8_t digits[5];

void LCD_WaterDigits()
{
  LCD_Clear();

  LCD_WriteCharTIC33(digits[4] >> 4, 0);
  LCD_WriteCharTIC33(digits[4] & 0x0F, 0);

  LCD_WriteCharTIC33(digits[3] >> 4, 0);
  LCD_WriteCharTIC33(digits[3] & 0x0F, 0);

  LCD_WriteCharTIC33(digits[2] >> 4, 1);
  LCD_WriteCharTIC33(digits[2] & 0x0F, 0);

  LCD_WriteCharTIC33(digits[1] >> 4, 0);
  LCD_WriteCharTIC33(digits[1] & 0x0F,0);

  LCD_WriteCharTIC33(digits[0] >> 4, 0);

  SLoad;

  lcd_pins_to_GND();
}

void LCD_HighAccuracy()
{
  LCD_Clear();

  LCD_WriteCharTIC33(14, 0);
  LCD_WriteCharTIC33(16, 0);
  LCD_WriteCharTIC33(12, 0);

  LCD_WriteCharTIC33(12, 1);
  LCD_WriteCharTIC33(digits[2] & 0x0F, 0);

  LCD_WriteCharTIC33(digits[1] >> 4, 0);
  LCD_WriteCharTIC33(digits[1] & 0x0F, 0);

  LCD_WriteCharTIC33(digits[0] >> 4, 0);
  LCD_WriteCharTIC33(digits[0] & 0x0F, 0);

  SLoad;

  lcd_pins_to_GND();
}

void LCD_SoftwareVersion()
{
  LCD_Clear();

  LCD_WriteCharTIC33(14, 0);
  LCD_WriteCharTIC33(15, 0);
  LCD_WriteCharTIC33(12, 0);
  LCD_WriteCharTIC33(2, 0);
  LCD_WriteCharTIC33(10, 0);
  LCD_WriteCharTIC33(1, 0);
  LCD_WriteCharTIC33(9, 0);
  LCD_WriteCharTIC33(15, 0);
  LCD_WriteCharTIC33(15, 0);

  SLoad;

  lcd_pins_to_GND();
}



void LCD_Clear(void)
{
    LCD_WriteCharTIC33(space,0);
    LCD_WriteCharTIC33(space,0);
    LCD_WriteCharTIC33(space,0);
    LCD_WriteCharTIC33(space,0);
    LCD_WriteCharTIC33(space,0);
    LCD_WriteCharTIC33(space,0);
    LCD_WriteCharTIC33(space,0);
    LCD_WriteCharTIC33(space,0);
    LCD_WriteCharTIC33(space,0);
    SLoad;
}
