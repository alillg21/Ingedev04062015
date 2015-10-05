// tcgs0024
//
// author:      Gábor Sárospataki
// date:        01.05.2007
// description: test case to dspCol and dspBCol  funcions

#include "sys.h"
static code const char *srcFile = __FILE__;

#define CHK CHECK(ret>=0,lblKO)

static void tmrPause100(int cSec) {
    tmrStart(0, cSec);
    while(tmrGet(0));
    tmrStop(0);
}

void tcgs0024(void) {
    int ret;
    byte bcgCol, fgdCol;

    dspStart();
    ret = dspColMode(1);
    CHK;
    for (bcgCol = colBeg + 1; bcgCol < colEnd; bcgCol++) {
        ret = dspBackCol(bcgCol);
        CHK;
        for (fgdCol = colBeg + 1; fgdCol < colEnd; fgdCol++) {
            ret = dspCol(fgdCol);
            CHK;
            ret = dspLS(1, "  UTA COLOR");
            CHK;
            tmrPause100(20);
        }
    }
    goto lblEnd;
  lblKO:
    Beep();
    Beep();
    Beep();
    dspLS(0, "KO!");
    goto lblEnd;
  lblEnd:
    tmrPause(5);
    dspColMode(0);
    dspStop();
    return;
}
