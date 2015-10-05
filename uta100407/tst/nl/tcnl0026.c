#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcnl0026.
//Functions testing:
//  getAppVer

void tcnl0026(void) {
    int ret;
    char appVer[3 + 1];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "getAppVersion");
    CHECK(ret >= 0, lblKO);

    //ret= getAppVer(appVer,"AE00P4");      
    ret = getAppVer(appVer, "FR004R");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, appVer);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
}
