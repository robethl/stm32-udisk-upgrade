#ifndef _LCD_H_
#define _LCD_H_

#include "stm32f4xx_hal.h"
#include "typedef.h"

#ifdef DNCM_1U
#define CHAR_PER_LINE 20
#else
#define CHAR_PER_LINE 16
#endif

#define BKLED_OFF   (0)
#define BKLED_LIGHT (1)

#define LINE_ONE	0
#define LINE_TWO	1


void lcdWrite(uint8_t data);
uint8_t lcdRead(void);
void lcdWriteCommand(uint8_t command);
void lcdWriteData(uint8_t data);
uint8_t lcdReadData(void);

void lcdInit(void);
void lcdSetBkled(uint32_t status);
uint8_t lcdGetChar(uint8_t line, uint8_t pos);
void lcdSetChar(uint8_t line, uint8_t pos, const uint8_t data);
void lcdDisplay(uint8_t line, const uint8_t *buffer);
void lcdDisplayScroll(uint8_t line, const uint8_t *buffer);
void lcdDisplayShift(bool b_left);
int32_t lcdDisplayCenter(uint8_t line, const uint8_t *buffer);

/*
参数说明：
    line:  光标所在位置y(纵向)。range: 0-1.
    pos :  光标所在位置x(横向)。range: 0-19.
    
*/
void lcdInitCursor(void);
void lcdDrawCursor(uint8_t line, uint8_t pos);
void lcdClearCursor(uint8_t line, uint8_t pos);

#endif
