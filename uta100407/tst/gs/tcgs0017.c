// tcgs0017
//
// author:      Gábor Sárospataki
// date:        27.09.2006
// description: test case to fmtAmt function

#include "pri.h"
#include <string.h>
#define CHK CHECK(ret>=0,lblKO)

void tcgs0017(void) {
    int ret;
    char buf[prtW + 1];

    ret = dspStart();
    CHK;
    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtAmt normal");
    CHECK(ret >= 0, lblKO);

    ret = fmtAmt(buf, "1", 2, 0);   //format 2-decimal amount
    CHECK(ret == 4, lblKO);
    CHECK(strcmp(buf, "0.01") == 0, lblKO);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtAmt(buf, "123", 3, "");    //format 3-decimal amount
    CHECK(ret == 5, lblKO);
    CHECK(strcmp(buf, "0.123") == 0, lblKO);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtAmt(buf, "12345", 0, 0);   //format amount without decimals
    CHECK(ret == 5, lblKO);
    CHECK(strcmp(buf, "12345") == 0, lblKO);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtAmt with ,");
    CHECK(ret >= 0, lblKO);

    ret = fmtAmt(buf, "1", 2, ","); //format 2-decimal amount
    CHECK(ret == 4, lblKO);
    CHECK(strcmp(buf, "0,01") == 0, lblKO);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtAmt(buf, "123", 3, ",");   //format 3-decimal amount
    CHECK(ret == 5, lblKO);
    CHECK(strcmp(buf, "0,123") == 0, lblKO);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtAmt(buf, "12345", 0, ","); //format amount without decimals
    CHECK(ret == 5, lblKO);
    CHECK(strcmp(buf, "12345") == 0, lblKO);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);
    ret = 1;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "With ',' and '|'");
    CHECK(ret >= 0, lblKO);

    ret = fmtAmt(buf, "1", 2, ",|");    //format 2-decimal amount
    CHECK(ret == 4, lblKO);
    CHECK(strcmp(buf, "0,01") == 0, lblKO);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtAmt(buf, "123", 3, ",|");  //format 3-decimal amount
    CHECK(ret == 5, lblKO);
    CHECK(strcmp(buf, "0,123") == 0, lblKO);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtAmt(buf, "12345", 0, ",|");    //format amount without decimals thousand separator
    CHECK(ret == 6, lblKO);
    CHECK(strcmp(buf, "12|345") == 0, lblKO);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtAmt(buf, "123456789", 2, ",|");    //format amount with 2 decimals and thousand separator
    CHECK(ret == 12, lblKO);
    CHECK(strcmp(buf, "1|234|567,89") == 0, lblKO);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);
    ret = 1;

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    dspStop();
}
