//Unitary UTA test case tcab0183.
//Functions testing of GPRS GSM plugin:
//  pgnImeiImsi
//  pgnSgnLvl
//  pgnBatLvl

#include "sys.h"
#include "tst.h"

void tcab0183(void) {
    int ret;
    char imei[15 + 1];
    char imsi[15 + 1];
    byte rssi;
    byte sta;
    char buf[dspW + 1];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    memset(imei, 0, sizeof(imei));
    memset(imsi, 0, sizeof(imsi));
    memset(buf, 0, sizeof(buf));

    ret = pgnImeiImsi(imei, imsi);
    //CHECK(ret>=0,lblKO);

    if(ret >= 0) {
        ret = dspLS(0, imei);
        CHECK(ret >= 0, lblKO);
        ret = dspLS(1, imsi);
        CHECK(ret >= 0, lblKO);
    } else {
        ret = dspLS(0, "pgnImeiImsi KO");
        CHECK(ret >= 0, lblKO);
    }
    tmrPause(1);

    rssi = 0;
    sta = 0;
    ret = pgnSgnLvl(&rssi, &sta);
    //CHECK(ret>=0,lblKO);
    if(ret >= 0) {
        num2dec(buf, rssi, 3);
        if(sta)
            strcat(buf, " connected");
        else
            strcat(buf, " NO GPRS");
    } else {
        strcpy(buf, "pgnSgnLvl KO");
    }
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = pgnBatLvl(0);
    //CHECK(ret>=0,lblKO);
    if(ret >= 0) {
        num2dec(buf, ret, 0);
    } else {
        strcpy(buf, "pgnBatLvl KO");
    }
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

  lblKO:
    goto lblEnd;
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
}
