#include <string.h>
#include "sys.h"
#include "tst.h"

//static code const char *srcFile= __FILE__;

//Unitary test case tcik0094.
//Functions testing isPlugged()

void tcik0094(void) {
    int ret;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Terminal is Plugged ?");
    CHECK(ret >= 0, lblKO);

    ret = isPlugged();
    CHECK(ret >= 0, lblKO);

    if(ret == 0)
        prtS("Terminal is not plugged");
    else
        prtS("Terminal is plugged");

    ret = dspLS(0, "OK");
    CHECK(ret >= 0, lblKO);
    goto lblEnd;
  lblKO:
    trcErr(ret);
    prtS("KO");
    dspLS(1, "KO!");
  lblEnd:
    tmrPause(2);
    dspLS(2, "STOP");
    prtStop();
    dspStop();
}
