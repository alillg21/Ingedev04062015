#ifndef __CTX_H

#define __TSTAR32__

#define __GMA__
#define __TEST__
#define __INGEDEV__ 6

//#define __CANVAS__
//#define __CHN_TCP__
//#define __CHN_PPP__
//#define __CHN_GPRS__
//#define __CHN_USB__
//#define __CHN_HDLC__
//#define __CHN_FTP__
//#define __DUKPT__
//#define __OWF__
//#define __EMV__
//#define __FONT__ Iso8859_35
//#define __FONT__ Iso8859_35_b
//#define __FONT__ Iso8859_2
//#define __FONT__ Iso8859_5
#define __SSA__ 517
//#undef __MNUMAX__
//#define __MNUMAX__ 100
//#define _ING_SIMULATION

/* for 8550 */
#undef __CNVW__
#undef __CNVH__
#undef __CNVD__
#define __CNVW__ 240
#define __CNVH__ 240
#define __CNVD__ 2

#define __TFT__
//#define __BCR__

#define __ECR__

//To disable tracing of DRW uncomment the following two lines:
//#undef TRCOPT
//#define TRCOPT 0xFFFFFFFFL & (~BIT(trcDRW))

//To enable tracing of I32 only uncomment the following two lines:
//#undef TRCOPT
//#define TRCOPT 0xFFFFFFFFL & (BIT(trcI32))

#define __CTX_H
#endif
