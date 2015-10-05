#include <sys.h>

#ifdef __TEST__

#include <string.h>
#include "tc.h"
#include "gcl.h"
#include "gcl_pgsup.h"
#include "gmaInqStatus.h"
#include "gsm.h"

#define __MAX_H__ (__DSPH__- 1)
#define __MAX_W__ (__DSPW__- 1)

// static state data
//
static char dspBuf[TC_BUF_LEN];
static char tmpBuf[TC_BUF_LEN];
static char title[__DSPW__];
static unsigned int ln, ln1, oldLn, tcOldLn;
static int stepCnt, steps;
static int showTi;
static int cnfByUser;
//static int maxSeq;		// max iterations for a sequence of tc
static int curSeq;		// current iteration in a sequence of tc
static int maxIter;		// max iterations for a single tc (inside a sequence of tc)
static int curIter;     // current iteration of a single tc (inside a sequence of tc)
static int pause, wait;
static int cancelled;
static card dspMode;
static tcCfg_t *tcPtr;
static int paramIdx;
static int retCode;
static int delayed, delayedLen;

// Declarations
//
static void infoMsg(const char *info);
static void setCancel(void);
static void cnfDef(void);
static void cnfMnu(void);
static tcCfg_t *cnfAuto(mnuCfg_t *item);
static void tcBeep(int param);
static char waitKey(int timo, checkEntry_t chk);
static void kbdCheck(void);
static char tcConfirm(char *msg);
static void tcClear(void);
static void pushLn(void);
static void popLn(void);
static void dspLines(char *dspPtr);
static void dspMultiLn(char *dspPtr);

// Functions for the redirection of the traces to the display
int trcDspS(const char *str);
int trcDspAN(const unsigned char *buf, int len);
int trcDspFN(const char *fmt, int val);
int trcDspFS(const char *fmt, const char *str);

static void runAuto(tcCfg_t *tcTab, int seq);
static int runMenu(mnuCfg_t *mnu);
void selectMain(void);

/*************************************
GSM errors reported by gsmReadError
when the modem reported a GSM_CME_ERROR

0 : Phone failure
1 : No connection to phone (*1)
2 : Phone adapter link reserved (*1)
3 : Operation not allowed
4 : Operation not supported
5 : PH-SIM PIN required (SIM lock)
10 : SIM not inserted
11 : SIM PIN required
12 : SIM PUK required
13 : SIM failure
14 : SIM busy (*1)
15 : SIM wrong (*1)
16 : Incorrect password
17 : SIM PIN2 required
18 : SIM PUK2 required
20 : Memory full
21 : Invalid index
22 : Not found
23 : Memory failure (*1)
24 : Text string too long
25 : Invalid characters in text string (*1)
26 : Dial string too long
27 : Invalid characters in dial string
30 : No network service
31 : Network timeout (*1)
32 : Network not allowed - emergency calls only (*2)
40 : Network personalization PIN required (Network lock) (*2)
100 : Unknown (*1)
103 : Illegal MS (#3) (*2)
106 : Illegal ME (#6) (*2)
107 : GPRS services not allowed (#7) (*2)
111 : PLMN not allowed (#11) (*2)
112 : Location area not allowed (#12) (*2)
113 : Roaming not allowed in this location area (#13) (*2)
132 : service option not supported (#32) (*2)
133 : requested service option not subscribed (#33) (*2)
134 : service option temporary out of order (#34) (*2)
148 : unspecified GPRS error (*2)
149 : PDP authentication failure (*2)
150 : invalid mobile class (*2)
500 : unknown error (*2)
515 : Please wait, init or command processing in progress (*2)
517 : SIM toolkit facility not supported (*2)
518 : SIM toolkit indication not received (*2)
519 : Reset the product to activate or change a new echo cancelation algo (*2)
520 : Automatic abort about get PLMN list for an incoming call (*2)
526 : PIN deactivation forbidden with this SIM card (*2)
527 : Please wait, RR or MM is busy. retry your selection later (*2)
528 : Location update failure, emergency calls only (*2)
529 : PLMN selection failure, emergency calls only (*2)
533 : Missing or unknown APN (*2)
			  (1) not available on WAVECOM module
					  (2) not available on MMG module
Nota: some new codes are subject to appear depending on release of software or if new modules are introduced. If so, contact the support team in order to get the definition of the code.
******************************************/

/*****************************************
ftpRetrieve() returns :

FTP_ERR_INVALID_FTP = Specified Ftp handle not found = -12544
NET_ERR_WRONG_APP = Calling application was not owner of the handle = -12295
NET_EINVAL = Invalid arguments = -26
******************************************/

// Return the next parameter as a string,
// from the parameter array defined in tcapp.c
const char *tcStrParam(void)
{
	const char *parm;
	
	VERIFY(tcPtr);
	VERIFY(paramIdx < PARAM_NUM);
	parm = tcPtr->param[paramIdx];
	paramIdx++;
	return parm ? parm : "";
}


// Return the next parameter as an integer,
// from the parameter array defined in tcapp.c
int tcIntParam(void)
{
	return atoi(tcStrParam());
}

// Set 'ret' as the return code for the test case,
// as read in tclib by tcGetRet()
void tcSetRet(int ret)
{
	retCode = ret;
}

// Get the return code internally set by the test case
static int tcGetRet(void)
{
	return retCode;
}

static void tcReset(void)
{
	paramIdx = 0;
	tcSetRet(0);
}


void tcSerial(const char *str)
{
//	(void) ecrSnd(str, strlen(str));
}


// Put ipVal into ip in the form "XX.XX.XX.XX"
void tcDecodeIpAdrs(char *ip, int ipVal)
{
	unsigned char *ipStr = (char *) &ipVal;
	
	sprintf(ip, "%d.%d.%d.%d", ipStr[0], ipStr[1], ipStr[2], ipStr[3]);
//	struct in_addr *addr = (struct in_addr *) &ipVal;
//	inet_ntoa(*addr, ip);
}

int tcConnectInfo(byte *connected, unsigned long *localIpAddr,
				  unsigned long *dns1, unsigned long *dns2)
{
	int ret;
	unsigned char *ipStr;
	char buf[50], *bufPtr = buf;

	ret = gmaInqGsmGprsInfo(connected, localIpAddr, dns1, dns2);
	CHECK(ret == 0, lblKO);

	if (connected)
	{
		ret = sprintf(bufPtr, "connected = %d ", *connected);
		bufPtr += ret;
	}

	if (localIpAddr)
	{
		ipStr = (char *) localIpAddr;
		ret = sprintf(bufPtr, "IP = %d.%d.%d.%d ", ipStr[0], ipStr[1], ipStr[2], ipStr[3]);
		bufPtr += ret;
	}

	if (dns1)
	{
		ipStr = (char *) dns1;
		ret = sprintf(bufPtr , "DNS1 = %d.%d.%d.%d ", ipStr[0], ipStr[1], ipStr[2], ipStr[3]);
		bufPtr += ret;
	}

	if (dns2)
	{
		ipStr = (char *) dns2;
		ret = sprintf(buf, "DNS2 = %d.%d.%d.%d", ipStr[0], ipStr[1], ipStr[2], ipStr[3]);
		bufPtr += ret;
	}

	tcDspS(buf);
	tcPrt(buf);
	return 1;

lblKO:
	return -1;
}


int tcDecodeComError(int err)
{
	int ret = 1;
	char *errMsg = "Error ?";

	switch (err)
	{
		case ERR_CONFLICT:  // = 
			errMsg = "ERR_CONFLICT";
			break;

		case ERR_SYS_RESOURCE_PB:  // -
			errMsg = "ERR_SYS_RESOURCE_PB";
			break;

		case ERR_UNKNOWN_CHANNEL:  //  -
			errMsg = "ERR_UNKNOWN_CHANNEL";
			break;

		case ERR_INVALID_PARAMETER:  // -
			errMsg = "ERR_INVALID_PARAMETER";
			break;

		case COM_ERR_NO_DYNA_DATA:  // - : the PCOM initialization failed,
			// no dynamic data have been retrieved, no channel can be opened
			errMsg = "COM_ERR_NO_DYNA_DATA";
			break;

		default:
			ret = 0;
			break;
	}

	sprintf(tmpBuf, " %s = %d", errMsg, err);
	return tcDspS(tmpBuf);
}


// Decode GSM error err,
// writing symbolic value for this error into errBuf
// (GSM_ERROR_START = -13312)
int tcDecodeGsmError(int err)
{
	int ret = 1;
	char *errMsg = "Error ?";

	switch (err)
	{
		case GSM_ERR_SEMAPHORE:  // = GSM_ERROR_START = -13312
			errMsg = "ERR_SEMAPHORE";
			break;

		case GSM_ERR_HANDLE:  // -13313
			errMsg = "ERR_HANDLE";
			break;

		case GSM_ERR_OPEN:  //  -13314
			errMsg = "ERR_OPEN";
			break;

		case GSM_ERROR:  // -13315 
			errMsg = "ERROR";
			break;

		case GSM_CMS_ERROR:  // -13316
			errMsg = "CMS_ERROR";
			break;

		case GSM_CME_ERROR:  // -13317
			errMsg = "CME_ERROR";
			break;

		case GSM_SIGNAL_NOT_READ:  // -13318
			errMsg = "SIGNAL_NOT_READ";
			break;

		case GSM_NOT_AVAILABLE:  // -13319
			errMsg = "NOT_AVAILABLE";
			break;

		default:
			ret = 0;
			break;
	}

	sprintf(tmpBuf, " GSM_%s = %d", errMsg, err);
	return tcDspS(tmpBuf);
}

// Decode GCL error err,
// writing symbolic value for this error into errBuf
// (GCL_ERROR_BASE = -14848)
int tcDecodeGclError(int err)
{
	int ret = 1;
	char *gclErr = 0;
	char *pgnErr = 0;

	switch (err)
	{
		case GCL_ERR_CONN_LIST_FULL:  // = GCL_ERROR_BASE - 1 = -14849
			gclErr = "CONN_LIST_FULL";
			break;

		case GCL_ERR_CONN_LIST_EMPTY: // -14850
			gclErr = "CONN_LIST_EMPTY";
			break;

		case GCL_ERR_TASK_CREATE:  // -14851
			gclErr = "TASK_CREATE";
			break;

		case GCL_ERR_OPEN_DEVICE:  // -14852
			gclErr = "OPEN_DEVICE";
			break;

		case GCL_ERR_CONFIG:  // -14854
			gclErr = "CONFIG";
			break;

		case GCL_ERR_NOT_CONNECTED:  // -14856
			gclErr = "NOT_CONNECTED";
			break;

		case GCL_ERR_NOT_STARTED:  // -14857
			gclErr = "NOT_STARTED";
			break;

		case GCL_ERR_TIMEOUT:  // -14858
			gclErr = "TIMEOUT";
			break;

		case GCL_ERR_BUF_OVERFLOW:  // -14859
			gclErr = "BUF_OVERFLOW";
			break;

		case GCL_ERR_SEND:  // -14861
			gclErr = "SEND";
			break;

		case GCL_ERR_RECEIVE:  // -14862
			gclErr = "RECEIVE";
			break;

		case GCL_ERR_ALREADY_STARTED:  // -14863
			gclErr = "ALREADY_STARTED";
			break;

		case GCL_ERR_LOGIN:  // -14864
			gclErr = "LOGIN";
			break;

		case GCL_ERR_NO_CARRIER:  // -14866
			gclErr = "NO_CARRIER";
			break;

		case GCL_ERR_NO_DIALTONE:  // -14867
			gclErr = "NO_DIALTONE";
			break;

		case GCL_ERR_NO_ANSWER:  // -14868
			gclErr = "NO_ANSWER";
			break;

		case GCL_ERR_CANCEL_BY_USER:  // -14869
			gclErr = "CANCEL_BY_USER";
			break;

		case GCL_ERR_BUSY:  // -14870
			gclErr = "BUSY";
			break;

		case GCL_ERR_INTERNAL_ERR:  // -14871
			gclErr = "INTERNAL_ERR";
			break;

		case GCL_ERR_TASK_CLOSE:  // -14876
			gclErr = "TASK_CLOSE";
			break;

		case GCL_ERR_CONNECTED:  // -14877
			gclErr = "CONNECTED";
			break;

		case GCL_ERR_LINE_IN_USE:  // -14879
			gclErr = "LINE_IN_USE";
			break;

		case GCL_ERR_LINE_FREE:  // -14880
			gclErr = "LINE_FREE";
			break;

		case GCL_ERR_LINE_UNPLUGGED:  // -14881
			gclErr = "LINE_UNPLUGGED";
			break;

		case GCL_ERR_LINE_IN_USE_OR_UNPLUGGED:  // -14882
			gclErr = "LINE_IN_USE_OR_UNPLUGGED";
			break;

		case GCL_ERR_DELAYED:  // -14883
			gclErr = "DELAYED";
			break;

		case GCL_ERR_BLACKLISTED:  // -14884
			gclErr = "BLACKLISTED";
			break;

		case GCL_ERR_BLACKLIST_FULL:  // -14885
			gclErr = "BLACKLIST_FULL";
			break;

		case GCL_ERR_CONN_REFUSED:  // -14888
			gclErr = "CONN_REFUSED";
			break;

		case GCL_ERR_GET_IP:  // -14889
			gclErr = "GET_IP";
			break;

		case GCL_ERR_OPEN_SOCKET:  // -14890
			gclErr = "OPEN_SOCKET";
			break;

		case GCL_ERR_SOCKET_CONNECT:  // -14891
			gclErr = "SOCKET_CONNECT";
			break;

		case GCL_ERR_CALLBACK_NOT_DEFINED:  // -14892
			gclErr = "CALLBACK_NOT_DEFINED";
			break;

		case GCL_ERR_DNS:  // -14898
			gclErr = "DNS";
			break;

		case GCL_ERR_ETH_TASK:  // -14899
			gclErr = "ETH_TASK";
			break;

		case GCL_ERR_NO_LNET_INSTALLED:  // -14901
			gclErr = "NO_LNET_INSTALLED";
			break;

		case GCL_ERR_NO_ERROR_INFO_AVAILABLE:  // -14902
			gclErr = "NO_ERROR_INFO_AVAILABLE";
			break;

		case GCL_ERR_USER:  // -14948
			gclErr = "USER";
			break;

		case GCL_PGSUP_ERR_PLUGIN_NOT_CONFIGURED:  // -14988
			pgnErr = "PLUGIN_NOT_CONFIGURED";
			break;

		case GCL_PGSUP_ERR_PG_CONNECT:  // -14989
			pgnErr = "PG_CONNECT";
			break;

		case GCL_PGSUP_ERR_PLUGIN_NOT_FOUND:  // -14990
			pgnErr = "PLUGIN_NOT_FOUND";
			break;

		default:
			ret = 0;
	}

	if (gclErr)
		sprintf(tmpBuf, " GCL_ERR_%s = %d", gclErr, err);
	else if (pgnErr)
		sprintf(tmpBuf, "GCL_PGSUP_ERR_%s = %d", pgnErr, err);
	
	return tcDspS(tmpBuf);
}


// Test the current value of stack pointer (stackPtr).
// If this is a max, print it followed by the
// function name (fnc) where this max was reached
void tcCheckStack(void *stackPtr, const char *fnc)
{
	static card minStack, maxStack;
	card stackVal;

	stackVal = (card) stackPtr;

	if (maxStack == 0)
	{
		// Assume that the first call of tcCheckStack()
		// is done near stack bottom : in gma.c
		minStack = stackVal;
	}
	else if (stackVal < minStack)
	{
		// Never go through this, if first call position is chosen well
		minStack = stackVal;
	}

	if (stackVal > maxStack)
	{
		int len;
		char buf[100];
		maxStack = stackVal;
		len = sprintf(buf, "%d : %s", maxStack - minStack, fnc);
		VERIFY(len < sizeof(buf));
		tcPrt(buf);
	}
}


int tcTimeInterval(void)
{
	static card ts;
	int interval = (int) (getTS(1) - ts);
	ts += interval;	
	return interval;
}


static void infoMsg(const char *info)
{
	dspLS(ln, info);
	Click();
	tmrPause(1);
}


static void setCancel(void)
{
	if (!cancelled)
	{
		cancelled = 1;
		infoMsg(">> Cancelled");
	}
}


int tcCancel(void)
{
	int ret = cancelled;
	cancelled = 0;
	return ret;
}


void tcPrtDateTime(const char *msg)
{
	char dTbuf[12 + 1];
	char prtBuf[100 + 1 + sizeof(dTbuf)];

	VERIFY(strlen(msg) + 1 + sizeof(dTbuf) <= sizeof(prtBuf));
	
	// Get system date and time in format YYMMDDhhmmss.
	getDateTime(dTbuf);
	sprintf(prtBuf, "%s %s", msg, dTbuf);
	tcPrt(prtBuf);
}


int tcDspMode(int option)
{
	return (dspMode & option) ? 1 : 0;
}


card tcDspModeSet(int option)
{
	card oldDspMode = dspMode;

	if (option < 0)
		dspMode &= ~(-option);  // disable option
	else
		dspMode |= option;      // enable option
	
	return oldDspMode;
}


card tcDspModeReset(int option)
{
	card oldDspMode = dspMode;
	
	dspMode = 0;
	(void) tcDspModeSet(option);

	return oldDspMode;
}


static void cnfMnu(void)
{
	int enKbdCheck = tcDspMode(KBD_CHECK);
	int stepByStep = tcDspMode(STP_BY_STP);
	int trc2hmi    = tcDspMode(TRC_2_HMI);
	int msg2prt    = tcDspMode(MSG_2_PRT);
	int msg2ser    = tcDspMode(MSG_2_SER);
	int showTS     = tcDspMode(SHOW_TI);
	int dsp        = 2 - tcDspMode(SCR_BY_SCR);

	tcDspInit(0);
	tcLnFeed(0);
	tcDspS(" CONFIGURATION");
	popLn();
	tcLnFeed(0);
	tcLnFeed(0);

								// minVal, maxVal, timeout (= 0 : no limit)
	if (tcWaitNum("Iterations", &maxIter, 1, 1000, 0) ==__CANCEL_KEY__)
		goto lblEnd;

	if (tcWaitNum("Show traces", &trc2hmi, 0, 1, 0) ==__CANCEL_KEY__)
		goto lblEnd;

	tcDspS(" WAIT: NO=0, EOT=1, ITER=2");

	if (tcWaitNum("Wait mode", &wait, 0, 2, 0) ==__CANCEL_KEY__)
		goto lblEnd;

	if (tcWaitNum("Pause", &pause, 0, 120, 0) ==__CANCEL_KEY__)
		goto lblEnd;

	tcDspS(" DSP: NO=0,SCR.BY.SCR=1,STD=2");
	if (tcWaitNum("Display mode", &dsp, 0, 2, 0) ==__CANCEL_KEY__)
		goto lblEnd;

	if (tcWaitNum("Print", &msg2prt, 0, 1, 0) ==__CANCEL_KEY__)
		goto lblEnd;

	if (tcWaitNum("Serial", &msg2ser, 0, 1, 0) ==__CANCEL_KEY__)
		goto lblEnd;

	if (tcWaitNum("Time Interval", &showTS, 0, 1, 0) ==__CANCEL_KEY__)
		goto lblEnd;

	if (tcWaitNum("Kbd check", &enKbdCheck, 0, 1, 0) ==__CANCEL_KEY__)
		goto lblEnd;

  lblEnd:
	cnfByUser = 1;
	tcDspModeSet(enKbdCheck ? KBD_CHECK : -KBD_CHECK);
	tcDspModeSet(stepByStep ? STP_BY_STP : -STP_BY_STP);
	tcDspModeSet(trc2hmi ? TRC_2_HMI : -TRC_2_HMI);
	tcDspModeSet(msg2prt ? MSG_2_PRT : -MSG_2_PRT);
	tcDspModeSet(msg2ser ? MSG_2_SER : -MSG_2_SER);
	tcDspModeSet(showTS ? SHOW_TI : -SHOW_TI);

	switch(dsp)
	{
		case 0:
			tcDspModeSet(-DSP_ENABLE);
			break;

		case 1:
			tcDspModeSet(DSP_ENABLE);
			tcDspModeSet(SCR_BY_SCR);
			break;

		case 2:
			tcDspModeSet(DSP_ENABLE);
			tcDspModeSet(-SCR_BY_SCR);
			break;
	}

	tmrPause(1);  // To have a last glance before Dsp clearing
	tcDspInit(0);
}


//
// timo > 0 : timeout controlled keyboard entry
// timo = 0 : keyboard entry with no time limitation
// timo < 0 : no wait, just read kbd on the fly
//
char waitKey(int timo, checkEntry_t chk)
{
	char key;

	kbdStart(0);  // do not flush kbd buffer

	if (timo > 0)
	{
		tmrStart(0, timo);  // start timer 0 for timo hsec
	}

	do
	{
		if (timo > 0 && !tmrGet(0))
		{
			key = 0;  // timed out => return 0
			break;
		}

		key = kbdKey();

	} while (timo >= 0 && !key);

	kbdStop();

	if (timo > 0)
	{
		tmrStop(0);
	}

	if (key)
	{
		tcBeep(0);
	}

	if (chk == DO_CHECK)
	{
		int waitAgain = 0;

		switch (key)
		{
			case __00_KEY__:
				cnfMnu();
				break;

			case'1':case'2':case'3':case'4':
			case'5':case'6':case'7':case'8':
			case'9':
				if (!tcDspMode(IN_MENU))
				{
					tcDspModeSet(-SCR_BY_SCR);
					tcDspModeSet(STP_BY_STP);
					stepCnt = steps = key - '0';
					waitAgain = 1;
				}
				break;

			case '0':
				if (!tcDspMode(IN_MENU) && tcDspMode(STP_BY_STP))
				{
					tcDspModeSet(-STP_BY_STP);
					infoMsg(">> Step by step disabled");
					waitAgain = 1;
				}
				break;

			case  __CANCEL_KEY__:
				setCancel();
				break;

			case __VALID_KEY__ :
				tcBeep(__VALID_KEY__);
				break;

			case __DEL_KEY__ :
				tcBeep(__DEL_KEY__);
				break;

			default :
				break;
		}	

		if (waitAgain)
		{
			key = waitKey(timo, NO_CHECK);
		}
	}

	return key;
}


void dspSeparator(char fill)
{
	if (tcDspMode(SCR_BY_SCR))
	{
		if (ln == __MAX_H__)
		{
			tcWait("more...", 0);
		}
	}
	else if (ln < __MAX_H__)
	{
		if (fill == 0 || fill == ' ')
		{
			dspLS(ln, "");
		}
		else
		{
			char fillBuf[__DSPW__ / 2];
			memset(fillBuf, fill, sizeof(fillBuf) - 1);
			fillBuf[sizeof(fillBuf) - 1] = 0;
			dspLS(ln, fillBuf);

			if (ln + 1 < __MAX_H__)
			{
				dspLS(ln + 1, "");
			}
		}
	}
}


static void lnFeed(void)
{
	if (ln < __MAX_H__)
	{
		ln++;
	}
	else
	{
		ln = ln1;
	}
}


void tcLnFeed(char fill)
{
	lnFeed();
	dspSeparator(fill);
}


static void pushLn(void)
{
	oldLn = ln;
}


static void popLn(void)
{
	ln = oldLn;
}


// The public version of pushLn
void tcPushLn(void)
{
	tcOldLn = ln;
}


// The public version of popLn
void tcPopLn(void)
{
	ln = tcOldLn;
}


static void tcBeep(int param)
{
	char *ptr = NULL;

	switch (param)
	{
		case __VALID_KEY__ :
			if (!tcDspMode(BEEP_ON))
			{
				ptr = ">> Beep on";
				tcDspModeSet(BEEP_ON);
			}
			
			break;
			
		case __DEL_KEY__ :
			if (tcDspMode(BEEP_ON))
			{
				ptr = ">> Beep off";
				tcDspModeSet(-BEEP_ON);
			}

			break;

		default :
			if (param < 0)  // force the click(s)
			{
				while (param++ < 0)
					Click();
			}
			else if (tcDspMode(BEEP_ON))
			{
				Click();
			}
	}
	
	if (ptr)
	{
		infoMsg(ptr);
	}
}


int tcLoop(void)
{
	return ((curSeq - 1) * maxIter) + curIter;
}


void tcDspInit(unsigned int lnVal)
{
	VERIFY(lnVal < __DSPH__);
	ln1 = lnVal;
	dspWdt(__DSPW__);
	dspHgt(__DSPH__);
	tcClear();
}

/*******
static void tcStop(void)
{
	prtStop();
	dspStop();
}
*******/

static void tcClear(void)
{
	dspClear();
	ln = ln1;
	pushLn();
	tcPushLn();
}


char *tcDspBuf(void)
{
	return dspBuf;
}


static void dspLines(char *dspPtr)
{
	int len, ret;
	
	pushLn();  // Record the present display position

	if (showTi)
	{
		int ti = tcTimeInterval();
		len = sprintf(tmpBuf, "+%d.%2.2d: ", ti / 100, ti % 100);
	}
	else
		len = 0;
	
	while (*dspPtr)	 // Display dspBuf, on multiple lines
	{				 // when dspBuf is longer than the screen
		(void) strncpy(tmpBuf + len, dspPtr, __DSPW__- len);
		tmpBuf[__DSPW__] = 0;

		ret = dspLS(ln, tmpBuf);

		if (ret < 0)
		{
			dspStart();
			dspLS(ln, tmpBuf);
		}

		lnFeed();
		dspPtr += strlen(tmpBuf) - len;
		len = 0;
	}

	showTi = 0;
}


static void dspMultiLn(char *dspPtr)
{
	dspLines(dspPtr);
	dspSeparator('-');
}


static void kbdCheck(void)
{
	if (tcDspMode(STP_BY_STP) && !tcDspMode(SCR_BY_SCR))
	{
		steps--;

		if  (steps == 0)
		{
			char buf[__DSPW__];

			steps = stepCnt;  // reset step count
			sprintf(buf, "%d step(s)", stepCnt);
			(void) tcWait(buf, 0);
		}
	}
	else
	{
		(void) waitKey(1, DO_CHECK); // detect (CANCEL) key on the fly (in 1hs)
	}	
}


int tcDspS(const char *str)
{
	static int iter;
	int len;
	char *itPtr = (char *) 0;
	
	if (str != (char *) 0)
	{
		len = strlen(str);

		if (len == 0)  // void line
		{
			str = " ";
			len = 1;
		}
		else if (str[len - 1] == '\n')
		{   // If string ending is a carriage return, remove it
			len--;  // (for traces redirection)
			//str[len] = 0; may not write into str
			
			if (len == 0)
				return 0;
		}

		VERIFY(delayedLen + len < TC_BUF_LEN);

		if (strncmp(dspBuf + delayedLen, str, len) == 0)  // if same string, redisplay it ...
		{   
			itPtr = strchr(dspBuf + delayedLen, 0);  // (localise the string end)
			sprintf(itPtr, " (%dx)   ", iter++);  // ... with iteration

			if (iter == 30)  // Info about the possibility of cancelling an eventual
			{				 // endless loop (Kb checking is done below)
				lnFeed();
				infoMsg(">> Cancel to stop");
			}
			
			popLn();  // ... return to the same line
		}
		else
		{
			iter = 2;
			strncpy(dspBuf + delayedLen, str, len);
			dspBuf[delayedLen + len] = 0;
		}
	}
	else  // str == 0, means dspBuf was updated before and displayed now
	{
		iter = 2;
		len = 0;
	}

	if (delayed)
	{
		delayedLen += len;
		delayed = 0;
	}
	else
	{
		delayedLen = 0;
		
#ifdef __TRC_2_HMI__
		if (tcDspMode(DSP_ENABLE))
			dspMultiLn(dspBuf);
#endif

#ifdef __MSG_2_PRT__
		if (tcDspMode(MSG_2_PRT))
			tcPrt(dspBuf);
#endif
		
#ifdef __MSG_2_SER__
		if (tcDspMode(MSG_2_SER))
			tcSerial(dspBuf);
#endif
		
		if (itPtr)
			*itPtr = 0;  // remove the iteration in dspBuf

		if (tcDspMode(KBD_CHECK) || iter >= 30)
			kbdCheck();
	}

	return 0;
}


int tcDspDS(const char *str)
{
	delayed = 1;
	return tcDspS(str);
}


int trcDspS(const char *str)
{
	int ret = 0;

	if (tcDspMode(TRC_2_HMI))
	{
		if (tcDspMode(SHOW_TI))
			showTi = 1;

		ret = tcDspS(str);
	}

	return ret;
}


int tcDspAN(const unsigned char *buf, int len)
{
	int ret, idx;
	int insPos, seq, savPos = 0 /* to make gcc happy */;
	unsigned char curC;
	int lastC = 0x100;
	
	for (seq = 1, idx = insPos = 0 ; idx < len ; idx++)
	{
		if (insPos >= sizeof (tmpBuf) - 10)  // We are at the end of the buffer
		{
			if (seq >= 4)  // the last sequence length is at least 3
			{
				// replace the sequence of identical data by the sequence length
				insPos = savPos + sprintf(&tmpBuf[savPos], "(%dx)", seq);
			}
			
			tmpBuf[insPos] = 0;    // Terminate the buffer as a string
			ret = tcDspS(tmpBuf);  // display this full buffer
			if (tcCancel())        // If cancel was pressed, stop displaying the buffer
				return ret;
			else
				return tcDspAN(buf + idx, len - idx);
		}

		curC = buf[idx];
		
		if (curC < 0x20 || curC > 0x7F)  // Not an alphanumeric character
		{
			if (curC == lastC)  // is a repeated byte
			{
				seq++;
				
				if (seq == 2) // Beginning of sequence (2 equal char)
				{
					savPos = insPos;  // remember insertion position
				}

				if (seq < 4)
				{
					// insert "\<hex byte>"
					insPos += sprintf(&tmpBuf[insPos], "\\%x", curC);  // insert the byte
				}
			}
			else  // two consecutive different char
			{
				if (seq >= 4)  // a sequence was in course, the length of which being 4 at least
				{
					// replace the sequence of identical data by the sequence length
					insPos = savPos + sprintf(&tmpBuf[savPos], "(%dx)", seq);
					// reinject byte into buffer
					idx--;
				}
				else
				{
					// insert "\<hex byte>"
					insPos += sprintf(&tmpBuf[insPos], "\\%x", curC);  // insert the byte
				}
				
				seq = 1;  // reset sequence length
			}
		}
		else  // an alphanum char
		{
			if (seq >= 4)  // the sequence length is at least 3
			{
				// replace the sequence of identical data by the sequence length
				insPos = savPos + sprintf(&tmpBuf[savPos], "(%dx)", seq);
				// reinject byte into buffer
				idx--;
			}
			else
			{
				tmpBuf[insPos++] = curC;
			}
			
			seq = 1;  // reset sequence length
		}

		lastC = curC;  // remember the byte
	}

	if (seq >= 4)  // the last sequence length is at least 3
	{
		// replace the sequence of identical data by the sequence length
		insPos = savPos + sprintf(&tmpBuf[savPos], "(%dx)", seq);
	}

	tmpBuf[insPos] = 0;  // Terminate the buffer as a string
	
	return tcDspS(tmpBuf);
}


int trcDspAN(const unsigned char *buf, int len)
{
	int ret = 0;
	
	if (tcDspMode(TRC_2_HMI))
	{
		if (tcDspMode(SHOW_TI))
			showTi = 1;

		ret = tcDspAN(buf, len);
	}
	
	return ret;
}


int tcDspPS(const char *str)
{
	popLn();  // current display location is the
			  // previous displayed line location : the
			  // last displayed message will be overwritten
	return tcDspS(str);
}


int tcDspFN(const char *fmt, int val)
{
	sprintf(tmpBuf, fmt, val);
	return tcDspS(tmpBuf);
}


int trcDspFN(const char *fmt, int val)
{
	int ret = 0;

	if (tcDspMode(TRC_2_HMI))
	{
		if (tcDspMode(SHOW_TI))
			showTi = 1;

		ret = tcDspFN(fmt, val);
	}

	return ret;
}


int tcDspF2N(const char *fmt, int val1, int val2)
{
	int ret = 0;

	if (tcDspMode(TRC_2_HMI))
	{
		if (tcDspMode(SHOW_TI))
			showTi = 1;

		sprintf(tmpBuf, fmt, val1, val2);
		ret = tcDspS(tmpBuf);
	}

	return ret;
}


int tcDspFS(const char *fmt, const char *str)
{
	if (str)
		sprintf(tmpBuf, fmt, str);
	else
		sprintf(tmpBuf, fmt, "<null>");

	return tcDspS(tmpBuf);
}


int trcDspFS(const char *fmt, const char *str)
{
	int ret = 0;

	if (tcDspMode(TRC_2_HMI))
	{
		if (tcDspMode(SHOW_TI))
			showTi = 1;

		ret = tcDspFS(fmt, str);
	}

	return ret;
}


int tcDspTitle(const char *tit, const char *subTit)
{
	int len;

	if (tit && tit[0])
	{
		if (subTit)
		{
			sprintf(title, "%s(%s)", tit, subTit);
		}
		else
		{
			strcpy(title, tit);
		}
	}
	// else, use the default title (after menu item label or pS1 parameter)
	
	len = sprintf(tmpBuf, ">> %s", title);

	if (curIter > 1)  // overwrite the previous ending message
	{
		if (tit)
		{
			popLn();  // Go to the last displayed line
			dspSeparator(0);  // clear it
			lnFeed();  // go to next line
		}
		
		sprintf(tmpBuf + len, "(%dx) ", tcLoop());
	}

	tcDspS(tmpBuf);
	
	return waitKey(150, DO_CHECK);  // 1.5s pause, CANCEL detection enabled
}


void tcPrt(const char *str)
{
	(void) prtStop();  // The return code does not report closing information
	(void) prtStart();

	if (str == (char *) 0)
	{
		str = dspBuf;
	}

	while (*str)  // Print str, on multiple lines when
	{			  // str is longer than the printer line
		unsigned char prtBuf[__PRTW__+ 1];
		(void) strncpy(prtBuf, str, __PRTW__);
		prtBuf[__PRTW__] = 0;
		prtS(prtBuf);
		str += strlen(prtBuf);
	}
	
	(void) prtStop();
}


char tcWait(const char *msg, int param)
{
	int ret, len, rc, timeout, line;
	char tmpMsg[3 *__DSPW__];
	char waitMsg[3 *__DSPW__];
	char wasDelayed;

	
	if (param == 1)
		tcBeep(0);
	else if (param < 0)
		tcBeep(param); // forces the click
	
	if (param > 1)
		timeout = param * 50; // param is in 1/2 second
	else
		timeout = 0;

	if (!msg || msg[0] == 0)
	{
		msg = "Hit a key";
	}

	len = sprintf(tmpMsg, "%s", msg) - 1;
	
	for (rc = 0 ; len > 0 ; len--)
	{
		if (tmpMsg[len] == 13 || tmpMsg[len] == 10)
			rc++;
		else
			break;
	}
	
	if (rc)
		tmpMsg[len + 1] = 0;
	
	if (delayedLen)
	{
		len = sprintf(waitMsg, "%s", tmpMsg);
		wasDelayed = 1;
	}
	else
	{
		len = sprintf(waitMsg, ">> ? (%s) <<", tmpMsg);
		wasDelayed = 0;
	}
	
	VERIFY (len <= sizeof(waitMsg));
	line = ln;
	
	tcDspS(waitMsg);

	if (wasDelayed)
		tcDspS(">> ? Hit a Key <<");

	while (rc--)
	{
		tcDspS("");
		lnFeed();
	}

	ret = waitKey(timeout, DO_CHECK);
	ln = line;
	
	return ret;
}


void tcWaitS(char *prompt, char *dst, int sz)
{
    int idx;                    // index in dst
    int idc = 0;                // index in the circuit
	int refresh;
    char key;                   // the current key
    char last = 0;              // the key pressed before the current one
    int done = 0;               // input done

    const char *letterGrb[] =
	{
        "0 ",
        "1.,!",
        "2ABC",
        "3DEF",
        "4GHI",
        "5JKL",
        "6MNO",
        "7PQRS",
        "8TUV",
        "9WXYZ",
        "",                     //OK
        "",                     //Esc
        "<#-",                  //00
        "",                     //Backspace
        "",                     //
        "?*;",                  //Paper Feed
        "",
        "",
        "",
        "?*;$@|",               //key * - it need for terminal I6280
        "",
        ".,!<#-",               //key # - it need for terminal I6280
    };

    const char *letterLrb[] =
	{
        "0 ",
        "1.,!",
        "2abc",
        "3def",
        "4ghi",
        "5jkl",
        "6mno",
        "7pqrs",
        "8tuv",
        "9wxyz",
        "",                     //OK
        "",                     //Esc
        "<#-",                  //00
        "",                     //Backspace
        "",                     //
        "?*;",                  //Paper Feed
        "",
        "",
        "",
        "?*;$@|",               //key * - it need for terminal I6280
        "",
        ".,!<#-",               //key # - it need for terminal I6280
    };

	VERIFY(sz);

	sprintf(tmpBuf, " %s :", prompt);
	tcDspS(tmpBuf);
	pushLn();
	sprintf(tmpBuf, "%s_", dst);
	tcDspS(tmpBuf);

	idx = strlen(dst);

    do
	{
		key = waitKey(0, NO_CHECK);
		refresh = 1;
		
        if (idx < sz && key >= '0' && key <= '9')  // numbers
		{
            dst[idx++] = key;
			dst[idx] = 0;
            last = key;
            idc = 1;
        }
		else  // Function keys
		{
			const char *idcPtr;

            switch (key)
			{
              case __VALID_KEY__:
				  dspBuf[0] = 0;  // reset memorized message
				  done = 1;
                  break;
				  
              case __CANCEL_KEY__:
				  dst[0] = 0;
                  done = 1;
                  break;
				  
              case __DEL_KEY__:
                  if (idx > 0)
				  {
                      dst[--idx] = 0;
                  }
				  
                  break;
				  
              case __F1_KEY__:  // F1 Down, now lower case
			  case __F2_KEY__:  // F2 Up, now capital letters
				  if (key == __F1_KEY__)
				  {
					  idcPtr = &letterLrb[last - 48][idc];
				  }
				  else
				  {
					  idcPtr = &letterGrb[last - 48][idc];
				  }
				  
                  if ((last >= '0' && last <= '9') || last == '<' || last == '?')
				  {
					  dst[idx - 1] = *idcPtr++;
					  
					  if (*idcPtr)
					  {
						  idc++;
					  }
					  else
					  {
						  idc = 0;
					  }
                  }
				  
                  break;

			  default:
				  refresh = 0;
				  break;
			}
		}

		if (refresh)
		{
			popLn();  // To overwrite previous display

			if (done)
			{
				strcpy(tmpBuf, dst);
			}
			else
			{
				sprintf(tmpBuf, "%s_", dst);				
			}
			
			tcDspS(tmpBuf);
		}
		
    } while (!done);
}
	

int tcWaitNum(const char *prompt, int *num, int minVal, int maxVal, int to)
{
	static storedNum;
	char key;
	int refresh, val, done, valSet;
	int correct = 0;

	do  // Display initial prompt
	{
		valSet = val = done = 0;
		sprintf(tmpBuf, " %s (%d-%d) [%d] ? _", prompt, minVal, maxVal, *num);
		dspLines(tmpBuf);
		popLn();
		//dspLS(ln, tmpBuf);

		do  // Process kbd entries
		{
			key = waitKey(100 * to, NO_CHECK);
			refresh = 1;

			switch (key)
			{
				case'0':case'1':case'2':case'3':case'4':
				case'5':case'6':case'7':case'8':
				case'9':
					val = (val * 10) + (key - '0');
					valSet = 1;
					break;

				case __DEL_KEY__:
					val /= 10;
					break;

				case __VALID_KEY__:
					done = correct = 1;

					if (valSet)
					{
						if (val >= minVal && val <= maxVal)
							*num = storedNum = val;
						else
							correct = refresh = 0;
					}

					break;

				case __CANCEL_KEY__:
					done = correct = 1;
					break;

				default:
					//tcDspFN("DEBUG: KEY = %d", key);
					if (key == 0)  // timed out
						done = correct = 1;
					else
						refresh = 0;
					break;
			}

			if (correct)
			{
				sprintf(tmpBuf, " %s [%d]", prompt, *num);
				dspLines(tmpBuf);
				popLn();
			}
			else if (refresh)
			{
				sprintf(tmpBuf, " %s (%d-%d) ? %d_", prompt, minVal, maxVal, val);
				dspLines(tmpBuf);
				popLn();
			}
		} while (!done);
	} while (!correct);

	tcLnFeed(0);  // Prepare next display

	return key;
}


#ifdef __TC_APP__
// (When TC_APP is not defined, tclib is not used to generate a test application
//  but may be used as a debugging and utilities tool box)

static char tcConfirm(char *msg)
{
	tcClear();
	tcDspS("Confirm selection");
	tcDspS(msg);
	return waitKey(0, NO_CHECK);
}

/*****
static int tcStart(void)
{
	int ret = dspStart();
	CHECK(ret >= 0, lblKO);

	ret = prtStop();   //In case prt was already opened
	CHECK(ret >= 0, lblPrtOpened);

	ret = prtStart();
	CHECK(ret >= 0, lblPrtKO);
	goto lblEnd;

lblPrtKO :
	tcWait("Printer error", 1);
	goto lblEnd;

lblPrtOpened:
	ret = 0;
	goto lblEnd;

lblKO :
	Beep(); Beep();
lblEnd :
	return ret;
}
**********/


static void cnfDef(void)
{
	maxSeq = maxIter = 1;
	wait = EOT_WAIT;
	pause = 2;

	dspMode = 0;

#ifdef __MSG_2_DSP__
	tcDspModeSet(DSP_ENABLE);
#endif

#ifdef __TRC_2_HMI__
	tcDspModeSet(TRC_2_HMI);
#endif

#ifdef __MSG_2_PRT__
	tcDspModeSet(MSG_2_PRT);
#endif

#ifdef __MSG_2_SER__
	tcDspModeSet(MSG_2_SER);
#endif

	cnfByUser = 0;
}


static void cnfSet(tcCfg_t *tcP)
{
	maxIter = tcP->loop;
	wait    = tcP->wait;
	pause   = tcP->pause;
}


static tcCfg_t *cnfAuto(mnuCfg_t *item)
{
	// array of 2 void tc articles
	// the article 0 is updated to be run by runAuto
	// the article 1 is used as terminator
	static tcCfg_t tcAuto[2];

	tcAuto[0].loop = maxIter;
	tcAuto[0].wait = wait;
	tcAuto[0].pause = pause;

	if (item->label[0] == 'T')  // "TC..."
	{
		int idx;

		for (idx = 0 ; idx < PARAM_NUM ; idx++)
			tcAuto[0].param[idx] = item->param[idx];

		tcAuto[0].tc = item->mnTc;

		//JP! remove ?
		if (maxIter > 1)
		{
			tcAuto[0].wait = NO_WAIT;
		}		
	}
	else
	{
		title[0] = 0;
		tcAuto[0].tc = 0;
	}	

	return tcAuto;
}


static void runAuto(tcCfg_t *tcTab, int seq)
{
	char defTitle[__DSPW__];
	//char defpS1[__DSPW__];
	int doClear;

	// Beep is off by default
	tcDspModeSet(-BEEP_ON);
	
	// Step by step not active by default
	tcDspModeSet(-STP_BY_STP);

	// At least 1 iteration
	if (seq == 0)
	{
		maxSeq = 1;
		tcDspModeSet(SCR_BY_SCR);
	}
	else // (when there is more than 1 iteration, screen by screen mode is not active,
		 //  to avoid being stopped at every end of screen)
		maxSeq = seq;

	for (curSeq = 1 ; curSeq <= maxSeq ; curSeq++)
	{
		// Run the test sequence
		//
		for (tcPtr = tcTab ; tcPtr->tc ; tcPtr++)
		{
			if (!cnfByUser)
			{
				cnfSet(tcPtr);
			}

			strcpy(defTitle, title);  // Save default title

			// tc title may be set also using first parameter.
			if (tcPtr->param[0] && *tcPtr->param[0])
				strcpy(title, tcPtr->param[0]);
		
			doClear = 1;
			
			for (curIter = 1 ; curIter <= maxIter ; curIter++)
			{
				int ret = -1;

				if (doClear)
					tcDspInit(0);
				
				tcReset();
				// Show title for one second
				// (tc0() begins by clearing the screen).
				tcDspTitle(0, 0);
				popLn();  // overwrite title at tc0 end
					
				if (!cancelled)
				{
					// Create the execution pointer 
					tc0p_t tc0 = (tc0p_t) tcPtr->tc;

					dspStop();  // (old tc0 usually starts dsp => stop it)
					tc0();      // run tc0
				}
					
				if (!cancelled)
				{
					ret = tcGetRet();
					if (ret != 0)  // Pause on tc error
					{
						int loop = tcLoop();
						
						if (loop == 1)
							sprintf(tmpBuf, " %s: error %d", title, ret);
						else
							sprintf(tmpBuf, " %s: error %d, loop %d", title, ret, loop);
							
						tcDspS(tmpBuf);
						(void) tcWait("Cancel stops", -2);  // (forces 2 beeps)
					}
					else if (wait == EOT_WAIT
							 && curIter == maxIter
							 && curSeq == maxSeq)  // Wait at test end
					{
						sprintf(tmpBuf, "%s done !", title);
						(void) tcWait(tmpBuf, -2);
						doClear = 1;
					}
					else if (wait == ITER_WAIT)  // Wait at every iteration
					{
						(void) tcWait("Cancel stops", 0);
						doClear = 0;
					}	
					else  // No wait, just make a pause
					{
						doClear = 1;
						sprintf(tmpBuf, ">> %s done %dx, Cancel stops", title, tcLoop());
						tcDspS(tmpBuf);
						popLn();  // To get this last message overwritten

						if (!cancelled)
						{
							// pause : ... + 10 : 100 ms is the minimum pause
							//  (for detecting the cancel key pressing to break the loop)
							(void) waitKey(100 * pause + 10, DO_CHECK);
						}
					}
				}

				if (tcCancel())  // if CANCEL was pressed,
				{
					curSeq = maxSeq + 1; // main loop exit condition
					break;				 // break the current loop
				}
			}

			strcpy(title, defTitle);  // Restore default title
		}
	}
}


static int runMenu(mnuCfg_t *mnu)
{
	int ret, again;
	mnuCfg_t *item;
	card oldDspMode;

	again = 1;
	do  // ... while again
	{
		oldDspMode = tcDspModeReset(DSP_ENABLE | KBD_CHECK | IN_MENU);

		do  // Loop for configuration menu management
		{
			tcDspInit(0);
		
			for (item = mnu ; *item->label ; item++)
			{
				if (item->choice)
				{
					sprintf(tmpBuf, " %c: %s", item->choice, item->label);
				}
				else
				{   // menu title
					sprintf(tmpBuf, " %s", item->label);
				}

				tcDspS(tmpBuf);
			}

			ret = tcWait("Select/Cancel", 0);
			
		} while (ret == __00_KEY__);

		tcDspModeSet(oldDspMode);  // restore original dspMode

		for (item = mnu ; *item->label ; item++)
		{
			if (item->choice == ret)
				break;
		}

		if (*item->label)  // Valid menu item is chosen
		{	
			// Set default tc title after the menu item label
			strcpy(title, item->label);

			if (item->mnTc)
			{
				switch (item->label[0])
				{
					case 'M': // "MNU..."
						runMenu((mnuCfg_t *) item->mnTc);
						break;
						
					case 'T': // "TC..."
						runAuto(cnfAuto(item), item->loop);
						break;
						
					case 'L': // "LST..."
						runAuto((tcCfg_t *) item->mnTc, item->loop);		
						break;
						
					default:
						break;
				}
			}
		}
		else  // No valid menu choice -> leave this menu
		{
			title[0] = 0;
			again = 0;
		}

	} while (again);

	return tcCancel();
}


void selectMain(void)
{
	// Set default config
	cnfDef();
	runMenu(tcMenu());
	dspStop();
}

#endif  // __TC_APP__

#else  // __TEST__ !defined

const char *tcIdleMsg(void) { return 0; }
const char *tcIdleApp(void) { return 0; }
int tcLoop(void) { return 0; }
char tcConfirm(char *msg) { return 0; }
//int tcStart(unsigned int lnVal) { return 0; }
void tcStop(void) {}
void tcClear(void) {}
char *tcDspBuf(void) { return 0; }
int tcDspS(const char *str) { return 0; }
int tcDspPS(const char *str) { return 0; }
int tcDspFN(const char *fmt, int val) { return 0; }
int tcDspF2N(const char *fmt, int val1, int val2) { return 0; }
int tcDspFS(const char *fmt, const char *str) { return 0; }
int tcDspAN(const unsigned char *buf, int len) { return 0; }
int tcDspTitle(const char *title, const char *subTitle) { return 0; }
void tcPrt(const char *str) {}
char tcWait(const char *msg, int ret) { return 0; }
void selectMain(void) {}

#endif
