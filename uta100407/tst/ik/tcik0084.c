//Unitary nonUTA test case tcik0084.
//Functions testing of GPRS connection:
//  gsmGetImsi
//  gsmGetImei
//  gsmGetSignalParameters

#include <stdio.h>
#include <string.h>             // for memset, strcpy, etc

#include <unicapt.h>
/*
// for LNET
#include <LNet.h>
#include <LNetDns.h>
#include <LNetFtp.h>
#include <LNetSmtp.h>
#include <LNetTcpip.h>
#include <LNetSocket.h>
*/
// for GSM
#include <gsm.h>

#define dftPin "0000"
#define dftAPN "orange.fr"
#define dftLogin "danfr"
#define dftPwd "mq36noqt"

/*
#define dftPin "5670"
#define dftAPN "futuraplus"
#define dftLogin "mts"
#define dftPwd "064"
*/

static uint32 hHmi = 0;
static uint32 hPrt = 0;
static void showS(const char *str, int idx) {
    static uint16 row = 0;

    if(idx == 0)
        row = 0;
    hmiADClearLine(hHmi, row);
    hmiADDisplayText(hHmi, row, 0, (char *) str);
    row += 1;
    if(row >= 15)
        row = 0;
}

static void printS(const char *str) {
    prnPrint(hPrt, (char *) str);
}

#define CHECK(CND,LBL,ERR) {if(!(CND)){show("*** ERROR ***",1); showS(ERR,1); printS("*** ERROR ***"); print(ERR); goto LBL;}}

#define CID 2

typedef struct {
    int16 status;
    uint16 length;
    uint8 datas[1];
} oneResult_t;

static uint32 gsmHandle;

static int startGPRS(void) {
    int16 ret;
    uint8 onOff;

    hmiADClearLine(hHmi, HMI_ALL_LINES);

    showS("gsmOpen", 0);
    ret = gsmOpen(&gsmHandle, "MODEM3");
    CHECK(ret == RET_OK, lblKO, "gsmOpen");

    showS("gsmOnOff", 1);
    onOff = 0;
    ret = gsmOnOff(gsmHandle, GET, &onOff); //Check if the GSM Device is turn on.
    if((onOff != 1) || (ret == RET_OK)) {
        onOff = 1;
        ret = gsmOnOff(gsmHandle, SET, &onOff); //Turn on the GSM Device.
    }
    CHECK(ret == RET_OK, lblClose, "gsmOnOff");

    ret = 0;
    goto lblEnd;
/*
lblReadError:
	if (ret == GSM_CME_ERROR){
	  gsmReadError(gsmHandle, &err);
	  sprintf(buf,"gsmReadError=%d",err); printS(buf);
	}
	*/
  lblClose:
    goto lblKO;

  lblKO:
    ret = -1;
    showS("gsmClose", 1);
    gsmClose(gsmHandle);
    printS("startGPRS KO!");
  lblEnd:
    return ret;
}

static int getIMEI(void) {
    int16 ret, err;
    char buf[256];
    char IMEI[15 + 1];

    showS("gsmGetImei", 1);     //terminal ID
    ret = gsmGetImei(gsmHandle, IMEI);
    showS(IMEI, 1);
    printS("IMEI :");
    printS(IMEI);
    CHECK(ret == RET_OK, lblReadErrorAndClose, "gsmGetImei");
    ret = 0;
    goto lblEnd;
  lblReadErrorAndClose:
    if(ret == GSM_CME_ERROR) {
        gsmReadError(gsmHandle, &err);
        sprintf(buf, "gsmReadError=%d", err);
        printS(buf);
    }
    goto lblKO;
  lblKO:
    ret = -1;
    showS("gsmClose", 1);
    gsmClose(gsmHandle);
    printS("KO!");
  lblEnd:
    return ret;
}

static int setPIN(void) {
    int16 ret, err;
    uint8 pinstat, nbpin1, nbpin2, nbpuk1, nbpuk2;
    char buf[256];

    show("gsmGetPinStatus", 1);
    gsmGetPinStatus(gsmHandle, &pinstat, &nbpin1, &nbpin2, &nbpuk1, &nbpuk2);
    if(pinstat == 1) {
        ret = gsmEnterPin(gsmHandle, "", dftPin);
        CHECK(ret == RET_OK, lblReadErrorAndClose, "gsmEnterPin");
    }
    ret = 0;
    goto lblEnd;
  lblReadErrorAndClose:
    if(ret == GSM_CME_ERROR) {
        gsmReadError(gsmHandle, &err);
        sprintf(buf, "gsmReadError=%d", err);
        printS(buf);
    }
    goto lblKO;
  lblKO:
    ret = -1;
    showS("gsmClose", 1);
    gsmClose(gsmHandle);
    printS("KO!");
  lblEnd:
    return ret;
}
static int getIMSI(void) {
    int16 ret, err;
    char buf[256];

    showS("gsmGetImsi", 1);     //sim ID
    ret = gsmGetImsi(gsmHandle, buf);
    showS(buf, 1);
    printS("IMSI :");
    printS(buf);
    CHECK(ret == RET_OK, lblReadErrorAndClose, "gsmGetImsi");
    ret = 0;
    goto lblEnd;
  lblReadErrorAndClose:
    if(ret == GSM_CME_ERROR) {
        gsmReadError(gsmHandle, &err);
        sprintf(buf, "gsmReadError=%d", err);
        printS(buf);
    }
    goto lblKO;
  lblKO:
    ret = -1;
    showS("gsmClose", 1);
    gsmClose(gsmHandle);
    printS("KO!");
  lblEnd:
    return ret;
}

static int getNetStat(void) {
    int16 ret, err;
    uint8 status;
    char buf[256];

    ret = gsmReadNetworkStatus(gsmHandle, &status);
    //Force operator selection only if GSM module is not registered on network
    if((status != 1) || (status != 5) || (ret != RET_OK)) { //1 and 5 registered status
        ret = gsmOperatorSelection(gsmHandle, 0, 2, "");
        if(ret != RET_OK) {
            psyTimerWakeAfter(SYS_TIME_SECOND * 3); //3 seconds, calibration test required
            ret = gsmOperatorSelection(gsmHandle, 0, 2, "");
            CHECK(ret == RET_OK, lblReadErrorAndClose, "gsmReadNetworkStatus");
        }
    }
    ret = 0;
    goto lblEnd;
  lblReadErrorAndClose:
    if(ret == GSM_CME_ERROR) {
        gsmReadError(gsmHandle, &err);
        sprintf(buf, "gsmReadError=%d", err);
        printS(buf);
    }
    goto lblKO;
  lblKO:
    ret = -1;
    showS("gsmClose", 1);
    gsmClose(gsmHandle);
    printS("KO!");
  lblEnd:
    return ret;
}

static int getSignParam(void) {
    int16 ret;
    uint8 rssi, ber;
    char buf[256];

    showS("SignalParameters", 1);
    ret = gsmGetSignalParameters(gsmHandle, &rssi, &ber);
    sprintf(buf, "rssi=%d", rssi);
    showS(buf, 1);
    printS(buf);
    sprintf(buf, "ber=%d", ber);
    showS(buf, 1);
    printS(buf);
    CHECK(ret == RET_OK, lblKO, "SignalParameters");
    ret = 0;
    goto lblEnd;
  lblKO:
    ret = -1;
    showS("gsmClose", 1);
    gsmClose(gsmHandle);
    printS("KO!");
  lblEnd:
    return ret;
}

static void stopGPRS(void) {
    hmiADClearLine(hHmi, HMI_ALL_LINES);
    showS("gsmClose", 1);
    gsmClose(gsmHandle);
    gsmHandle = 0;
}

void tcik0084(void) {
    int ret;
    uint8 idx, idxOK = 0, idxErr = 0;
    char buf[256];

    hmiOpen("DEFAULT", &hHmi);
    prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    idx = 0;
    do {
        idx++;
        sprintf(buf, "iteration =%d", idx);
        showS(buf, 1);
        printS(buf);
        ret = startGPRS();
        CHECK(ret >= 0, lblKOIter, "startGPRS KO");
        ret = getIMEI();
        CHECK(ret >= 0, lblKOIter, "getIMEI KO");
        ret = setPIN();
        CHECK(ret >= 0, lblKOIter, "setPIN KO");
        ret = getIMSI();
        CHECK(ret >= 0, lblKOIter, "getIMSI KO");
        ret = getNetStat();
        CHECK(ret >= 0, lblKOIter, "getNetStat KO");
        ret = getSignParam();
        CHECK(ret >= 0, lblKOIter, "getSignParam KO");
        idxOK++;
        goto lblEndIter;
      lblKOIter:
        idxErr++;
        psyTimerWakeAfter(SYS_TIME_SECOND);
        stopGPRS();
      lblEndIter:
        printS("________________________");
    } while(idx < 20);
    printS("nonUTA test: IMSI IMSI");
    sprintf(buf, "success  =%d", idxOK);
    printS(buf);
    sprintf(buf, "Fail =%d", idxErr);
    printS(buf);
    psyTimerWakeAfter(SYS_TIME_SECOND);
    goto lblEnd;
  lblKO:
    goto lblEnd;
  lblEnd:
    //Beep();
    stopGPRS();
    hmiClose(hHmi);
    prnClose(hPrt);
}
