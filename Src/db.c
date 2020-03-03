#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "typedef.h"
#include "rettype.h"
#include "db.h"

#define DB_MASTER   1
#define DB_BUCKUP   2
/*
* 扇区2/3 共32k用于保持数据
*/
#define DB_LEN      0x8000

#define FLASH_SECTOR_2_ADDR     (FLASH_BASE + 0x8000)
#define FLASH_SECTOR_3_ADDR     (FLASH_BASE + 0xc000)

/*--------------------------------------------------*/

static FLASH_EraseInitTypeDef erase_init_struct = {
    .TypeErase = FLASH_TYPEERASE_SECTORS,
    .Banks = FLASH_BANK_1,
    .Sector = FLASH_SECTOR_2,
    .NbSectors = 1,
    .VoltageRange = FLASH_VOLTAGE_RANGE_3
};

static Db_t db_master, db_bakup;

static int32_t load(uint32_t db_type)
{
    uint32_t i = 0, addr = 0;
    uint8_t *pdb = NULL;

    if(db_type == DB_MASTER){
        addr = FLASH_SECTOR_2_ADDR;
        pdb = (uint8_t*)&db_master;
    }else if(db_type == DB_BUCKUP){
        addr = FLASH_SECTOR_3_ADDR;
        pdb = (uint8_t*)&db_bakup;
    }else{
        return FAILURE;
    }

    for(i=0; i < sizeof(Db_t); i++){
        *(pdb + i) = *(__IO uint8_t*)(addr + i);
    }
    return SUCCESS;
}

static int32_t save(uint32_t db_type)
{
    uint32_t i = 0, addr = 0, sector_error = 0;
    uint8_t *pdb = NULL;

    if(db_type == DB_MASTER){
        erase_init_struct.Sector = FLASH_SECTOR_2;
        addr = FLASH_SECTOR_2_ADDR;
        pdb = (uint8_t*)&db_master;
    }else if(db_type == DB_BUCKUP){
        erase_init_struct.Sector = FLASH_SECTOR_3;
        addr = FLASH_SECTOR_3_ADDR;
        pdb = (uint8_t*)&db_bakup;
    }else{
        return FAILURE;
    }

    HAL_FLASH_Unlock(); 
    HAL_FLASHEx_Erase(&erase_init_struct, &sector_error);
    for(i=0; i < sizeof(Db_t); i++){
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + i, *(pdb + i));
    }
    HAL_FLASH_Lock();//上锁
    return SUCCESS;
}

int32_t dbLoad(void)
{
    load(DB_MASTER);
    load(DB_BUCKUP);
    return SUCCESS;
}

int32_t dbSave(void)
{
    save(DB_MASTER);
    save(DB_BUCKUP);
    return SUCCESS;
}

bool dbCheckAppValid(void)
{
    uint16_t app_valid = ('W' << 8) | 'F';
    if(db_master.app_valid == app_valid || db_bakup.app_valid == app_valid) return true;
    return false;
}

int32_t dbSetAppValid(uint16_t app_valid)
{
    db_master.app_valid = app_valid;
    db_bakup.app_valid = app_valid;
    dbSave();
    return SUCCESS;
}

uint16_t dbGetAppVersion(void)
{
    uint16_t app_valid = ('W' << 8) | 'F';
    if(db_master.app_valid == app_valid) return db_master.app_version;
    if(db_bakup.app_valid == app_valid) return db_bakup.app_version;
    return 0;
}

int32_t dbSetAppVersion(uint16_t app_version)
{
    db_master.app_version = app_version;
    db_bakup.app_version = app_version;
    return SUCCESS;
}
