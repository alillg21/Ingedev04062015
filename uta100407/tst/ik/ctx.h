#ifndef __CTX_H

#define __TST5T__

#define __TEST__
//#define __DEBUG__

#define __INGEDEV__ 6
#define __CANVAS__
#define __CHN_TCP__
#define __CHN_SCK__
//#define __CHN_PPP__
//#define __CHN_GSM__
#define __CHN_GPRS__
//#define __CHN_USB__
//#define __CHN_HDLC__
#define __CHN_FTP__
//#define __DUKPT__
//#define __OWF__
#define __EMV__
//#define __FONT__ Iso8859_35
//#define __FONT__ Iso8859_35_b
//#define __FONT__ Iso8859_2
//#define __FONT__ Iso8859_5
//#define __ECR__
#define __SSA__ 541

#undef __MNUMAX__
#define __MNUMAX__ 100
//#define _ING_SIMULATION

/* for 8550 */

#undef __CNVW__
#undef __CNVH__
#undef __CNVD__
#define __CNVW__ 240
#define __CNVH__ 240
#define __CNVD__ 1
#define __TFT__
#define __BCR__

#undef __DSPH__
#define __DSPH__ 15

#undef __DSPW__
#define __DSPW__ 30
/* */

//tcgs0031
#define __LISTMAX__ 7000

//To disable tracing of DRW uncomment the following two lines:
//#undef TRCOPT
//#define TRCOPT 0xFFFFFFFFL & (~BIT(trcDRW))

//To enable tracing of I32 only uncomment the following two lines:
//#undef TRCOPT
//#define TRCOPT 0xFFFFFFFFL & (BIT(trcI32))

#define __SIGN_CAP__ 1

#define __CTX_H
#endif
