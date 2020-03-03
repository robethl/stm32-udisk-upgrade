/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "typedef.h"
#include "lcd.h"
#include "db.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifdef DNCM_1U
#define OTA_DEVICE  "[DNCM-1U]"
#else 
#define OTA_DEVICE  "[DNCM-1W]"
#endif
#define SECTION_LEN (512)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static FLASH_EraseInitTypeDef erase_init_struct = {
    .TypeErase = FLASH_TYPEERASE_SECTORS,
    .Banks = FLASH_BANK_1,
    .Sector = FLASH_SECTOR_4,
    .NbSectors = 1,
    .VoltageRange = FLASH_VOLTAGE_RANGE_3
};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
int32_t f_readline(FIL* fp, uint8_t* buff, uint32_t size, uint32_t* br)
{
    uint32_t act_read_len = 0;
    uint8_t c = 0;
    *br = 0;
    while(true){
        if(FR_OK == f_read(fp, &c, 1, (void *)&act_read_len) && act_read_len){
            if(c == '\r')continue;
            if(c == '\n')break;
            if(*br < size - 1) buff[(*br)++] = c;
        }else{
            break;     
        } 
    }
    buff[(*br)] = 0;
    return SUCCESS;
}

bool parseKeyValue(uint8_t* str, uint8_t* key, uint32_t *value)
{
    while(*str == ' ')++str;
    if(memcmp(str, key, strlen((char*)key)))return false;
    str += strlen((char*)key);

    while(*str == ' ')++str;

    if(*str++ != '=')return false;

    while(*str == ' ')++str;

    *value = strtol((char*)str, NULL, 10);
    return true;
}

bool bootCheckAppVersion(void)
{
    FIL fp;
    uint32_t  act_read_len = 0;
    uint8_t data[SECTION_LEN];

    if( f_open(&fp, "ota.txt", FA_OPEN_EXISTING | FA_READ) != FR_OK){
        printf("no ota.txt\n");
        return false;
    }

    if(f_readline(&fp, data, sizeof(data), &act_read_len) && act_read_len){
        if(strcmp(OTA_DEVICE, (char*)data)){
            printf("ota.txt format error: device type\n");
            return false;
        }
    }else{
        printf("ota.txt: no device type\n");
        return false;
    }

    if(f_readline(&fp, data, sizeof(data), &act_read_len) && act_read_len){
        uint32_t app_version = 0;
        if(!parseKeyValue(data, "app_version", &app_version)){
            printf("ota.txt format error: app_version\n");
            return false;
        }

        if(app_version == dbGetAppVersion()) {
            printf("the app is newest\n");
            return false;
        }
        return true;
    }
    printf("ota.txt: no app_version\n");
    return false;
}
/**
  * @brief  Files operations: Read/Write and compare
  * @param  None
  * @retval None
  */
int32_t bootOta(uint32_t add)
{
    FIL fp;
    uint32_t i = 0, sector_error = 0, act_read_len = 0, section = 0, last_section = 0;
    uint8_t data[SECTION_LEN];
    char buf[CHAR_PER_LINE + 1] = {0};

    if( f_open(&fp, "ota.bin", FA_OPEN_EXISTING | FA_READ) != FR_OK){
        printf("no ota.bin\n");
        return -1;
    }

    lcdDisplay(LINE_ONE, "OTA>>>");
    dbSetAppValid(0);

    lcdDisplay(LINE_TWO, "Erase flash");
    HAL_FLASH_Unlock(); 
    
    erase_init_struct.Sector = FLASH_SECTOR_4;
    HAL_FLASHEx_Erase(&erase_init_struct, &sector_error);
    erase_init_struct.Sector = FLASH_SECTOR_5;
    HAL_FLASHEx_Erase(&erase_init_struct, &sector_error);

    last_section = f_size(&fp) / SECTION_LEN + 1;

    while(true){
        if((section % 50) == 0){
            snprintf(buf, sizeof(buf), "write %d%%", section * 100 / last_section);
            lcdDisplay(LINE_TWO, (uint8_t*)buf);
        }

        if((FR_OK == f_read(&fp, data, sizeof(data), (void *)&act_read_len)) && act_read_len){
            for(i = 0; i < act_read_len; i++){
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, add++, data[i]);
            }
            ++section;
        }else{
            break;
        } 
    };
    
    HAL_FLASH_Lock();//上锁  
    f_close(&fp);

    if(section < last_section)return -2;
    
    lcdDisplay(LINE_TWO, "write 100%");
    dbSetAppValid(('W' << 8) | 'F');
    return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
