#include "sys.h"
#ifdef __TEST__
#include <string.h>
#include "gcl.h"
#include "gcl_pgsup.h"

#include "tc.h"

// Unitary test case tcjd0177.
// Testing gclSck
// This test will work with GMA 1.71 or above,
//  with packages ver. 3 (includes GSM plugin)

// for LNET
#include <LNet.h>
#include <LNetDns.h>
#include <LNetFtp.h>
#include <LNetSmtp.h>
#include <LNetTcpip.h>
#include <LNetSocket.h>

static int tsMin, tsMean, tsMax;

#define __DVK_ADR__          "82.247.161.69|6789"
#define __COMTECH_ADR__      "91.121.18.221|6789"
#define __FUTURA_PLUS_ADR__  "172.19.49.10|6789"

#if 0
#define __IP_ADR__ __COMTECH_ADR__
#define __SRV_LABEL__ " COMTECH"
#else
#define __IP_ADR__ __FUTURA_PLUS_ADR__
#define __SRV_LABEL__ " FUTURA+"
#endif

// This function was removed from gcl.c ...
//
int pgsupErrorCheck(int *fnc, int *fRet)
{
	int ret;
	gclPGSUPErrorInfo_t gclErr;

	ret = gclPGSUPGetLastErrorInfo(&gclErr);
	CHECK(ret == RET_OK, lblNoError);

	trcFN("failed PGSUP func = %d\n", gclErr.pgsupFunc);
	trcFN("PGSUP func ret = %d\n", gclErr.ret);
	*fnc = gclErr.pgsupFunc;
	*fRet = gclErr.ret;
	return 1;

lblNoError:
	*fnc = 0;
	*fRet = 0;
	return -1;
}

static int testPing(void){
	int try, ret;

	for (try = 1 ; try < 4 ; try++)
	{
		tmrPause(1);
		tcDspFN("ping %d...", try);
		ret = gclWaitPing();
		
		if (ret >= 0)
			tcDspFN("ping %d done", try);
		else
			tcDspFN("ping %d failed", try);
	}
	
	return 1;
}

static int testStart(byte ctl, const char *tid)
{
	int ret = 0;
    char *lab = NULL;
	const char *init = NULL;
	
	// ctl =
	// 0 local echo SIMELITE, msg "HELLO!"
	// 1 local echo, msg "HELLO!"
	// 2 DVK, bml msg
	// 3 SIMELITE local echo bml, bml msg
	//ctl = 0;
	
	switch (ctl)
	{
		case 0:  // msg = "HELLO!"
		case 3:  // bml msg
			lab = ctl == 0 ? " loc SIM" : " echo bml";
			//init = "127.0.0.1|6789";
			init = "192.168.0.5|2000";
			break;
				
		case 1:  // msg = "HELLO!"
			lab = " echo";
			//init = "10.10.59.27|2000";
			init = "10.2.4.189|6789";
			break;
				
		case 2:  // bml msg
			lab = __SRV_LABEL__;
			init = __IP_ADR__;
			break;
				
		default:
			lab = " CTL ERROR";
			ret = -1;
			break;
	}

	tcDspTitle(lab, tid);

	if (ret == 0)
	{
		tcDspFS(" IP = %s", init);
		ret = utaGclStart(gclSck, init);
	}
	
	return ret;
}


static int appAtr(tBuffer *buf, byte atr, const char *val)
{
    int ret;
	
    VERIFY(buf);
    ret= bufSet(buf, atr, 1);
    ret= bufSet(buf, 3, 1);
    ret= bufAppStr(buf, val);
    ret= bufSet(buf, 0, 1);
    return ret;
}


static int makeMsg(byte ctl, byte *msg, const char *tid)
{
    int ret;
    card len;
    tBuffer buf;
    byte dat[256];
    bufInit(&buf, dat, 256);
    ret= -1;

	switch (ctl)
	{
	    case 0:
        case 1: 
          strcpy(msg, "HELLO!");
          ret = strlen(msg);
          break;
		  
        case 2:
        case 3:
          // x00 x00 x00 3 x01 x01 x04 x00 x85 x0B x03 5 V x00 x05 x03 h a n d s h a k e x00
		  // x1D x03 2 0 0 7 0 6 0 4 1 1 1 0 1 0 x00 x06 x03 1 x00 x0F x03 0 x00 x14 x03 0 x00 x01 
          bufApp(&buf, (byte *) "\x01\x01\x04\x00", 4);  // bml header
          bufSet(&buf, '\x85', 1);  // tag req with attributes
          appAtr(&buf, '\x0B', "5V");  // app
          appAtr(&buf, '\x05', "handshake");  // id
          appAtr(&buf, '\x1D', "20070604111010");  // dt
          appAtr(&buf, '\x06', tid);  // tid
          appAtr(&buf, 0x0F, "0");  // mid
          appAtr(&buf, 0x14, "0");  // aut
          bufSet(&buf, '\x01', 1);  // bmlEND
          
          len = bufLen(&buf);
          num2bin(msg, len, 4);
          memcpy(msg + 4, bufPtr(&buf), bufLen(&buf));
          ret = 4 + bufLen(&buf);
          break;
		  
        default:
          break;
    }
	
    return ret;
}


static int testRecv(byte ctl)
{
    int ret = 0;
    tBuffer buf;
    byte dat[256];
	
    bufInit(&buf, dat, 256);

	switch (ctl)
	{
		case 0:
		case 1: 
			ret = utaGclRecvBuf(&buf, (byte *) "!", 20);
			CHECK(ret == 6, lblKO);
			CHECK(memcmp("HELLO!", bufPtr(&buf), 6) == 0, lblKO);
			break;
		  
        case 2:
        case 3:
		    bufInit(&buf, dat, 4);
			ret = utaGclRecvBuf(&buf, 0, 200);  // 200s timeout
			CHECK(ret == 4, lblKO);
			break;
			
        default:
			break;
    }
	
	return ret;
	
  lblKO:
	return -1;
}

// snd = 0, rcv = 0 : close connection immediately after opening
// snd = 0, rcv = 1 : connect, then go to the receive
//      state => the server closes the connection on timeout 
// snd = 1, rcv = 1 : perform send and receive = full handshake
//
void tcjd0177(void)
{
	uint8 connected;
	int tsConnected = 0;
	uint32 localIpAddr, dns1, dns2;
	static int savErr;
	int fnc, ret = 0, esc;
	byte len;
    byte tmp[256];
	byte rssi, gprsStat;
	char *errMsg;
	int iter;
	int snd;
	int rcv;
	const char *tId;
	byte ctl = 2;  // Topup Handshake
	
	tId = "111";
	snd = tcIntParam();
	rcv = tcIntParam();
	iter = tcLoop();	
    ret = testStart(ctl, tId); CHECK(ret >= 0, lblKO);

	// (needs gmaLibPgComm lib)
	ret = gmaInqGsmSignLevel(&rssi, &gprsStat);
	if (ret == RET_OK)
		tcDspF2N(" rssi = %d, gprsStat = %d", rssi, gprsStat);
	else
		tcDspFN("gmaInqGsmSignLevel ret = %d", ret);

	// reset the error info before communication attempt
	gclPGSUPSerrorReset();

	tcDspS(" utaGclDial..."); 
    ret = utaGclDial(); CHECK(ret >= 0,lblKO);
	tcDspPS(" utaGclDial OK"); 
	esc = 0;
	(void) getTS(0); // set starting time
	
	do 
	{
		psyTimerWakeAfter(15);
		ret = utaGclDialInfo();

		switch (ret)
		{
			case gclStaPreDial:  tcDspS(" PreDial"); break;								 
			case gclStaDial:     tcDspS(" Dial");    break;
			case gclStaConnect:  tcDspS(" Connect"); break;
			case gclStaLogin:    tcDspS(" Login");   break;
								 
			case gclStaFinished: tsConnected = getTS(1);
								 tcDspFN(" Connected in %d hs", tsConnected);
								 break;
								 
			default:             esc = 1;  // error : break the loop
								 tcDspFN(" dial info = %d", ret);
								 break;
		}

		if (!esc)
			esc = tcCancel(); // user cancelled ?

		CHECK(!esc, lblEnd);
		
	} while (ret != gclStaFinished);

	CHECK(ret == gclStaFinished, lblKO);

	tcConnectInfo(&connected, &localIpAddr, &dns1, &dns2);
	
	if (iter == 1)
	{	// Reset the time stamp
		tsMin = tsMean = tsMax = tsConnected;
	}
	else
	{
		if (tsConnected < tsMin)
			tsMin = tsConnected;

		if (tsConnected > tsMax)
			tsMax = tsConnected;

		tsMean += (tsConnected - tsMean) / iter;

		sprintf(tcDspBuf(), " min=%d, MAX=%d, Mean=%d", tsMin, tsMax, tsMean);
		tcDspS(NULL);
	}

	if (snd)	// perform send...
	{
		tcDspS(" utaGclSend...");
		ret = makeMsg(ctl, tmp, tId);  // tId = handshake tid
		len = (byte) ret;
//		tmrSleep(30);  // Small pause to let gateway react
		ret = utaGclSendBuf(tmp, len);
		tcDspPS(" utaGclSend OK");
	}
	
	if (rcv)	// perform receive...
	{
		tcDspS(" utaGclRecv...");
		(void) getTS(0);  // set starting time
		ret = testRecv(ctl);
		CHECK(ret > 0, lblKO);
		tcDspPS(" utaGclRecv OK");
		sprintf(tmp, " Reception lasted %dhs", getTS(1));
		tcDspS(tmp);
	}
	
	//testPing();
	ret = 0;
    goto lblEnd;

lblKO:
    trcErr(ret);
    ret = utaGclDialErr();
	errMsg = 0;
	
	switch (ret)
	{
		// GCL errors
		case gclErrCancel:     errMsg = " User cancelled"; break;
		case gclErrDial:       errMsg = " Dial error";     break;
		case gclErrBusy:       errMsg = " Busy";           break;
		case gclErrNoDialTone: errMsg = " No dial tone";   break;
		case gclErrNoCarrier:  errMsg = " No carrier";     break;
		case gclErrNoAnswer:   errMsg = " No answer";      break;
		case gclErrLogin:      errMsg = " Login pb";       break;
		default: // (gclErrEnd)
			ret = gclLastError();
			tcDspFN("utaGclDialErr = %d", ret);
			tcDecodeGclError(ret);
			
			// Now check GSM plugin errors
			if (pgsupErrorCheck(&fnc, &ret) != -1)
			{
				tcDspFN(" PGN fnc = %d", fnc);
				tcDecodeGclError(ret);
			}
			break;
	}

	if (errMsg)
		tcDspS(errMsg);

	if (ret != savErr)  // This is a new error
	{
		tcPrt(0);
		tcPrtDateTime("at:");
		savErr = ret;
	}

	ret = -1;
	
lblEnd:
	utaGclStop();

	if (iter % 3 == 0)  // every 3 iterations, disconnect from GPRS
	{
		tcDspS(" Detach...");
		tmrPause(2);
		gmaInqGsmGprsDiscon(1);
	}
	else
		tcDspS(" Close...");
	
	tcSetRet(ret);
//	tcSetRet(0);  // Avoids being paused by tclib in case of error
}

#endif  // def __TEST__
