
#include <stdio.h>
#include <unicapt.h>
#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "tst.h"

//for LGCL
#include "gcl.h"
#include "gcl_gprs.h"
#include "gcl_gsm.h"

// for LNET
#include <LNet.h>
#include <LNetDns.h>
#include <LNetFtp.h>
#include <LNetSmtp.h>
#include <LNetTcpip.h>
#include <LNetSocket.h>

//Unitary test case tcik0068.
//Functions testing of GPRS connection:
//      GPRS connection via GCL
//      FTP via socket
//      GPRS disconnection

typedef struct sGclCfg {
    byte chn;                   //communications channel used
    //gclTelephone_t tel;
    gclGPRS_t gprs;
    gclGSM_t gsm;
    char *pin;                  //pin of simcard
    uint32 rcvDly;              //receiving timeout
    const byte *rcvTrm;         //terminators string
} tGclCfg;
static tGclCfg cfg;

#define CHK CHECK(ret==RET_OK,lblKO)
static int16 nullCallBack(gclConfig_t * gcl) {
    return RET_OK;
}
static int16 connErrorCallBack(gclConfig_t * gcl) {
    gclSetError(0);
    return gclPostMessage(GCL_RETRY);
}
int16 GprsRecvFunc(gclConfig_t * gcl, uint8 * data, uint32 * actuallyRead,
                   uint32 maxLen) {
    gcl->communicationTimeout = cfg.rcvDly;
    return gclGPRSReceive(gcl, data, actuallyRead, maxLen);
}

#define INIT_LEN 1024
static int getTok(char *tok, const char *buf, char sep) {   //parse a token from the buffer separated by sep
    int idx = 0;

    while(*buf) {
        if(*buf == sep)
            break;
        *tok++ = *buf++;
        idx++;
        VERIFY(idx <= INIT_LEN);
    }
    *tok = 0;
    return idx;
}
static int getPinTok(const char *init) {
    int ret, len;
    static char pin[16 + 1];

    *pin = 0;
    ret = getTok(pin, init, '|');
    CHECK(ret > 0, lblKO);

    cfg.pin = pin;
    len = ret;

    return len;

  lblKO:
    return -1;
}
static int getAPNTok(const char *init) {
    int ret, len;
    static char Apn[16 + 1];

    *Apn = 0;
    ret = getTok(Apn, init, '|');
    CHECK(ret > 0, lblKO);
    len = ret;
    cfg.gprs.APN = Apn;

    return len;
  lblKO:
    return -1;
}
static int getLoginTok(const char *init) {
    int ret, len;
    static char login[16 + 1];
    static char password[16 + 1];

    *login = 0;
    *password = 0;

    ret = getTok(login, init, '|'); //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);
    len = ret + 1;
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;

    ret = getTok(password, init, '|');  //retrieve mandatory phone
    CHECK(ret > 0, lblKO);

    cfg.gprs.loginName = login; //"danfr";
    cfg.gprs.password = password;   //"mq36noqt";

    len += ret;
    return len;
  lblKO:
    return -1;

}
static int getTCPIPTok(const char *init) {
    int ret, len;
    static char IP[16 + 1];
    static char Port[16 + 1];

    *IP = 0;
    *Port = 0;

    ret = getTok(IP, init, '|');    //retrieve the phone parameters token from initialisation string
    CHECK(ret > 0, lblKO);
    len = ret + 1;

    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;

    ret = getTok(Port, init, '|');  //retrieve mandatory phone
    CHECK(ret > 0, lblKO);

    cfg.gprs.ipAddress = IP;
    cfg.gprs.tcpPort = Port;

    len += ret;

    return len;
  lblKO:
    return -1;
}

static int PinSend(void) {
    int ret;
    uint8 on, nbpin1, nbpin2, nbpuk1, nbpuk2, pinstat;
    uint32 gsmHandle;

    ret = gsmOpen(&gsmHandle, "MODEM3");
    CHECK(ret == GSM_OK, lblOpen);

    on = 1;
    ret = gsmOnOff(gsmHandle, SET, &on);
    CHECK(ret == GSM_OK, lblOnOff);

    gsmGetPinStatus(gsmHandle, &pinstat, &nbpin1, &nbpin2, &nbpuk1, &nbpuk2);
    if(pinstat == 1) {
        ret = gsmEnterPin(gsmHandle, "", cfg.pin);
        CHECK(ret == GSM_OK, lblPin);
    }
    gsmClose(gsmHandle);
    return 1;

  lblPin:
    trcFN("gsmEnterPin ret=%d\n", ret);
    gsmClose(gsmHandle);
    goto lblKO;
  lblOpen:
    trcFN("gsmOpen ret=%d\n", ret);
    goto lblKO;
  lblOnOff:
    trcFN("gsmOnOff ret=%d\n", ret);
    gsmClose(gsmHandle);
    goto lblKO;
  lblKO:
    return -1;
}

static int getGprsInit(const char *init) {  //retrieve cfg.tel from init
    int ret;

    VERIFY(init);
    VERIFY(strlen(init) <= INIT_LEN);

    cfg.gprs.communicationTimeout = 5000;
    cfg.gprs.connectionId = 1;
    cfg.gprs.connectTimeout = 4000;
    cfg.gprs.contextId = 1;
    cfg.gprs.lcpFlags = NI_PPP_LCP_PAP | NI_PPP_LCP_PCOMP | NI_PPP_LCP_ACOMP;
    cfg.gprs.loginTimeout = 3000;
    cfg.gprs.retries = 1;

    ret = getPinTok(init);

    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator

    ret = getAPNTok(init);
    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator

    ret = getLoginTok(init);
    CHECK(ret > 0, lblKO);
    init += ret;                //skip token
    VERIFY(*init == '|');
    init++;                     //skip separator

    ret = getTCPIPTok(init);
    CHECK(ret > 0, lblKO);

    ret = PinSend();
    CHECK(ret > 0, lblKO);

    return 1;

  lblKO:
    return -1;
}

static int StartGprs(const char *init) {
    int ret;
    gclFunctionList_t fnc;

    trcFS("StartGprs init=%s\n", init);
    ret = getGprsInit(init);    //parse initialisation string
    CHECK(ret > 0, lblKO);

    fnc.predial = gclGPRSPreDial;
    fnc.dial = gclGPRSDial;
    fnc.connect = gclGPRSConnect;
    fnc.login = nullCallBack;
    fnc.hangup = gclGPRSHangUp;
    fnc.retry = gclConnectRetry;
    fnc.error = connErrorCallBack;
    fnc.send = gclGPRSSend;
    fnc.recv = /*gclGPRSReceive */ GprsRecvFunc;

    ret = gclGPRSSet(&cfg.gprs, &fnc, 0, NULL);
    CHK;
    return 1;
  lblKO:
    trcFN("StartGprs err ret=%d\n", ret);
    return -1;

}

int StopGPRS(void) {
    int ret;

    trcS("StopGPRS\n");
    ret = gclStop();
    CHK;
    return 1;
  lblKO:
    trcFN("StopGPRS err ret=%d\n", ret);
    return -1;
}

void tcik0068(void) {
    int ret;
    char *ptr;
    ftpHandle_t hFtp;
    char buf[8 + 1 + 3 + 1];

    ptr = "0000|orange.fr|danfr|mq36noqt|082.247.161.069|6789";
    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    CHECK(ret >= 0, lblKO);

    ret = StartGprs(ptr);
    CHECK(ret >= 0, lblKO);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Gcl conn OK");
    CHECK(ret >= 0, lblKO);

    /* Create a new FTP client session */
    ret = dspLS(1, "ftpStart...");
    CHECK(ret >= 0, lblKO);

    hFtp = -1;
    ret = ftpOpen(&hFtp);
    CHECK(ret == RET_OK, lblKO);
    trcFN(" FTP handler = %d\n", hFtp);

    ret = dspLS(1, "ftpStart OK");
    CHECK(ret >= 0, lblKO);

    //ret = dspLS(2, "ftpType..");
    //CHECK(ret >= 0, lblKO);

    //ret= ftpType(hFtp,FTP_BINARY);
    //CHECK(ret == RET_OK, lblKO);

    ret = dspLS(2, "ftpConnect..");
    CHECK(ret >= 0, lblKO);

    ret = ftpConnect(hFtp, "81.80.165.104", "soner", "ve2mi1xo6", FTP_ACTIVE, 0);   //IK debug
    //ret = ftpConnect(hFtp,"81.80.165.104", "soner", "ve2mi1xo6", FTP_PASSIVE, 0);    //IK debug
    //ret = ftpConnect(hFtp,"ftp.fr.ingenico.com", "soner", "ve2mi1xo6", FTP_ACTIVE, 0);    //IK debug
    //ret = ftpConnect(hFtp,(char *)"82.247.161.69", (char *)"ingenico", (char *)"fedbo9d7", FTP_ACTIVE, 0);    //IK debug
    CHECK(ret == RET_OK, lblKO);

    ret = dspLS(2, "ftpConnect OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = ftpChDir(hFtp, (char *) "test");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "ftpCD test");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    nvmFileName(buf, 16);
    ret = ftpRetrieve(hFtp, buf, "5V09999999045.dnl");
    CHECK(ret > 0, lblKO);

    ret = dspLS(2, "ftpGet OK");
    CHECK(ret >= 0, lblKO);

    tmrPause(1);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    ftpDisconnect(hFtp);
    ftpClose(hFtp);
    StopGPRS();
    tmrPause(3);
    prtStop();
    dspStop();
}
