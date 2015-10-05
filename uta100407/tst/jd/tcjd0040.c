#include <string.h>
#include "sys.h"
#include "tc.h"
#include "gsm.h"  // transitory

// Integration test case tcjd0040
// GPRS connection with chn
// Unitary test.

// This test will work with GMA 1.71 or above,
// with packages ver.1 (does not include GSM plugin)

#if 1  // Comtech
// CHECK PIN CODE BEFORE LAUNCHING TEST !
// JP test sim
#define __INIT__   "0000|orange.fr|danfr|mq36noqt|"
// Milena test sim  #define __INIT__   "3421|gprsinternet|mts|064|"
// topup France
#define __IP_ADR__ "91.121.18.221|6789"
//"212.027.042.012|10"; //free.fr wrong port for testing
//"212.27.42.12|110"; //free.fr

#else  // Futura+
// CHECK PIN CODE BEFORE LAUNCHING TEST !
// JP test sim in Serbia
#define __INIT__   "8269|futuraplus|mts|064|"
// JP test sim for T5 #define __INIT__   "7110|futuraplus|mts|064|"
// Milena test sim in Serbia #define __INIT__   "3421|futuraplus|mts|064|"
#define __IP_ADR__ "172.19.49.10|6789"    // topup Serbie

#endif

void tcjd0040(void)  // GPRS send msg
{
	int ret;
	char *ptr;
	tBuffer buf;
	byte dat[1024];

	bufInit(&buf, dat, sizeof(dat));
	bufReset(&buf);

	tcDspModeSet(SCR_BY_SCR);
	tcDspTitle("TCJD40", "GPRS send msg");
	tcDspFS(" IP = %s", __IP_ADR__);
	tcDspFS(" INIT = %s", __INIT__);
	
	// GPRS Connection
	tcDspS("comStart...");
	ret = comStart(chnGprs); // SIM not initialised yet
	CHECK(ret >= 0, lblKO);
	ret = tcDspPS("comStart OK");

	ptr = __INIT__;
	tcDspFS("GPRS init = \"%s\"", ptr);
	ret = tcDspS("comSet...");
	ret = comSet(ptr);
	CHECK(ret >= 0, lblKO);
	ret = tcDspPS("comSet OK");

	CHECK(ret >= 0, lblKO);
	tcDspS("DialIP ...");

	ptr = __IP_ADR__;
	
	tcDspFS("comDial(%s)", ptr);
	ret = comDial(ptr);
	CHECK(ret >= 0, lblKO);
	tcDspPS("DialIP OK");
	
	bufApp(&buf,(byte *)"\x00\x00\x00\x0A\x01\x01\x04\x00\x85\x05\x03\33\x00\x01",14);
	
	tcDspS("utaGprsSend..."); CHECK(ret>=0,lblKO);
	ret= comSendBuf(bufPtr(&buf),bufLen(&buf));
	tcDspPS("utaGprsSend OK");

	bufReset(&buf);
	ret= comRecvBuf(&buf,0,100);
	CHECK(ret>=0,lblKO);
	tcDspS("Receiving done");
	CHECK(ret>=0,lblKO);
	goto lblEnd;

  lblKO:
	trcErr(ret);
    tcDspS("Connection Fail");
	tcDecodeGsmError(ret);
    goto lblEnd;
  lblEnd:
	tcDspS("GPRS: End\n");
	ret = comHangStart();
	tcDspFN("comHangStart : ret=%d\n", ret);
	ret = comHangWait();
	tcDspFN("comHangWait : ret=%d\n", ret);
	ret = comStop();
	tcDspFN("comStop : ret=%d\n", ret);
}

