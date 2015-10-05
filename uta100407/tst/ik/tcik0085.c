//Unitary nonUTA test case tcik0084.
//Functions testing of GPRS GCL plugin:
//  gmaInqGsmSignLevel

#include <stdio.h>
#include <string.h>             // for memset, strcpy, etc

#include <unicapt.h>

// for GSM
//#include "menusup.h"
#include "gmaUtil.h"
#include "gsmconf.h"
//#include "gcl.h"
//#include "gcl_gsm.h"
//#include <gsm.h>

//static uint32 rcvDly=30;
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

void tcik0085(void) {
    int ret;
    uint8 rssi, gprsStatus;
    char buf[256];
    const char *ptr;

    hmiOpen("DEFAULT", &hHmi);
    prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    printS("nonUTA test: IMSI IMSI");

    //ret=getGprsInit(rcvDly); //All initialisation will do by GSM plugin

    printS("IMEI");
    ptr = (char *) gsmcReturnImei();
    strcpy(buf, ptr);
    showS(buf, 1);
    printS(buf);

    printS("IMSI");
    ptr = (char *) gsmcReturnImsi();
    strcpy(buf, ptr);
    showS(buf, 1);
    printS(buf);

    ret = gmaInqGsmSignLevel(&rssi, &gprsStatus);
    CHECK(ret >= 0, lblKO, "getSignParam KO");
    sprintf(buf, "rssi =%d", rssi);
    showS(buf, 1);
    printS(buf);
    sprintf(buf, "gprsStatus =%d", gprsStatus);
    showS(buf, 1);
    printS(buf);

    psyTimerWakeAfter(SYS_TIME_SECOND);
    goto lblEnd;
  lblKO:
    goto lblEnd;
  lblEnd:
    //stopGPRS();
    hmiClose(hHmi);
    prnClose(hPrt);
}
