#ifndef __TVCHANNELPLAN_H__
#define __TVCHANNELPLAN_H__

/*
CP:   Channel Plan
TVCP: TV CP
WF:   Wanfa
std:  STanDard
*/
#define TV_CHANNEL_PLAN_NAME_LEN 16
typedef struct
{
   int no;     // channel no.
   float freq; // unit: MHz
} tvChannel_t;

typedef struct __tvChannelPlan_t
{
   char stdName[TV_CHANNEL_PLAN_NAME_LEN];
   tvChannel_t *tblPtr;
   int len;
} tvChannelPlan_t;

typedef struct
{
   tvChannelPlan_t *pChPlanTbl;
   int cnt;
} tvChannelPlanHdl_t;

tvChannelPlanHdl_t const *WFTVCP_GetEntry(void);

#endif //__TVCHANNELPLAN_H__
