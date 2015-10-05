/** \file 
 * Unitary test case for pri component(tcik0149.c): internal modem (HDLC) testing.
 * \sa
 *  - comStart()
 *  - comSet()
 *  - comStop()
 *  - comDial()
 *  - comHangStart()
 *  - comHangWait()
 *  - comRecv()
 *  - comRecvBuf()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcik0149(void) {
    int ret;
    char *ptr;
    char msg[dspW + 1];
    tBuffer buf;
    byte dat[256];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    memset(msg, 0, dspW + 1);

    ret = comStart(chnHdlc);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet...");
    CHECK(ret >= 0, lblKO);
    ptr = "8N11200";

    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comSet...");

    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    TRCDT("comSet done");

    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial...");
    CHECK(ret >= 0, lblKO);

    //ptr = "0-0146258134";       //NCC
    ptr = "18134";              //NCC
    //ptr = "0-0836065555";       //France

    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comDial...");

    ret = comDial(ptr);
    if(ret < 0) {
        switch (-ret) {
          case comAnn:
              ret = dspLS(2, "comDial Aborted");
              break;
          case comBusy:
              ret = dspLS(2, "comDial BUSY");
              break;
          case comNoDialTone:
              ret = dspLS(2, "NO TONE");
              break;
          case comNoCarrier:
              ret = dspLS(2, "NO CARRIER");
              break;
          default:
              ret = dspLS(2, "UNKNOWN PB");
              break;
        }
        goto lblEnd;
    }
    CHECK(ret >= 0, lblKO);
    TRCDT("comDial done");
    ret = dspLS(1, "comDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Sending ...");
    CHECK(ret >= 0, lblKO);

    memcpy(msg, "testCC", 6 + 1);
    ret = comSendBuf(msg, sizeof(msg));
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "Receiving buf");
    CHECK(ret >= 0, lblKO);
    bufInit(&buf, dat, 256);

    ret = comRecvBuf(&buf, (byte *) "C", 20);
    CHECK(ret >= 0, lblKO);

    memcpy(msg, bufPtr(&buf), dspW);
    dspLS(2, msg);
    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = comHangStart();
    CHECK(ret >= 0, lblKO);

    ret = comHangWait();
    CHECK(ret >= 0, lblKO);

    ret = comStop();            //close the port
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    comStop();
    dspStop();
}
