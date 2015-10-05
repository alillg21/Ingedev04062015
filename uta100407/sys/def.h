/** \file
 * Default defines
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/sys/def.h $
 *
 * $Id: def.h 2514 2009-12-02 21:23:55Z mpamintuan $
 */

#ifndef __DEF_H

//Platform specific defines

#define __HMISTD__
#define __CRYSTD__

#ifndef __TELIUM__
#ifndef __UNICAPT__
#define __UNICAPT__
#endif
#endif

//The macro __DEBUG__ is defined in simulation mode
#ifndef __DEBUG__

#ifdef TURBOC
#define __DEBUG__
#endif

#ifdef WIN32
#define __TEST__
#define __DEBUG__
#endif

#ifdef _ING_SIMULATION
#define __TEST__
#define __DEBUG__
#endif

#endif

#ifndef TRCOPT
#define TRCOPT 0xFFFFFFFFL
#endif

#define trcOpt TRCOPT
enum eTrc {
    trcBeg,

    //sys components
    trcTBX,
    trcI32,
    trcHMI,
    trcMEM,
    trcCHN,
    trcGCL,
    trcPGN,
    trcC32,
    trcE32,
    trcEXT,

    //pri components
    trcFMT,
    trcDLG,
    trcCOM,
    trcSTD,
    trcA40,
    trcDBA,
    trcDRV,
    trcDRW,
    trcXML,
    trcSTM,
    trcGUI,

    trcEnd
};

#define BIT(B) (0x01<<(B-1))

#ifndef __TEST__

#ifdef TURBOC
#define __TEST__
#endif

#ifdef WIN32
#define __TEST__
#endif

#endif

#ifndef __DSPH__
#define __DSPH__ 4
#endif

#ifndef __DSPW__
#define __DSPW__ 16
#endif

#ifndef __FNTH__
#define __FNTH__ 16
#endif

#define __DSPINFH__ 4

//Canvas width, height, depth
//#define __CANVAS__
#ifndef __CNVW__
#define __CNVW__ 128
#endif

#ifndef __CNVH__
#define __CNVH__ 64
#endif

#ifndef __CNVD__
#define __CNVD__ 1
#endif

//#define __CNVW__ 240
//#define __CNVH__ 240
//#define __CNVD__ 8
//#define __TFT__

// Font sizes
#define PRT_NORMAL_SIZE   24
#define PRT_DOUBLE_SIZE   12
#define PRT_QDOUBLE_SIZE   6
#define PRT_HALF_SIZE     48
#define PRT_42COLUMN_SIZE 42

//select default printing font
#define __PRTW__ PRT_NORMAL_SIZE

//maximum number of items in the menu
#ifndef __MNUMAX__
//#define __MNUMAX__ 30
#define __MNUMAX__ 40
#endif

//COMMUNICATION CHANNELS ENABLED
#define __CHN_COM__
//#define __CHN_SERIAL__
//#define __CHN_HDLC__
//#define __CHN_TCP__
//#define __CHN_PPP__
//#define __CHN_GSM__
//#define __CHN_GPRS__
//#define __CHN_WIFI__
//#define __CHN_USB__
//#define __CHN_FTP__
//#define __CHN_SCK__

//GMA plugins used
//#define __PGN_GSM__
//#define __PGN_POR__
//#define __PGN_SMF__

//SSA features activated
//#define __DUKPT__
//#define __OWF__
//#define __RSA__
//#define __EMV__

//The variable is defined if the testing is performed from Paris
#define __PARIS__

//Language Setting
#define __ENGLISH__

#ifdef __UNICAPT__
//Master application used
#define __GMA__
#define __COUNTRY__ COM_T35_France
#endif

//individual configurations
#include "ctx.h"

#ifndef __DEBUG_TRC_PATH__
#define __DEBUG_TRC_PATH__ "\\DEBUG.TRC"
#endif

#ifdef __TELIUM__
#ifndef __FONT__
#define __FONT__ "/SYSTEM/COUR.SGN"
#endif
#endif

#ifdef __APR__
#undef __HMISTD__
#define __HMIAPR__
#endif

//Set mandatory defines if it is not done in context definition
#ifdef __CNVD__
#ifndef __FGD__

#if (__CNVD__==1)
#define __FGD__ 1
#define __BGD__ 0
#endif

#if (__CNVD__==8)
#define __FGD__ 0x0
#define __BGD__ 0xF
#endif

#if (__CNVD__==12)
#define __FGD__ 0x0FFF
#define __BGD__ 0x0000
#endif

#endif
#endif

#ifndef __FGD__
#define __FGD__ 1
#define __BGD__ 0
#endif

#ifdef __UNICAPT__
#ifndef __SSA__
#define __SSA__ 517
#endif

//context defines
#ifdef __FONT__
int ssaStarted(void);
int ssaHandle(void);

#include <ssaStd.h>
#include <ssasec.h>
#endif
#endif

#ifdef __EMV__
#define __OWNTAGPRE__ 0xDF
#define __FSBUFLEN__ 262

#ifdef __APR__
#undef __OWNTAGPRE__
#define __OWNTAGPRE__ 0x1F
#endif

#endif

#ifdef __APR__
#undef __HMISTD__
#define __HMIAPR__
#undef __CRYSTD__
#define __CRYAPR__
#endif

#ifndef __IMSI__
#define __IMSI__ "208017101903742"
#endif

#ifndef __IMEI__
#define __IMEI__ "352826003014794"
#endif

#ifndef __SAP__
#define __SAP__ "SAP0123456789012"
#endif

#ifndef __SER__
#define __SER__ "123456789012"
#endif

#define __DEF_H
#endif
