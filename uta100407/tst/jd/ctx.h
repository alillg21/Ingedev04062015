#ifndef __CTX_H
#define __CTX_H

#undef __TCJD0177__
#undef __TCJD0040__
#undef __TCJD32__

#define __TCJD32__
#define __TCJD0040__
#define __TCJD0177__

#define __GMA__
#define __INGEDEV__ 6

#define __TEST__
#define JP_DEBUG
#define __TC_APP__  // We are creating a test case application

// Traces are (or not) sent to hmi (dsp or prt)
#undef __TRC_2_HMI__
#define __TRC_2_HMI__ 1

// Traces are (or not) sent to dsp
#undef __MSG_2_DSP__
#define __MSG_2_DSP__ 0

// Traces are (or not) sent to prt
#undef __MSG_2_PRT__
#define __MSG_2_PRT__ 0


//#define __CANVAS__


#undef __CHN_COM__
#undef __CHN_GPRS__
#undef __CHN_GSM__
#undef __CHN_HDLC__
#undef __CHN_SCK__
#undef __CHN_TCP__
#undef __CHN_FTP__
#undef __CHN_PPP__
#undef __CHN_SERIAL__
#undef __CHN_WIFI__
#undef __CHN_USB__

#ifdef __TCJD0177__
#define __CHN_SCK__
#endif

#ifdef __TCJD0040__
#define __CHN_COM__
#define __CHN_GPRS__
#define __CHN_TCP__
#endif

//#define __CHN_COM__
//#define __CHN_GPRS__
//#define __CHN_GSM__
//#define __CHN_HDLC__
//#define __CHN_SCK__
//#define __CHN_TCP__
//#define __CHN_FTP__
//#define __CHN_PPP__
//#define __CHN_SERIAL__
//#define __CHN_WIFI__
//#define __CHN_USB__

// Plugins to link with
#define __PGN_POR__
#define __PGN_SMF__

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
#undef __DSPH__
#define __DSPH__ 15
#undef __DSPW__
#define __DSPW__ 30

#undef __CNVW__
#define __CNVW__ 240
#undef __CNVH__
#define __CNVH__ 240
#undef __CNVD__
//#define __CNVD__ 2
#define __CNVD__ 8

#define __TFT__

#define __BCR__
#define __ECR__
#define __CANVAS__

//To disable tracing of DRW uncomment the following two lines:
//#undef TRCOPT
//#define TRCOPT 0xFFFFFFFFL & (~BIT(trcDRW))

//To enable tracing of I32 only uncomment the following two lines:
//#undef TRCOPT
//#define TRCOPT 0xFFFFFFFFL & (BIT(trcI32))

#endif
