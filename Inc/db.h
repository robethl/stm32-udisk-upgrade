#ifndef _DB_H_
#define _DB_H_

#include "tvchannelplan.h"

typedef enum{
    HDMI = 0,
    CVBS = 1,
#ifdef YPBPR_EN     
    YPBPR = 2,
#endif    
    COLORBAR,
}EncoderSource_e;

typedef enum{
    ASF_32K = 0,
    ASF_44K_1 = 1,
    ASF_48K = 2,
    ASF_AUTO = 3,
}EncoderAudioSF_e;

typedef enum{
    MPEG2 = 0,
    H264 = 1
}EncoderVideoStd_e;

typedef enum{
    MP1L2 = 0,
    AAC = 1
}EncoderAudioStd_e;

typedef enum{
    J83A = 0,
    J83B,
    DVBT,
#ifdef DVBT2_EN
    DVBT2,
#endif    
    ATSC,
    DTMB,
    ISDBT
}ModStd_e;

typedef enum{
    DBM = 0,
    DBUV = 1,
    DBMV = 2
}PowerUnit_e;

typedef enum{
    ALWAYS_ON = 0,
    ALWAYS_OFF = 1,
    AUTO_OFF = 2,
}SystemLcdMode_e;

typedef enum{
    ENGLISH = 0,
}SystemLanguage_e;

typedef enum{
    RCKEY_DIABLE = 0,
    RCKEY_NORMAL = 1,
    RCKEY_BRASIL = 2
}RCKeyType_e;

/*以字读写方式访问内部flash*/
#pragma  pack(push, 4)

#define DB_FLAG_LEN 12
#define DB_FLAG "db.dncm-1x07"  //default值有变动, 版本变更

#define NAME_LEN    (13)
#define USER_CHANNEL_MAX    (32)
#define PASSWORD_LEN    (4)
#define INVALID_CHANNEL (-1)

#define SUPPER_PASSWORD "root"
#define DEFAULT_PASSWORD "0000"

#define USER_CH_FREQ_ACTION_CANCEL (-1)
#define USER_CH_FREQ_ACTION_NOP    (0)
#define USER_CH_FREQ_ACTION_EDIT   (1)
#define USER_CH_FREQ_ACTION_ADD    (2)

typedef struct
{
    int32_t action;
    int32_t no;
    int32_t freq;
} UserChannelFrequency_t, *pUserChannelFrequency_t;

typedef struct{
    uint8_t source;
    struct{
        uint8_t std;
        uint8_t rc_mode;
        int32_t coderate;   //kb
    } video;
    struct{
        uint8_t std;
        uint8_t sf;
    } audio;
    struct{
        struct{
            uint8_t name[NAME_LEN];
            uint8_t len;
        } service;
        struct{
            uint8_t name[NAME_LEN];
            uint8_t len;
        } provider;
        int32_t sid;
        int32_t brasil_sid;
        int32_t pmt_pid;
        int32_t pcr_pid;
        int32_t video_pid;
        int32_t audio_pid;
        int32_t lcn;
        int32_t rc_key_id;
    } proginfo;
}Encoder_t, *pEncoder_t;

typedef struct{
    int32_t ts_id;
    int32_t ori_network_id;
    struct{
        int32_t network_id; 
        int32_t version;
        struct{
            uint8_t name[NAME_LEN];
            uint8_t len;
        } network;
    } nit;
    uint8_t lcn_insert; /*0:disable 1:dvb 2:NorDig v1 3:NorDig v2 4:dvb+NorDig v2 4:NorDig v1+v2*/
    uint8_t vct_insert; /*0:disable 1:ATSC(8 VSB)-4 j83b 64QAM(SCTE_mode_1) 256QAM(SCTE_mode_2)*/
    uint8_t rckey_insert;   /*0:disable 1:normal 2:Brasil*/
}Stream_t, *pStream_t;

typedef struct{
    uint8_t std;
    struct{
        struct{
            uint8_t constellation;  /*16-QAM 32-QAM 64-QAM 128-QAM 256-QAM*/
            int32_t symbolrate; //KS/s
        } j83a;
        struct{
            uint8_t constellation; /*64-QAM 256-QAM*/
        } j83b;
        struct{
            uint8_t constellation; /*QPSK 16-QAM 64-QAM*/
            uint8_t fft;/*2k 4k 8k*/
            uint8_t guardinterval; /*1/4 1/8 1/16 1/32*/
            uint8_t fec;/*1/2 2/3 3/4 5/6 7/8*/
            int32_t bandwidth; //KHz
        } dvbt;
        struct{
            uint8_t l1_constellation; /*BPSK QPSK 16-QAM 64-QAM*/
            uint8_t plp_constellation; /*QPSK 16-QAM 64-QAM 256-QAM*/
            uint8_t fft; /*1k 2k 4k 8k 16k 32k*/
            uint8_t fec;/*1/2 3/5 2/3 3/4 4/5 5/6 1/3 2/5*/
            uint8_t guardinterval; /*1/4 1/8 1/16 1/32 1/128 19/128 19/256*/
            uint8_t pp; /*PP1~PP8*/
            uint8_t fec_type; /*16200 64800*/
            int32_t bandwidth; //KHz
        } dvbt2;
        struct{
            uint8_t constellation; /*QPSK 4-QAMNR 16-QAM 32-QAM 64-QAM */
            uint8_t time_interleaved; /*disable 240 720*/
            uint8_t fec;/*0.4 0.6 0.8*/
            uint8_t carrier_mode; /*3780 1*/
            uint8_t frame_sync; /*420 945 595*/
            int32_t bandwidth; //KHz
        } dtmb;
        struct{
            uint8_t constellation; /*DQPSK QPSK 16-QAM 64-QAM*/
            uint8_t fft;/*2k 8k 4k*/
            uint8_t guardinterval; /*1/32 1/16 1/8 1/4*/
            uint8_t fec;/*1/2 2/3 3/4 5/6 7/8*/
            uint8_t time_interleaved; /*disable TI-1 TI-2 TI-3*/
        } isdbt;
    } param;
    struct{
        struct{
            int32_t last_index;
            int32_t index;
            int32_t last_ch;
            int32_t ch;
        } table;
        int32_t value; //KHz
    } rf_frequency;
    int32_t rf_power;  //dB
    uint8_t rf_power_unit;  /*dBm(dBmW)=dBuV-107;  dBuV=60+dBmV  dBuV*/
    uint8_t rf_en;
}Mod_t, *pMod_t;

typedef struct{
    Encoder_t encoder;
    Stream_t stream;
    Mod_t mod;
}Config_t, *pConfig_t;

typedef struct{
    struct{
        uint8_t mode;   /*0:always on  1:always off  2:auto*/
        int32_t delay;
    } lcd;
    struct{
        uint8_t sum_of_tv_channel;
        tvChannel_t tv_channel[USER_CHANNEL_MAX];
    } user_channel_table;
    uint8_t language;
    uint8_t password[PASSWORD_LEN + 1];
    uint8_t lock_keyboard;
}System_t, *pSystem_t;

typedef struct{
    uint16_t app_valid; /*"WF"*/
    uint16_t app_version;
    uint8_t flag[DB_FLAG_LEN];
    uint8_t sn[16];
    Config_t config;
    System_t system;
    uint32_t crc32; /*flag ~ system*/
}Db_t, *pDb_t;

#pragma pack(pop)

#define DB_SUCCESS              0
#define DB_ERR_RUNNING_PARAM    -1
#define DB_ERR_BACKUP_PARAM     -2

int32_t dbLoad(void);
int32_t dbSave(void);
bool dbCheckAppValid(void);
int32_t dbSetAppValid(uint16_t app_valid);
uint16_t dbGetAppVersion(void);
int32_t dbSetAppVersion(uint16_t app_version);

#endif
