#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcnl0019.
//Amount Input in farsi

#define CHK CHECK(ret>=0,lblKO)

void tcnl0019(void) {
    int ret;
    char str[30] = "";

//  ret= cryStart('M'); CHECK(ret>=0,lblKO);
    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    //ret= usrInfo(infAmtInput); CHECK(ret>=0,lblKO); 
    ret = dspLS(1, "       \xe6\xf3\x93\xf5 \xf1\xa1\xa2\x90");
    CHECK(ret >= 0, lblKO);
    ret = enterFmtStrFarsi(2, str, 2, 12);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    dspStop();
//  cryStop('M');
}
