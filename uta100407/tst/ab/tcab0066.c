#include <string.h>
#include "sys.h"
#include "tst.h"

//Integration test case tcab0066.
//Testing timer event processing

int tcab0066(card id, card * date, card * time) {
    int ret;
    char YYMMDDhhmmss[12 + 1];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    memset(YYMMDDhhmmss, 0, 12 + 1);

    //ret= prtS("Date:"); CHECK(ret>=0,lblKO);
    dat2asc(YYMMDDhhmmss, *date);
    //ret= prtS(YYMMDDhhmmss); CHECK(ret>=0,lblKO);

    //ret= prtS("Time:"); CHECK(ret>=0,lblKO);
    tim2asc(YYMMDDhhmmss + 6, *time);
    //ret= prtS(YYMMDDhhmmss+6); CHECK(ret>=0,lblKO);

    ret = dspLS(0, "Date/Time:");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, YYMMDDhhmmss);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    tmrPause(1);
    prtStop();
    dspStop();
    return 0;
}
