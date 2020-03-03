#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "typedef.h"
#include "lcd.h"

#define CMD_BASIC_CLEAR         0x01

#define CMD_BASIC_HOME              0x02

#define CMD_BASIC_ENTRY_MODE        0x04
#define EM_DIR_L2R              0x02
#define EM_SHIFT_DISPLAY            0x01

#define CMD_BASIC_DISPLAY           0x08
#define DP_DISPLAY_ON               0x04
#define DP_CURSOR_ON                0x02
#define DP_BLINK_ON                 0x01

#define CMD_BASIC_CURSOR_DISPLAY_CTRL   0x10
#define CDC_SHIFT_DISPLAY           0x08
#define CMC_DIR_L2R                 0x04

#define CMD_BASIC_FUNCTION_SET  0x20
#define FUNSET_DL_8BIT              0x10
#define FUNSET_TWO_LINE		0x08
#define FUNSET_DF_5X10			0x04

#define CMD_BASIC_SET_CGRAM_ADDR    0x40

#define CMD_BASIC_SET_DDRAM_ADDR    0x80

#define CMD_EXTENDED_SR         0x02
#define SR_VERTICAL_SCROLL          0x01

#define CMD_EXTENDED_FUNCTION_SET   0x24
#define FUNSET_GRAPHICS_ON          0x02

#define CMD_EXTENDED_SET_SCROLL_ADDR    0x40
#define CMD_EXTENDED_SET_GRAPHICS_ADDR  0x80

#define delay10ms()     HAL_Delay(10)

static __INLINE void lcdBisEnter(bool bit8)
{
    uint8_t cmd = CMD_BASIC_FUNCTION_SET;
    if(bit8) cmd |= FUNSET_DL_8BIT;
	cmd |= FUNSET_TWO_LINE;
    lcdWriteCommand(cmd);   
}

static __INLINE void lcdSetCGRAMAddr(uint8_t idx, uint8_t addr)
{
    lcdWriteCommand(CMD_BASIC_SET_CGRAM_ADDR  | (idx << 4) | addr);
}

static __INLINE void lcdSetDDRAMAddr(uint8_t line, uint8_t pos)
{
	lcdWriteCommand(CMD_BASIC_SET_DDRAM_ADDR  | (line * 0x40 + pos));
}

/******************************************
以下为扩展指令集的指令
适用LCM:TOPWAY: LM16032DDC-0B-01
******************************************/
static __INLINE void lcdEisEnter(bool bit8, bool graphics)
{
    uint8_t cmd = CMD_EXTENDED_FUNCTION_SET;
    if(bit8) cmd |= FUNSET_DL_8BIT;
    if(graphics) cmd |= FUNSET_GRAPHICS_ON;
    lcdWriteCommand(cmd);       
}

static __INLINE void lcdSetGDRAMPos(uint8_t x, uint8_t y)
{
    lcdWriteCommand(CMD_EXTENDED_SET_GRAPHICS_ADDR  | (y & 0x1f));
    lcdWriteCommand(CMD_EXTENDED_SET_GRAPHICS_ADDR  | (x & 0x0f));
}

void lcdInit(void)
{
    /*all pin output*/
    uint32_t i = LCD_BKLED;
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    for(; i <= LCD_D7; ++i){   
        pLcdConfigure_t plcd_configure = &g_lcd_configure[i]; 
        GPIO_InitStruct.Pin = plcd_configure->index;
        if(LCD_BKLED == i) GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        else GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(plcd_configure->port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(plcd_configure->port, plcd_configure->index, GPIO_PIN_RESET);
    }

    lcdWrite(CMD_BASIC_FUNCTION_SET | FUNSET_DL_8BIT); //LCD_DB = 0x30;
    lcdSetPin(LCD_RS, 0);   //LCD_RS = 0;
    lcdSetPin(LCD_RW, 0);   //LCD_RW = 0;
    for (i=0; i<3; i++){
        lcdSetPin(LCD_CE, 1);   //LCD_CE = 1;
        lcdSetPin(LCD_CE, 0);   //LCD_CE = 0; 
        delay10ms();
    }	

	lcdBisEnter(true);
	lcdWriteCommand(CMD_BASIC_CLEAR);
	lcdWriteCommand(CMD_BASIC_ENTRY_MODE | EM_DIR_L2R);
	lcdWriteCommand(CMD_BASIC_DISPLAY | DP_DISPLAY_ON);
}

void lcdSetBkled(uint32_t status)
{
    lcdSetPin(LCD_BKLED, (status == BKLED_OFF) ? 1 : 0);
    delayNOP();
}

uint8_t lcdGetChar(uint8_t line, uint8_t pos)
{
    uint8_t data = 0;
	if(line > LINE_TWO){
	    printf("line error\n");
	    return 0;
	}
	lcdSetDDRAMAddr(line, pos);
    data = lcdReadData();
    lcdSetDDRAMAddr(line, pos); 

    return data;
}

void lcdSetChar(uint8_t line, uint8_t pos, const uint8_t data)
{
	if(line > LINE_TWO){
	    printf("line error\n");
	    return;
	}
	lcdSetDDRAMAddr(line, pos);
    lcdWriteData(data);
    lcdSetDDRAMAddr(line, pos);
}

void lcdDisplay(uint8_t line, const uint8_t *buffer)
{
	uint8_t pos = 0;
	if(line > LINE_TWO){
	    printf("line error\n");
	    return;
	}
    lcdWriteCommand(CMD_BASIC_HOME);
	lcdSetDDRAMAddr(line, 0);
	while(buffer[pos]) {
	    lcdWriteData(buffer[pos++]);
	    if(pos >= 40) break;
	}
    while(pos++ < 40){
        lcdWriteData(' ');
    }
}

void lcdDisplayShift(bool b_left)
{
    if(b_left)lcdWriteCommand(CMD_BASIC_CURSOR_DISPLAY_CTRL | CDC_SHIFT_DISPLAY);
    else lcdWriteCommand(CMD_BASIC_CURSOR_DISPLAY_CTRL | CDC_SHIFT_DISPLAY | CMC_DIR_L2R);
}

int32_t lcdDisplayCenter(uint8_t line, const uint8_t *buffer)
{
	uint8_t i = 0, pos = 0;
    uint32_t len = 0, fixed_pos = 0;

	if(line > LINE_TWO){
	    printf("line error\n");
	    return -1;
	}
    len = strlen((char*)buffer);

    lcdWriteCommand(CMD_BASIC_HOME);
    lcdSetDDRAMAddr(line, 0);
    if(len <= CHAR_PER_LINE){
		fixed_pos = (CHAR_PER_LINE - len) / 2;
        pos = fixed_pos;
		while(fixed_pos--){      
			lcdWriteData(' ');
		}
        fixed_pos = pos;
	}
    while(buffer[i]) {
	    lcdWriteData(buffer[i++]);
	    if((++pos) >= CHAR_PER_LINE) break;
	}
    while(pos++ < CHAR_PER_LINE){
        lcdWriteData(' ');
    }
    return fixed_pos;
}

void lcdInitCursor(void)
{
    return;
}
/*
参数说明：
    line:  光标所在位置y(纵向)。range: 0-1.
    pos :  光标所在位置x(横向)。range: 0-15(19).
*/
void lcdDrawCursor(uint8_t line, uint8_t pos)
{
    lcdWriteCommand(CMD_BASIC_DISPLAY | DP_DISPLAY_ON | DP_CURSOR_ON);
	lcdSetDDRAMAddr(line, pos);
}

void lcdClearCursor(uint8_t line, uint8_t pos)
{
    lcdWriteCommand(CMD_BASIC_DISPLAY | DP_DISPLAY_ON);
}

