#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcfa0001 copy from tcab0026.
//Functions testing:

//  enterDate
//  enterTime

void tcfa0001(void) {
    int ret;
    char buf[dspW + 1];

    dspClear();
    ret = dspLS(0, "enterDate");
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, dspW + 1);
    ret = enterDate(1, buf, 6);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    dspClear();
    ret = dspLS(0, "enterTime");
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, dspW + 1);
    ret = enterTime(1, buf, 6);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    dspStop();
}
