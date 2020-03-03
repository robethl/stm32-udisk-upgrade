#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "usart.h"

/* hook keil c putc and getc implemented */
int fputc(int ch, FILE *f)
{
    uint8_t temp[1]={ch};
    HAL_UART_Transmit(&huart1, temp, 1, 200);
    return ch;  
}

int fgetc(FILE *f)
{
    uint8_t temp = 0;

    HAL_StatusTypeDef nres = HAL_UART_Receive(&huart1, &temp, 1, 200);
    if(nres == HAL_OK)return temp;
    return -1;
}

int ferror(FILE *f)
{
  /* Your implementation of ferror */
  return EOF;
}

//******************************************************************************
