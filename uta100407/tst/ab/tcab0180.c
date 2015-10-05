//Unitary nonUTA test case tcab0180.
//Functions testing of GPRS GSM plugin:
//  gmaInqGsmSignLevel
//  gmaInqGsmImeiImsi

#include <stdio.h>
#include <string.h>             // for memset, strcpy, etc
#include <unicapt.h>
#include <gmaInqStatus.h>

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

void tcab0180(void) {
    int ret;
    uint8 rssi, gprsStatus;
    char imsi[15 + 1];
    char imei[15 + 1];
    char buf[256 + 1];

    hmiOpen("DEFAULT", &hHmi);
    prnOpen("DEFAULT", &hPrt, PRN_IMMEDIATE);
    printS("nonUTA test: IMSI IMSI");

    ret = gmaInqGsmImeiImsi(imei, imsi);
    CHECK(ret >= 0, lblKO, "gmaInqGsmImeiImsi KO");
    printS("IMEI");
    showS(imei, 1);
    printS(imei);
    printS("IMSI");
    showS(imsi, 1);
    printS(imsi);

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
    hmiClose(hHmi);
    prnClose(hPrt);
}
