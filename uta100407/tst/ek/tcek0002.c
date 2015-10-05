/** \file
 * Unitary test case tcek0002.
 * Functions testing:
 * \sa
 *  - comStart()
 *  - comSet()
 *  - comRecvBuf()
 */

#include <sdk30.h>
#include "sys.h"
#include "tst.h"

/* Test case using RS232 port   */

void tcek0002(void) {
    int ret = -1;
    byte b;
    char msg[dspW + 1];
    tBuffer buf;
    byte dat[256];
    int nCount = 0;
    char *ptr = NULL;
    char *pMsg = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    trcS("#BEG tcek0002#\n");

    bufInit(&buf, dat, sizeof(dat));

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    memset(msg, 0, dspW + 1);

    ret = dspLS(0, "comStart..");
    tmrPause(1);

    ret = comStart(chnCom1);
    CHECK(ret >= 0, lblKOStart);

    ret = dspLS(0, "comSet..");
    tmrPause(1);

    ptr = "8N119200";
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKOSet);
    TRCDT("comSet done");

    ret = dspLS(0, "comDial..");
    tmrPause(1);

    ret = comDial("");
    CHECK(ret >= 0, lblKODial);

    bufCpy(&buf, pMsg, strlen(pMsg) + 1);

    ret = dspLS(0, "comSend..");
    ret = comSendBuf(bufPtr(&buf), bufDim(&buf));
    CHECK(ret, lblKOSend);
    ret = dspLS(1, "comRecv..");
    do {
        ret = comRecv(&b, 60);
        if(ret < 0)
            break;
        if(ret)
            nCount += ret;
    } while(nCount < 8);

    ret = dspLS(2, "comRecvBuf..");
    bufSet(&buf, 0, sizeof(dat));
    ret = comRecvBuf(&buf, (byte *) "C", 60);
    CHECK(ret, lblKOReceive);

    bufCpy(&buf, pMsg, strlen(pMsg) + 1);

    ret = dspLS(0, "comSendBuf..");
    tmrPause(1);
    ret = comSendBuf(bufPtr(&buf), bufDim(&buf));
    CHECK(ret, lblKOSend);
    goto lblOK;

  lblKO:
    trcS("Pre-req Failed\n");
    goto lblEnd;
  lblKOStart:
    trcS("comStart Failed\n");
    goto lblEnd;
  lblKOSet:
    trcS("comSet Failed\n");
    goto lblEnd;
  lblKODial:
    trcS("comDial Failed\n");
    goto lblEnd;
  lblKOReceive:
    trcS("comReceive Failed\n");
    goto lblEnd;
  lblKOSend:
    trcS("comSend Failed\n");
    goto lblEnd;
  lblOK:
    trcFS("Tx Message : %s\n", bufPtr(&buf));
  lblEnd:
    trcErr(ret);
    comStop();
    trcS("#END tcek0002()#\n");
    return;

}
