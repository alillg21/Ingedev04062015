/** \file
 * Unitary UTA test case : socket connection chnSck for Teluim
 * Transport layer is managed by Teluim manager
 * please config Network access on it
 */

#include "sys.h"
#include "tst.h"

void tcik0162(void) {
    int ret = 0;
    char *ptr;
    tBuffer buf;
    byte dat[1024];

    trcS("   ***   tcik0162   ***   \n");
    bufInit(&buf, dat, 1024);
    bufReset(&buf);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    dspLS(0, "comStart...");
    ret = comStart(chnSck);
    CHECK(ret >= 0, lblKO);
    //ptr = "10.110.1.128|16000|";
    ptr = "91.121.18.221|6789|";
    //ptr = "";
    //ptr = "212.27.48.10|80|";
    trcS(ptr);
    tmrPause(1);
    dspLS(1, "comSet...");
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    //ptr = "91.121.18.221|6789|";
    dspLS(1, "comDial...");
    ret = comDial(ptr);
    CHECK(ret >= 0, lblKO);
    bufApp(&buf,
           (byte *) "\x00\x00\x00\x0A\x01\x01\x04\x00\x85\x05\x03\33\x00\x01",
           14);
    dspLS(0, "comSendBuf...");
    ret = comSendBuf(bufPtr(&buf), bufLen(&buf));
    CHECK(ret >= 0, lblKO);

    dspLS(1, "comRecvBuf...");
    bufReset(&buf);
    ret = comRecvBuf(&buf, 0, 10);
    CHECK(ret >= 0, lblKO);

    trcS("rcv = ");
    trcS(bufPtr(&buf));
    trcS("\n");
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "Test KO");
  lblEnd:
    ret = comHangStart();
    trcFN("comHangStart : ret=%d\n", ret);
    ret = comHangWait();
    trcFN("comHangWait : ret=%d\n", ret);
    ret = comStop();
    trcFN("comStop : ret=%d\n", ret);
    prtStop();
    dspStop();
    tmrPause(3);
    trcS("End of tcik0162\n");
}
