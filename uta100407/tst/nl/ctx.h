#ifndef __CTX_H

#define __TST5PAE__
#define __GMA__

#define __TEST__
//#define __CANVAS__
//#define __CHN_TCP__
#define __CHN_PPP__
//#define __CHN_COM__
#define __CHN_GPRS__
#define __CHN_GSM__
//#define __CHN_USB__
//#define __CHN_HDLC__
//#define __CHN_FTP__
#define __DUKPT__
#define __OWF__
#define __EMV__
//#define __FONT__ Iso8859_35
//#define __FONT__ Iso8859_35_b

#undef __UMAX__
#define __MNUMAX__ 10
//#define __PRTW__ PRT_42COLUMN_SIZE

/* for 8550
#undef __CNVW__
#undef __CNVH__
#undef __CNVD__
#define __CNVW__ 24
#define __CNVH__ 240
#define __CNVD__ 2
*/

//To disable tracing of DRW uncomment the following two lines:
//#undef TRCOPT
//#define TRCOPT 0xFFFFFFFFL & (~BIT(trcDRW))

//To enable tracing of I32 only uncomment the following two lines:
//#undef TRCOPT
//#define TRCOPT 0xFFFFFFFFL & (BIT(trcI32))

#define __CTX_H
#endif
