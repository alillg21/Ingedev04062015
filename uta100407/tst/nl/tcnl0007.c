#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcnl0007.
//Functions testing:
//  enterTxt

void tcnl0007(void) {
    int ret;
    char buf[25 + 1];
    byte fmt = '*';

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    dspClear();
    ret = dspLS(0, "enterTxt");
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, dspW + 1);
    ret = enterTxt(1, buf, 33 + 1, fmt);
    prtS(buf);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);

    fmt = 0xff;
    memset(buf, 0, dspW + 1);
    dspClear();
    ret = dspLS(0, "enterTxt");
    ret = enterTxt(1, buf, 9 + 1, fmt);
    prtS(buf);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:
    tmrPause(3);
    dspStop();
}
