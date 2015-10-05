/** \file
 * Integration test case tcik0040
 * GPRS connection with chn
 * Unitary test.
 * \sa
 *  - comStart()
 *  - comSet()
 *  - comRecvBuf()
 *  - comDial()
 *  - comSendBuf()
 */

#include "sys.h"
#include "tst.h"

void tcik0040(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[1024];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, 1024);
    bufReset(&buf);

    //GPRS Connection
    trcS("GPRS: Start\n");
    dspLS(0, "comStart...");
    //prtS("tcik0040");
    tmrPause(1);
    ret = comStart(chnGprs);    //SIM not initialised yet
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comStart OK");

    //ptr= "0000|orange.fr|danfr|mq36noqt|";
    //ptr= "5670|futuraplus|mts|064|";
    //ptr = "0000|shwap|||";
    //ptr = "0000|orange.fr|||";
    ptr = "|internet.globe.com.ph|||";
    trcS(ptr);
    //prtS(ptr);
    tmrPause(1);
    ret = dspLS(0, "comSet...");
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comSet OK");
    //prtS("comSet ok");

    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "DialIP ...");
    ptr = 0;
    ptr = "210.1.78.78|60";
    //ptr = "91.121.18.221|6789";
    //ptr = "212.27.42.12|110";   //free.fr
    //prtS(ptr);
    trcFS("comDial... ptr=%s\n", ptr);
    ret = comDial(ptr);
    CHECK(ret >= 0, lblKO);
    //prtS("comDial ok");

    bufApp(&buf,
           (byte *) "\x00\x00\x00\x0A\x01\x01\x04\x00\x85\x05\x03\33\x00\x01",
           14);

    ret = dspLS(0, "utaGprsSend..");
    CHECK(ret >= 0, lblKO);
    ret = comSendBuf(bufPtr(&buf), bufLen(&buf));
    ret = dspLS(0, "utaGprsSend..OK");
    //prtS("Send");

    bufReset(&buf);
    ret = comRecvBuf(&buf, 0, 100);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "Receiving done");
    CHECK(ret >= 0, lblKO);

    //prtS("Receive");

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
