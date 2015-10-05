#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcik0081.
//Functions testing for operator selection:

void tcik0081(void) {
    int ret;
    char *ptr;

    //byte rssi=0, ber=0;
    //byte *msg = (byte *) "HELLO!";
    tBuffer buf;
    byte dat[128];

    bufInit(&buf, dat, 1024);
    bufReset(&buf);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = comStart(chnGprs);    //SIM not initialised yet
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comStart OK");

    //ret = comGetSignParams(&rssi,&ber);
    //CHECK(ret >= 0, lblKO);

    ptr = "0000|orange.fr|0|0|";
    trcS(ptr);

    ret = dspLS(0, "comSet...");
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ptr = 0;
    ptr = "212.27.42.12|110";   //free.fr
    trcFS("comDial... ptr=%s\n", ptr);
    ret = comDial(ptr);
    CHECK(ret >= 0, lblKO);

    bufApp(&buf,
           (byte *) "\x00\x00\x00\x0A\x01\x01\x04\x00\x85\x05\x03\33\x00\x01",
           14);

    ret = dspLS(0, "utaGprsSend..");
    CHECK(ret >= 0, lblKO);
    ret = comSendBuf(bufPtr(&buf), bufLen(&buf));
    ret = dspLS(0, "utaGprsSend..OK");

    bufReset(&buf);
    ret = comRecvBuf(&buf, 0, 100);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "Receiving done");
    CHECK(ret >= 0, lblKO);

    tmrPause(3);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    tmrPause(2);
    prtStop();
    dspStop();
}
