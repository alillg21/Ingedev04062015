/// \file tcik0160.c

#include <string.h>
#include "sys.h"
#include "tst.h"

/** Integration test case for sys component: send TCP IP packet via GPRS .
 * \sa
 */

void tcik0160(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[1024 * 16];
    card len, idx;

    //char print[24];
    byte binLen[4];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    len = 3;

    len = ((1024 * len) - 4);
    bufInit(&buf, dat, len);
    bufReset(&buf);
    //sprintf(print,"len %d",len);
    //prtS(print);
    ret = num2bin(binLen, len, 4);
    bufApp(&buf, (byte *) binLen, 4);
    bufApp(&buf,
           (byte *) "\x33\x44\x55\x01\x04\x00\x85\x05\x03\33\x00\x01\xEE\xFF",
           12);
    for (idx = 0; idx < ((len / 16) - 1); idx++) {
        bufApp(&buf, (byte *)
               "\x11\x22\x33\x44\x55\x01\x04\x00\x85\x05\x03\33\x00\x01\xEE\xFF",
               16);
    }

    trcS("GPRS: Start\n");
    dspLS(0, "comStart...");
    tmrPause(1);
    ret = comStart(chnGprs);    //SIM not initialised yet
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comStart OK");

    ptr = "0000|orange.fr|||";
    trcS(ptr);
    tmrPause(1);
    ret = dspLS(0, "comSet...");
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "DialIP ...");
    ptr = 0;
    ptr = "91.121.18.221|6789";
    trcFS("comDial... ptr=%s\n", ptr);
    ret = comDial(ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "utaGprsSend..");
    CHECK(ret >= 0, lblKO);
    ret = comSendBuf(bufPtr(&buf), bufLen(&buf));
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "utaGprsSend..OK");
    prtS("donne");

    bufReset(&buf);
    ret = comRecvBuf(&buf, 0, 100);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "Receiving done");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(1, "Connection Fail");
    goto lblEnd;
  lblEnd:
    trcS("GPRS: End\n");
    ret = comHangStart();
    trcFN("comHangStart : ret=%d\n", ret);
    ret = comHangWait();
    trcFN("comHangWait : ret=%d\n", ret);
    ret = comStop();
    trcFN("comStop : ret=%d\n", ret);
    prtStop();
    dspStop();
    tmrPause(3);
}
