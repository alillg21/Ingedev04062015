#ifndef __CTX_H

#define __APPID__ 0x006E
#undef __GMA__
#undef __APR__

#define __CHN_COM__
#define __CHN_TCP__
#define __CHN_SCK__
#define __CHN_GPRS__
#define __CHN_PPP__

#define __IDLEMSG__ "ISO9ZID\nApp Template"
#define __IDLEAPP__ "SIMAS"

#define __INGEDEV__ 7
#ifndef __TELIUM__
#define __TELIUM__
#endif
#ifdef __UNICAPT__
#undef __UNICAPT__
#endif

#define __FONT__ "/SYSTEM/COUR.SGN"

#define __EMV__
//#define __TEST__

#define __RootKeyLoc__ 126
//#define __appMKeyLoc__ 3
#define __appMKeyLoc__ 17 //@@AS0.0.26 : MasterKey location 17 to 32, WorkingKey location: 1 to 16
#define __appPinKeyLoc__ 1

//#define _TELIUM_2_
#ifdef _TELIUM_2_
#ifdef __DEBUG__
#define SAT_DEBUG
#else //__DEBUG__
#define SAT_PROD
#endif //__DEBUG__
#else //_TELIUM_2_
#ifdef __DEBUG__
#define SST_DEBUG
#else //__DEBUG__
#define SST_PROD
#endif //__DEBUG__
#endif //_TELIUM_2_

#ifdef SST_PROD
//10000604-0036-2E.V03
#define VAR_NUMBER_SIGN		0x2E
#define CARD_NUMBER_SIGN	0x10000604
#define ID_SCR_XXX_BANK		0x002E2030
#define ID_BANK_XXX		0x80000001
#else //SST_RPOD
#ifdef SST_DEBUG
#define VAR_NUMBER_SIGN		0xFA
#define CARD_NUMBER_SIGN	0x0003C009
#define ID_SCR_XXX_BANK		0x00FA2030
#define ID_BANK_XXX		0x80000001
#if 0
#define VAR_NUMBER_SIGN		0xDB
#define CARD_NUMBER_SIGN	0x00010526
#define ID_SCR_XXX_BANK		0x00DB2030
#define ID_BANK_XXX		    0x80000001
#endif //#if 0
#endif //SST_DEBUG
#endif //SST_PROD



#ifdef SAT_PROD
#define VAR_NUMBER_SIGN		0x018D
#define CARD_NUMBER_SIGN	0x10000691
#define ID_SCR_XXX_BANK		0x018D2030
#define ID_BANK_XXX		0x80000001
#else //SAT_PROD
#ifdef SAT_DEBUG
#define VAR_NUMBER_SIGN		0xFA
#define CARD_NUMBER_SIGN	0x0003C009
#define ID_SCR_XXX_BANK		0x00FA2030
#define ID_BANK_XXX		0x80000001
/*
#define VAR_NUMBER_SIGN		0x9A
#define CARD_NUMBER_SIGN	0x0003C00E
#define ID_SCR_XXX_BANK		0x009A2030
#define ID_BANK_XXX		0x80000001
*/
#endif //SAT_DEBUG
#endif //SAT_PROD

#undef __DEBUG_TRC_PATH__
#define __DEBUG_TRC_PATH__ "c:\\DEBUG.TRC"

#define __CTX_H
#endif
