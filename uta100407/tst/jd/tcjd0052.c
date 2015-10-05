
#include <stdio.h>
#include <unicapt.h>
//#include <string.h>
#include "sys.h"
#include "tc.h"
// for GSM
#include <gsm.h>
// for LNET
#include <LNet.h>
#include <LNetDns.h>
#include <LNetFtp.h>
#include <LNetSmtp.h>
#include <LNetTcpip.h>
#include <LNetSocket.h>

//Unitary test case tcjd0052.
//Functions testing of GPRS connection:
//Function testing of GPRS connection, second part of test:
// 		GPRS is connected yet
// 		activate PDP Context 1
// 		send message via TCP/IP to server
//		GPRS disconnection
//First part of test tcjd0050

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)
//#define USE_MODEM
#define ACTIVCID	(2)

netIpAddr_t		localIpAddr;
netIpAddr_t		dns1IpAddr;
netIpAddr_t		dns2IpAddr;

static gsmGprsContext_t gsmGprsContextTIM = {
	ACTIVCID,
	"IP",
	"orange.fr",
	"",
	0,
	0
};

uint32 h;

static gsmGprsProfile_t gsmGprsProfileTIM = {
	ACTIVCID,
	0,
	0,
	3,
	0,
	0
};

static char *phone = "*99***1#";
static char *ko = " KO!";


typedef struct{
	int16	status;
	uint16	length;
	uint8	datas[1];
}oneResult_t;

oneResult_t		hmiResult;

static uint32 niHandle;
static uint32 gsmHandle;
static int socketHandle;

typedef struct sComChn {
    byte chn;                   //communications channel used
    byte chnRole;
    union {
#ifdef __CHN_TCP__
        struct {                //chn= chnTcp...
            netNi_t hdl;
            int16 sck;          //it is also ppp.sck
            byte idx;
            int16 srvSck;
            int16 cltSck;
        } tcp;
#endif
#ifdef __CHN_GPRS__
        struct {                //chn= chnGprs
            uint32 hdl;
            int16 sck;
        } gprs;
#endif
    } prm;
} tComChn;
static tComChn com;

static int startGPRSConnection(void){

	uint8 onOff, rssi, ber, pinstat;
	uint8 nbpin1, nbpin2, nbpuk1, nbpuk2;
	int16 ret;
	uint16 err;
	//char imei[16], imsi[16];
	int i;

	int n;
	uint8 netStat;
	uint8 attachStat;

	netChannelList_t pBuffer[NET_CHANNEL_MAX];
	netNiConfig_t config;

	dspClear();

	ret = gsmOpen(&gsmHandle, "MODEM3");
	CHECK(ret==RET_OK,lblKO);

	onOff = 1;
	ret = gsmOnOff(gsmHandle, SET, &onOff);
	CHECK(ret==RET_OK,lblClose);

	//ret = gsmGetImei(gsmHandle, imei);
	//HECK(ret==RET_OK,lblClose);

	gsmGetPinStatus(gsmHandle, &pinstat, &nbpin1, &nbpin2, &nbpuk1, &nbpuk2);

	if (pinstat == 1){
		ret = gsmEnterPin(gsmHandle, "", "0000");
		CHECK(ret==RET_OK,lblReadErrorAndClose);
	}

	psyTimerWakeAfter(SYS_TIME_SECOND);
	ret = gsmGetSignalParameters(gsmHandle, &rssi, &ber);
	CHECK(ret==RET_OK,lblKO);

	ret = gsmGprsGetNetworkStatus (gsmHandle, &netStat);
	CHECK(ret==RET_OK,lblReadError);

	if (netStat == 1){
		// already connected!
	}else{
		attachStat = 1;
		ret = gsmGprsAttachStatus(gsmHandle, SET, &attachStat);
		CHECK(ret==RET_OK,lblReadError);

		for (n = 0; n < 60; ++n)
		{
			psyTimerWakeAfter(SYS_TIME_SECOND);
			ret = gsmGprsAttachStatus(gsmHandle, GET, &attachStat);
			CHECK(ret==RET_OK,lblReadError);
			if (attachStat == 1) break;
		}

		ret = gsmGprsSetPDPContext(gsmHandle, &gsmGprsContextTIM); //Config of PDP Context
		CHECK(ret==RET_OK,lblReadError);
		ret = gsmGprsSetQoSProfile(gsmHandle, &gsmGprsProfileTIM);
		CHECK(ret==RET_OK,lblReadError);
	}

		tcDsp(" GPRS attached");
	if ((ret = netNiOpen(NET_NI_PPP, &niHandle)) != RET_OK)
	{
		tcDsp(" netNiOpen failed");
	}

	ret = netCfgIdentify(NET_CHANNEL_LIST, NET_CHANNEL_LIST_SIZE, (void*) pBuffer);
	CHECK(ret==RET_OK,lblKO);

	for (i = 0; pBuffer[i].name[0] != '\0'; ++i)
		if (!strcmp(pBuffer[i].name, "MODEM3")) break;
	CHECK((pBuffer[i].name[0] != '\0'),lblKO);

	memset(&config, 0, sizeof(config));

	config.ppp.fields = NI_PPP_F_CHANNEL | NI_PPP_F_PPPCFG | NI_PPP_F_PHONENUMBER|NI_PPP_F_LOCALIPADDRESS;
	config.ppp.channel = i;
	config.ppp.pppCfg.fields = NI_PPP_F_LCPFLAGS | NI_PPP_F_IPCPFLAGS | NI_PPP_F_VJMAX
		| NI_PPP_F_LOCALUSERNAME | NI_PPP_F_LOCALPASSWORD | NI_PPP_F_INFLAGS;
	config.ppp.pppCfg.lcpFlags = NI_PPP_LCP_PAP | NI_PPP_LCP_PCOMP | NI_PPP_LCP_ACOMP;
	config.ppp.pppCfg.ipcpFlags = /*NI_PPP_IPCP_ADDR*/NI_PPP_IPCP_ACCEPT_LOCAL|NI_PPP_IPCP_ACCEPT_REMOTE| NI_PPP_IPCP_DNS1 | NI_PPP_IPCP_DNS2;
	config.ppp.pppCfg.inFlags = NI_PPP_IN_DEFAULT_ROUTE;
	config.ppp.pppCfg.vjMax = 48;

	config.ppp.pppCfg.localUsername = "danfr";
	config.ppp.pppCfg.localPassword = "mq36noqt";
	config.ppp.phoneNumber = phone;

	ret = netNiConfigSet(niHandle, NET_NI_LEVEL_1, &config);
	CHECK(ret==RET_OK,lblKO);
	tcDsp(" Configuration set.");

	ret = netNiStart(niHandle, 120 * SYS_TIME_SECOND);
	CHECK(ret==RET_OK,lblKO);
	tcDsp(" Network started.");

	memset(&config, 0, sizeof(config));
	config.ppp.fields = NI_PPP_F_LOCALIPADDRESS | NI_PPP_F_PPPCFG;
	config.ppp.pppCfg.fields = NI_PPP_F_DNS1IPADDRESS | NI_PPP_F_DNS2IPADDRESS;

	ret = netNiConfigGet(niHandle, NET_NI_LEVEL_1, &config);
	CHECK(ret==RET_OK,lblKO);

	localIpAddr = config.ppp.localIpAddress;
	dns1IpAddr  = config.ppp.pppCfg.dns1IpAddress;
	dns2IpAddr  = config.ppp.pppCfg.dns2IpAddress;

	ret=0;
	goto lblEnd;
lblReadErrorAndClose:
	if (ret == GSM_CME_ERROR) gsmReadError(gsmHandle, &err);
	gsmClose(gsmHandle);
	goto lblKO;
lblClose:
	gsmClose(gsmHandle);
	goto lblKO;
lblReadError:
	if (ret == GSM_CME_ERROR) gsmReadError(gsmHandle, &err);
lblKO:
	ret=-1;
	tcDsp(ko);
lblEnd:
	return ret;
}

static void stopGPRSConnection(void){
	int ret;
	netNiStop(niHandle);
	netNiClose(niHandle);
// Disconnect the PDP context to avoid GSM Data problem
// Not required if you want to do several GPRS without GSM data
	ret=gsmGprsSetPDPActivate (gsmHandle , 0 , ACTIVCID );
	gsmClose(gsmHandle);
	return;
lblKO:
    trcErr(ret);
    tcDsp(ko);
    return;
}

static void deactivatePDPContext(void){
	int ret;
// Disconnect the PDP context to avoid GSM Data problem
// Not required if you want to do several GPRS without GSM data
	ret=gsmGprsSetPDPActivate (gsmHandle , 0 , ACTIVCID );
	//gsmClose(gsmHandle);
	return;
lblKO:
    trcErr(ret);
    tcDsp(ko);
    return;
}

static void activatePDPContext(void){
	int ret;
// Disconnect the PDP context to avoid GSM Data problem
// Not required if you want to do several GPRS without GSM data
	ret=gsmGprsSetPDPActivate (gsmHandle , 1 , ACTIVCID );
	return;
lblKO:
    trcErr(ret);
    tcDsp(ko);
    return;
}

static void startIPConnection(void){
    char data[256];
    	int16 errorCode;
		struct sockaddr_in addr;
		tBuffer buf;
    byte dat[1024];

		bufInit(&buf,dat,1024);
		bufReset(&buf);

    VERIFY(com.chn == chnTcp5100 || com.chn == chnTcp7780 || com.chn == chnPpp
           || com.chn == chnGprs);

     socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (socketHandle == -1)
			{
				tcDsp(" Error Socket");
				tmrPause(3);
				return;
			}
	/*
    addr.sin_addr.s_addr = inet_addr("212.27.42.12");	//free.fr
    addr.sin_port = htons(110);
    */
    addr.sin_addr.s_addr = inet_addr("82.247.161.69");	//serveur topup
    addr.sin_port = htons(6789);
    addr.sin_family = AF_INET;
    memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));


    if (connect(socketHandle, (struct sockaddr *) &addr, sizeof(addr)) != 0)
			{
				tcDsp(" Error Connect");
				tmrPause(3);
				return;
			}
		tcDsp(" Connected");

		bufAppStr(&buf,(byte *)"\x00\x0B\x85\x05\x03lalala\x00\x01");
		errorCode = send(socketHandle, bufPtr(&buf),bufLen(&buf),0);
		tcDsp(" utaGprsSend..");

	if (errorCode == -1)
	{
		tcDsp(" write error -1");
		//tmrPause(3);
		return;
	}
    else
    {
		tcDsp(" write OK");
		//tmrPause(3);
    }

	tcDsp(" read Start");
	errorCode = read(socketHandle, data, 256);
	tcDsp(" read Done");

	if (errorCode == -1)
	{
		tcDsp(" read Error -1");
		//tmrPause(3);
		return;
	}
    else
    {
       	tcDspVal("RX [%d]",errorCode);
    }


    close(socketHandle);
		goto lblEnd;
lblEnd:
	//tmrPause(3);
	return;
}

int tcjd0052(int pI1, int pI2, char *pS1, char *pS2) {
	int ret;
	//startGPRSConnection();
	//activatePDPContext();
	startIPConnection();
	goto lblEnd;
lblKO:
    trcErr(ret);
    tcDsp(ko);
lblEnd:
	//deactivatePDPContext();
	stopGPRSConnection();
}
