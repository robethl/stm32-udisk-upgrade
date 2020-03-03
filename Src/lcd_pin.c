#include <string.h>
#include "stm32f4xx_hal.h"
#include "typedef.h"
#include "lcd.h"

#define delayNOP()  	//HAL_Delay(1)

typedef enum
{
    LCD_BKLED = 0,
    LCD_CE,
    LCD_RW,
    LCD_RS,
    LCD_D0,
    LCD_D1,
    LCD_D2,
    LCD_D3,
    LCD_D4,
    LCD_D5,
    LCD_D6,
    LCD_D7
}Lcd_e;

typedef struct{
    GPIO_TypeDef* port;
    uint16_t index;
}LcdConfigure_t, *pLcdConfigure_t;

static LcdConfigure_t g_lcd_configure[] = {
    {GPIOB, GPIO_PIN_9},
    {GPIOC, GPIO_PIN_11},
    {GPIOC, GPIO_PIN_10},
    {GPIOA, GPIO_PIN_15},
    {GPIOC, GPIO_PIN_12},
    {GPIOD, GPIO_PIN_2},
    {GPIOB, GPIO_PIN_3},
    {GPIOB, GPIO_PIN_4},
    {GPIOB, GPIO_PIN_5},
    {GPIOB, GPIO_PIN_6},
    {GPIOB, GPIO_PIN_7},
    {GPIOB, GPIO_PIN_8}
};


static inline uint32_t lcdGetPin(Lcd_e pin)
{
    pLcdConfigure_t plcd_configure = &g_lcd_configure[pin];
    return  HAL_GPIO_ReadPin(plcd_configure->port, plcd_configure->index);
}

static inline void lcdSetPin(Lcd_e pin, uint32_t is_set)
{
    pLcdConfigure_t plcd_configure = &g_lcd_configure[pin];
    HAL_GPIO_WritePin(plcd_configure->port, plcd_configure->index, is_set ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void lcdSetDbusIn()
{
    uint32_t i = LCD_D0;
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    for(; i <= LCD_D7; ++i){   
        pLcdConfigure_t plcd_configure = &g_lcd_configure[i]; 
        GPIO_InitStruct.Pin = plcd_configure->index;
        HAL_GPIO_Init(plcd_configure->port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(plcd_configure->port, plcd_configure->index, GPIO_PIN_RESET);
    }
}

static void lcdSetDbusOut()
{
    uint32_t i = LCD_D0;
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    for(; i <= LCD_D7; ++i){   
        pLcdConfigure_t plcd_configure = &g_lcd_configure[i]; 
        GPIO_InitStruct.Pin = plcd_configure->index;
        HAL_GPIO_Init(plcd_configure->port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(plcd_configure->port, plcd_configure->index, GPIO_PIN_RESET);
    }
}

static void lcdWrite(uint8_t data)
{
    lcdSetDbusOut();
    delayNOP();
    lcdSetPin(LCD_D0, (data >> 0) & 0x01);
    lcdSetPin(LCD_D1, (data >> 1) & 0x01);
    lcdSetPin(LCD_D2, (data >> 2) & 0x01);
    lcdSetPin(LCD_D3, (data >> 3) & 0x01);
    lcdSetPin(LCD_D4, (data >> 4) & 0x01);
    lcdSetPin(LCD_D5, (data >> 5) & 0x01);
    lcdSetPin(LCD_D6, (data >> 6) & 0x01);
    lcdSetPin(LCD_D7, (data >> 7) & 0x01);
    delayNOP();
}

static uint8_t lcdRead(void)
{
    uint8_t data = 0;
    lcdSetDbusIn();
    delayNOP();
    data |= (lcdGetPin(LCD_D0) << 0);
    data |= (lcdGetPin(LCD_D1) << 1);
    data |= (lcdGetPin(LCD_D2) << 2);
    data |= (lcdGetPin(LCD_D3) << 3);
    data |= (lcdGetPin(LCD_D4) << 4);
    data |= (lcdGetPin(LCD_D5) << 5);
    data |= (lcdGetPin(LCD_D6) << 6);
    data |= (lcdGetPin(LCD_D7) << 7);
    
    return data;
}

void lcdWriteCommand(uint8_t command)
{
    bool busy = true;
    lcdSetPin(LCD_RS, 0);   
    lcdSetPin(LCD_RW, 1);
    while(busy)
    {
        lcdWrite(0xFF);
        lcdSetPin(LCD_CE, 1);
        busy = (lcdRead() & 0x80) ? true : false;
        lcdSetPin(LCD_CE, 0);
    }    
    lcdSetPin(LCD_RW, 0);
    lcdSetPin(LCD_CE, 1);   
    lcdWrite(command);
    lcdSetPin(LCD_CE, 0);
}

void lcdWriteData(uint8_t data)
{
    bool busy = true;
    lcdSetPin(LCD_RS, 0);
    lcdSetPin(LCD_RW, 1);
    while(busy)
    {
        lcdWrite(0xFF);
        lcdSetPin(LCD_CE, 1);
        busy = (lcdRead() & 0x80) ? true : false;
        lcdSetPin(LCD_CE, 0);
    }
    lcdSetPin(LCD_RS, 1);
    lcdSetPin(LCD_RW, 0);
    lcdSetPin(LCD_CE, 1);
    lcdWrite(data);
    lcdSetPin(LCD_CE, 0);
}

uint8_t lcdReadData(void)
{
    uint8_t data = 0;
    bool busy = true;
    lcdSetPin(LCD_RS, 0);
    lcdSetPin(LCD_RW, 1);
    while(busy)
    {
        lcdWrite(0xFF);
        lcdSetPin(LCD_CE, 1);
        busy = (lcdRead() & 0x80) ? true : false;
        lcdSetPin(LCD_CE, 0);
    }
    lcdSetPin(LCD_RS, 1);
    lcdSetPin(LCD_RW, 1);
    //lcdWrite(0xff);           
    lcdSetPin(LCD_CE, 1);
    data = lcdRead();
    lcdSetPin(LCD_CE, 0);
    return data;
}

