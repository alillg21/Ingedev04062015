#include <string.h>
#include "sys.h"
#include "tst.h"

//Integration test case tcab0071.
//Testing PPP connection with free.fr

void tcab0071(void) {
#ifdef __TEST__
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[128];

    bufInit(&buf, dat, 128);

    //trcMode(1);

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    TRCDT("comStart beg");
    ret = comStart(chnPpp);
    CHECK(ret >= 0, lblKO);
    TRCDT("comStart end");
    ret = dspLS(0, "comStart OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comSet...");
    CHECK(ret >= 0, lblKO);
    TRCDT("comSet beg");
    ret = comSet("0,0860922000|danfr|mq36noqt|60");
    CHECK(ret >= 0, lblKO);     //free.fr
    TRCDT("comSet end");
    ret = dspLS(1, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "comDial...");
    CHECK(ret >= 0, lblKO);
    ptr = "212.27.42.12-110";   //free.fr
    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comDial beg");
    ret = comDial(ptr);
    TRCDT("comDial end");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, "comDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comRecvBuf start");
    CHECK(ret >= 0, lblKO);
    TRCDT("comRecvBuf beg");
    ret = comRecvBuf(&buf, (byte *) ">", 128);
    CHECK(ret > 0, lblKO);
    TRCDT("comRecvBuf end");
    ret = dspLS(0, "comRecvBuf done");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, (char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    tmrPause(2);
    comStop();
    dspStop();
#endif
}
