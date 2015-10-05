// tcgs0022
//
// author:      Gábor Sárospataki
// date:        01.03.2007
// description: test case to bcrXXX funcions

#include "sys.h"
#include <string.h>

#define CHK CHECK(ret>=0,lblKO)

void tcgs0022(void) {
#ifdef __BCR__
    char buffer[512];
    int ret = -1;

    memset(buffer, 0, sizeof(buffer));

    ret = dspStart();
    CHK;
    ret = dspClear();
    CHK;
    ret = kbdStart(1);
    CHK;
    ret = tmrStart(1, 10 * 100);
    CHK;
    ret = dspLS(0, "Open the BCR");
    ret = bcrStart();
    CHK;
    ret = dspLS(1, "Reading (max 10s)");
    CHK;
    ret = bcrReq();
    ret = dspLS(2, "Press a key is the cancel");
    CHK;
    while(tmrGet(1)) {
        CHECK(!kbdKey(), lblCancel);
        ret = bcrGet(buffer);
        CHK;
        if(ret) {
            Click();
            dspLS(2, "Get barcode:");
            dspLS(3, buffer);
            tmrPause(5);
            goto lblEnd;
        }
    }
    ret = bcrCancel();          //stop the blinking of the reader
    Click();
    ret = dspLS(3, "Timeout");
    CHK;
    tmrPause(5);
    goto lblEnd;
  lblCancel:
    bcrCancel();
    Click();
    dspLS(3, "Canceled");
    tmrPause(5);
    goto lblEnd;
  lblKO:
    Beep();
    Beep();
    Beep();
    dspLS(0, "KO!");
    tmrPause(5);
    goto lblEnd;
  lblEnd:
    tmrStop(1);
    bcrStop();
    kbdStop();
    dspStop();
    return;
#endif
}
