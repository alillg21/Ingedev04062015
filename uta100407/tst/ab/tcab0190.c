#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcab0190.
//Functions testing:
//  pgnSmfStart
//  reboot

void tcab0190(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "pgnSmfStart...");
    CHECK(ret >= 0, lblKO);

    pgnSmfStart();

    ret = dspLS(1, "pgnSmfStart Done");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "Now reboot...");
    CHECK(ret >= 0, lblKO);
    tmrPause(3);

    reboot();

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
}
