/// \file tcik0100.c
#include <stdio.h>
#include <string.h>             // for memset, strcpy, etc

#include <unicapt.h>
//#include "tst.h"

// for LNET
#include <LNet.h>
#include <LNetDns.h>
#include <LNetFtp.h>
#include <LNetSmtp.h>
#include <LNetTcpip.h>
#include <LNetSocket.h>

// for GSM
#include <gsm.h>

#define dftPin "0000"
#define dftAPN "orange.fr"
#define dftLogin "danfr"
#define dftPwd "mq36noqt"

/*
//serbie
#define dftPin "9868"
#define dftAPN "futuraplus"
#define dftLogin "mts"
#define dftPwd "064"
*/

/** NON UTA Unitary test case for GPRS connection.
 * \sa
 *  - gsmOnOff()
 */

/*
#define dftPin "5670"
#define dftAPN "futuraplus"
#define dftLogin "mts"
#define dftPwd "064"
*/
//free.fr
//static const char *dftIP= "212.27.42.12";
//static int dftPort= 110;
//first message to be sent to free.fr
//static const char *dftSendMsg= "stat\x0D\x0A";

//serveur topup
static char *dftIP = "91.121.18.221";
static int dftPort = 6789;

//static char *dftIP= "172.19.49.10";
//static int dftPort= 6789;
//first message to be sent to topup
static const char *dftSendMsg =
    "\x00\x00\x00\x0C\x01\x01\x04\x00\x85\x05\x03\33\x00\x01\x0D\x0A";
//static const char dftSendMsg[103]= "\x00\x00\x00\x5E\x01\x01\x04\x00\x85\x0B\x03\x35\x56\x00\x05\x03\x72\x65\x63\x68\x61\x72\x67\x65\x00\x1D\x03\x32\x30\x30\x37\x30\x36\x30\x34\x31\x31\x31\x30\x31\x30\x00\x06\x03\x39\x39\x39\x39\x39\x39\x39\x39\x00\x0F\x03\x31\x30\x00\x10\x03\x30\x30\x31\x00\x11\x03\x30\x30\x31\x00\x12\x03\x33\x38\x31\x36\x34\x35\x30\x34\x30\x35\x35\x38\x00\x13\x03\x31\x39\x2E\x30\x30\x00\x14\x03\x33\x00\x01\xB3\x58\x96\x8C";

static const char *phone = "*99***1#";

static uint32 hHmi = 0;
static uint32 hPrt = 0;
void show(const char *str, int idx) {
    static uint16 row = 0;

    if(idx == 0)
        row = 0;
    hmiADClearLine(hHmi, row);
    hmiADDisplayText(hHmi, row, 0, (char *) str);
    row += 1;
    if(row >= 15)
        row = 0;
}

void print(const char *str) {
    prnPrint(hPrt, (char *) str);
}

#define CHECK(CND,LBL,ERR) {if(!(CND)){show("*** ERROR ***",1); show(ERR,1); print("*** ERROR ***"); print(ERR); goto LBL;}}

#define CID 1
static gsmGprsContext_t GprsPdp = { //GPRS PDP context for orange.fr SIM
    CID,                        //uint8 cid;            //PDP context identifier. a numeric parameter (1-32)
    //which specifies a particular PDP context definition
    "IP",                       //uint8 PDPType[16+1];    //(Packet Data Protocol type) a string parameter
    //which specifies the type of PDP. "IP" Internet
    //Protocol or "PPP" Point to Point Protocol
    dftAPN,                     //uint8 APN[64+1]; //(Access Point Name) which is a logical name that
    //is used to select GGSN or external packet data
    //network. If the value is NULL or omitted, then
    //the subscription value will be requested
    "",                         //uint8 PDPAddress[16+1];   //A string parameter that identifies the Mobile in
    //the address space applicable to the PDP (optional)
    0,                          //uint8 dComp;               //To control PDP data compression 0:off (default), 1 on. (optional)
    0                           //uint8 hComp;               //To control PDP header compression 0:off (default),1 on. (optional)
};

//uint32 h;

static gsmGprsProfile_t GprsProfile = {
    CID,                        //uint8 cid;               //PDP context identifier
    0,                          //uint8 precedence;          //Precedence class, 0: Subscribed by the Nwk / default if value is omitted
    0,                          //uint8 delay;               //Delay class, 0: Subscribed by the Nwk / default if value is omitted
    3,                          //uint8 reliability;         //Reliability class, Non real-time traffic, error sensitive application that can cope with data loss, GMM/SM, and SMS.
    0,                          //uint8  peak                //Subscribed by the Nwk / default if value is omitted
    0                           //uint8  mean                //Subscribed by the Nwk / default if value is omitted
};

typedef struct {
    int16 status;
    uint16 length;
    uint8 datas[1];
} oneResult_t;

static uint32 niHandle;
static uint32 gsmHandle;
static int socketHandle;

static int startGPRSConnection(void) {
    int16 ret, err;
    uint8 onOff, rssi, ber, pinstat, netStat, nbpin1, nbpin2, nbpuk1, nbpuk2,
        attachStat, status;
    char buf[256];
    int n;

    hmiADClearLine(hHmi, HMI_ALL_LINES);

    show("gsmOpen", 0);
    ret = gsmOpen(&gsmHandle, "MODEM3");
    CHECK(ret == RET_OK, lblKO, "gsmOpen");

    show("gsmOnOff", 1);
    onOff = 1;
    ret = gsmOnOff(gsmHandle, GET, &onOff); //Check if the GSM Device is turn on.
    if((onOff != 1) || (ret == RET_OK))
        ret = gsmOnOff(gsmHandle, SET, &onOff); //Turn on the GSM Device.
    CHECK(ret == RET_OK, lblClose, "gsmOnOff");
/* //optional
	show("gsmGetImei",1); //terminal ID
	ret = gsmGetImei(gsmHandle, buf);
	show(buf,1);
	CHECK(ret==RET_OK,lblClose,"gsmGetImei");
*/
    show("gsmGetPinStatus", 1);
    gsmGetPinStatus(gsmHandle, &pinstat, &nbpin1, &nbpin2, &nbpuk1, &nbpuk2);
    if(pinstat == 1) {
        ret = gsmEnterPin(gsmHandle, "", dftPin);
        CHECK(ret == RET_OK, lblReadErrorAndClose, "gsmEnterPin");
    }
/*
	show("gsmGetImsi",1); //sim ID
	ret = gsmGetImsi(gsmHandle, buf);
	show(buf,1);
	CHECK(ret==RET_OK,lblReadErrorAndClose,"gsmGetImsi");
*/
    ret = gsmGetNetworkStatus(gsmHandle, &status);
    //Force operator selection only if GSM module is not registered on network
    if((status != 1) || (status != 5) || (ret != RET_OK)) { //1 and 5 registered status
        ret = gsmOperatorSelection(gsmHandle, 0, 2, "");
        if(ret != RET_OK) {
            psyTimerWakeAfter(SYS_TIME_SECOND * 3); //3 seconds, calibration test required
            ret = gsmOperatorSelection(gsmHandle, 0, 2, "");
            CHECK(ret == RET_OK, lblReadErrorAndClose, "gsmReadNetworkStatus");
        }
    }

    psyTimerWakeAfter(SYS_TIME_SECOND);
    show("SignalParameters", 1);
    ret = gsmGetSignalParameters(gsmHandle, &rssi, &ber);
    sprintf(buf, "rssi=%d", rssi);
    show(buf, 1);
    sprintf(buf, "ber=%d", ber);
    show(buf, 1);
    CHECK(ret == RET_OK, lblKO, "SignalParameters");

    hmiADClearLine(hHmi, HMI_ALL_LINES);    //Clear screen
    show("GetNetworkStatus", 0);
    netStat = 0;
    ret = gsmGprsGetNetworkStatus(gsmHandle, &netStat);
    sprintf(buf, "netStat=%d", netStat);
    show(buf, 1);
    CHECK(ret == RET_OK, lblReadError, "GetNetworkStatus");

    if(netStat == 1) {
        show("already connected!", 1);
        print("already connected!");
    } else {
        print("attach to gprs..");
        attachStat = 1;
        show("gsmGprsAttachStatus SET", 1);
        ret = gsmGprsAttachStatus(gsmHandle, SET, &attachStat);
        //CHECK(ret==RET_OK,lblReadError,"gsmGprsAttachStatus  SET");
        if((attachStat == 0) || (ret != RET_OK)) {  //try to attach
            for (n = 0; n < 180; ++n) { //wait 3 minutes
                psyTimerWakeAfter(SYS_TIME_SECOND);
                show("gsmGprsAttachStatus GET", 1);
                ret = gsmGprsAttachStatus(gsmHandle, GET, &attachStat);
                sprintf(buf, "attachStat=%d", attachStat);
                show(buf, 1);
                if(ret == GSM_CME_ERROR) {
                    gsmReadError(gsmHandle, &err);
                    if(err == 515)
                        continue;
                    else
                        goto lblReadErrorAndClose;
                }
                CHECK(ret == RET_OK, lblReadError, "gsmGprsAttachStatus GET");
                if(attachStat == 1)
                    break;
            }
        }
        sprintf(buf, "attachStat=%d", attachStat);
        show(buf, 1);

    }
    show("gsmGprsSetPDPContext", 1);
    ret = gsmGprsSetPDPContext(gsmHandle, &GprsPdp);
    CHECK(ret == RET_OK, lblReadError, "gsmGprsSetPDPContext");

    show("gsmGprsSetQoSProfile", 1);
    ret = gsmGprsSetQoSProfile(gsmHandle, &GprsProfile);
    CHECK(ret == RET_OK, lblReadError, "gsmGprsSetQoSProfile");

    show("gsmGprsSetPDPActivate", 1);
    gsmGprsSetPDPActivate(gsmHandle, 1, CID);
    CHECK(ret == RET_OK, lblReadError, "gsmGprsSetPDPActivate");

    show("GPRS attached", 1);
    print("GPRS attached");
    ret = 0;
    goto lblEnd;
  lblReadErrorAndClose:
    if(ret == GSM_CME_ERROR) {
        gsmReadError(gsmHandle, &err);
        sprintf(buf, "gsmReadError=%d", err);
        print(buf);
    }
    goto lblKO;
  lblReadError:
    if(ret == GSM_CME_ERROR) {
        gsmReadError(gsmHandle, &err);
        sprintf(buf, "gsmReadError=%d", err);
        print(buf);
    }
  lblClose:
    goto lblKO;
  lblKO:
    ret = -1;
    show("gsmClose", 1);
    gsmClose(gsmHandle);
    print("KO!");
  lblEnd:
    return ret;
}

static int startNI(void) {
    int16 ret, err;
    netChannelList_t pBuffer[NET_CHANNEL_MAX];
    netNiConfig_t config;
    struct in_addr s_ipAddr;
    char buf[256];
    int i;

    hmiADClearLine(hHmi, HMI_ALL_LINES);

    show("netNiOpen", 0);
    ret = netNiOpen(NET_NI_PPP, &niHandle);
    CHECK(ret == RET_OK, lblReadError, "netNiOpen");

    show("netCfgIdentify", 1);
    ret =
        netCfgIdentify(NET_CHANNEL_LIST, NET_CHANNEL_LIST_SIZE,
                       (void *) pBuffer);
    CHECK(ret == RET_OK, lblKO, "netCfgIdentify");
    for (i = 0; pBuffer[i].name[0] != '\0'; ++i)
        if(!strcmp(pBuffer[i].name, "MODEM3"))
            break;
    CHECK((pBuffer[i].name[0] != '\0'), lblKO, "MODEM3");

    memset(&config, 0, sizeof(config));
    config.ppp.fields =
        NI_PPP_F_CHANNEL | NI_PPP_F_PPPCFG | NI_PPP_F_PHONENUMBER |
        NI_PPP_F_LOCALIPADDRESS;
    config.ppp.channel = i;
    config.ppp.pppCfg.fields = NI_PPP_F_LCPFLAGS | NI_PPP_F_IPCPFLAGS   /*| NI_PPP_F_VJMAX */
        | NI_PPP_F_LOCALUSERNAME | NI_PPP_F_LOCALPASSWORD | NI_PPP_F_INFLAGS;
    config.ppp.pppCfg.lcpFlags =
        NI_PPP_LCP_PAP | NI_PPP_LCP_PCOMP | NI_PPP_LCP_ACOMP;
    config.ppp.pppCfg.ipcpFlags =
        /*NI_PPP_IPCP_ADDR */
        NI_PPP_IPCP_ACCEPT_LOCAL | NI_PPP_IPCP_ACCEPT_REMOTE | NI_PPP_IPCP_DNS1
        | NI_PPP_IPCP_DNS2;
    config.ppp.pppCfg.inFlags =
        NI_PPP_IN_DEFAULT_ROUTE /* NI_PPP_IN_RESTART */ ;

    //config.eth.inFlags = NI_ETH_IN_IPADDR_VIA_DHCP;
    //config.eth.dhcpReleaseTimeout = 0;
    //config.eth.outFlags = NI_ETH_IN_DHCP_NOT_SAME_IADDR;
    //config.loopback.fields = NI_LOOP_F_CHANNEL;
    //config.ppp.pppCfg.vjMax = 48;
    config.ppp.pppCfg.localUsername = dftLogin;
    config.ppp.pppCfg.localPassword = dftPwd;
    config.ppp.phoneNumber = (char *) phone;

    show("netNiConfigSet", 1);
    ret = netNiConfigSet(niHandle, NET_NI_LEVEL_1, &config);
    CHECK(ret == RET_OK, lblKO, "netNiConfigSet");

    show("netNiStart", 1);
    ret = netNiStart(niHandle, 120 * SYS_TIME_SECOND);
    CHECK(ret == RET_OK, lblKO, "netNiStart");

    // --------------------------------------------------------------------------------
    // Code to GET the local IP address --------------------------------------
    memset(&config, 0, sizeof(config));

    config.ppp.fields = NI_PPP_F_LOCALIPADDRESS | NI_PPP_F_PPPCFG;
    config.ppp.pppCfg.fields = NI_PPP_F_DNS1IPADDRESS | NI_PPP_F_DNS2IPADDRESS;
    ret = netNiConfigGet(niHandle, NET_NI_LEVEL_1, &config);
    CHECK(ret == RET_OK, lblKO, "netNiConfigGet");

    {
        s_ipAddr.s_addr = config.ppp.localIpAddress;
        inet_ntoa(s_ipAddr, buf);
        print("local IP addr:");
        print(buf);
        sprintf(buf, "DNS1=%X", config.ppp.pppCfg.dns1IpAddress);
        print(buf);
        sprintf(buf, "DNS2=%X", config.ppp.pppCfg.dns2IpAddress);
        print(buf);
    }

    ret = 0;
    goto lblEnd;
  lblReadError:
    if(ret == GSM_CME_ERROR) {
        gsmReadError(gsmHandle, &err);
        sprintf(buf, "gsmReadError=%d", err);
        print(buf);
    }
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int testPing(void) {
    int ret;
    icmpPingResult_t result;
    char buf[256];

    hmiADClearLine(hHmi, HMI_ALL_LINES);

    show("ping 1...", 0);
    ret = icmpPingEx((char *) dftIP, 1000, 54, 32, &result);
    if(ret > 0)
        sprintf(buf, "1: %d", result.time);
    else
        strcpy(buf, "ping failed");
    show(buf, 1);

    show("ping 2...", 1);
    ret = icmpPingEx((char *) dftIP, 1000, 54, 32, &result);
    if(ret > 0)
        sprintf(buf, "2: %d", result.time);
    else
        strcpy(buf, "ping failed");
    show(buf, 1);

    show("ping 3...", 1);
    ret = icmpPingEx((char *) dftIP, 1000, 54, 32, &result);
    if(ret > 0)
        sprintf(buf, "3: %d", result.time);
    else
        strcpy(buf, "ping failed");
    show(buf, 1);

    return 1;
}

static void stopGPRSConnection(void) {
    uint8 sta;
    int16 ret;
    char buf[256];

    hmiADClearLine(hHmi, HMI_ALL_LINES);

    show("netNiStop", 0);
    netNiStop(niHandle);

    show("netNiClose", 1);
    netNiClose(niHandle);
    niHandle = 0;
    psyTimerWakeAfter(SYS_TIME_SECOND);
// Disconnect the PDP context to avoid GSM Data problem
// Not required if you want to do several GPRS without GSM data
    show("PDP DEActivate", 1);
    gsmGprsSetPDPActivate(gsmHandle, 0, 255);
/*
	show("gsmHangUp",2);
	ret = gsmHangUp(gsmHandle);
	if(ret != RET_OK)
		print("gsmHangUp fail");
*/
    show("GPRS detach", 0);
    sta = 0;
    ret = gsmGprsAttachStatus(gsmHandle, SET, &sta);
    if(ret != RET_OK)
        print("Detach GPRS fail");
    //psyTimerWakeAfter(SYS_TIME_SECOND);
    ret = gsmGprsGetNetworkStatus(gsmHandle, &sta);
    if(ret != RET_OK)
        print("GPRS NetStat fail");
    if((sta != 1) && (sta != 5))
        print("GPRS NetStat detached");
    sprintf(buf, "GetNetSta: %d", sta);
    print(buf);

    show("gsmClose", 0);
    gsmClose(gsmHandle);
    gsmHandle = 0;
    psyTimerWakeAfter(3 * SYS_TIME_SECOND);
}

static int startIPConnection(void) {
    struct sockaddr_in addr;

    hmiADClearLine(hHmi, HMI_ALL_LINES);

    show("socket", 0);
    socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketHandle == -1) {
        print("Error Socket");
        return -1;
    }

    addr.sin_addr.s_addr = inet_addr(dftIP);
    addr.sin_port = htons((uint16) dftPort);

    addr.sin_family = AF_INET;
    memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

    show("connect", 1);
    if(connect(socketHandle, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        print("Error Connect");
        return -1;
    }
    show("Connected", 1);

    return 1;
}

static int testSendReceive(void) {
    //char data[256];
    int16 errorCode;

    show("utaGprsSend..", 1);
    errorCode = send(socketHandle, dftSendMsg, strlen(dftSendMsg), 0);
    if(errorCode == -1) {
        print("write error -1");
        return -1;
    } else {
        show("write OK", 1);
    }
    psyTimerWakeAfter(2 * SYS_TIME_SECOND);
/*
	show("read",1);
	errorCode = read(socketHandle, data, 256);
	show("read Done",1);
	if (errorCode == -1){
		print("read Error -1");
		return -1;
	}
  else{
		show("read OK",1);
		//print(data);
  }
	print("write read OK");
	*/
    show("close", 1);
    close(socketHandle);

    show("testSendReceive OK", 1);
    return 1;
}

void tcik0100(void) {
    int ret;

    hmiOpen("DEFAULT", &hHmi);
    prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    print("tcik0100 start");

    ret = startGPRSConnection();
    CHECK(ret >= 0, lblKO, "KO");
    psyTimerWakeAfter(SYS_TIME_SECOND);
    ret = startNI();
    CHECK(ret >= 0, lblKO, "KO");
    psyTimerWakeAfter(SYS_TIME_SECOND);
    //ret= testPing(); CHECK(ret>=0,lblKO,"KO");
    //psyTimerWakeAfter(SYS_TIME_SECOND);
    ret = startIPConnection();
    CHECK(ret >= 0, lblKO, "KO");
    psyTimerWakeAfter(SYS_TIME_SECOND);
    ret = testSendReceive();
    CHECK(ret >= 0, lblKO, "KO");
    psyTimerWakeAfter(5 * SYS_TIME_SECOND);
    goto lblEnd;
  lblKO:
    goto lblEnd;
  lblEnd:
    //Beep();
    stopGPRSConnection();
    print("----------------");
    hmiClose(hHmi);
    prnClose(hPrt);
}
