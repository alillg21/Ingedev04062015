/** \file
 * Unitary test case tcek0001.
 * Functions testing:
 * \sa
 *  - comStart()
 *  - comSet()
 *  - comRecvBuf()
 */

#include <sdk30.h>
#include "sys.h"
#include "tst.h"

/* Test case using USB Slave port   */
//Connect to PC via USB slave port
//Run Hercules on PC site
//Open related comport corresponding to USB
//Run the test case
//After connection type 8 characters
//For each character received a beep is issued
//After 8 characters a string is sent to Hercules
void tcek0001(void) {
    int ret = -1;
    byte b;
    char msg[dspW + 1];
    tBuffer buf;
    byte dat[256];
    int nCount = 0;
    char *pMsg = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    trcS("#BEG tcek0001#\n");

    bufInit(&buf, dat, sizeof(dat));

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    memset(msg, 0, dspW + 1);

    ret = dspLS(0, "comStart..");
    tmrPause(1);

    ret = comStart(chnUSB);
    CHECK(ret >= 0, lblKOStart);

    ret = dspLS(0, "comSet..");
    tmrPause(1);

    ret = comSet("");
    CHECK(ret >= 0, lblKOSet);
    TRCDT("comSet done");

    ret = dspLS(0, "comDial..");
    tmrPause(1);

    ret = comDial("");
    CHECK(ret >= 0, lblKODial);
	tmrPause(1);
	ret = dspLS(0, "comReceive..");
	tmrPause(1);

    do {
        ret = comRecv(&b, 60);
        if(ret) {
            nCount += ret;
            Beep();
        }
    } while(nCount < 8);

    bufSet(&buf, 0, sizeof(dat));
    ret = comRecvBuf(&buf, (byte *) "C", 60);
    CHECK(ret, lblKOReceive);

    bufCpy(&buf, pMsg, strlen(pMsg) + 1);

    ret = dspLS(0, "comSend..");
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
    trcS("#END tcek0001()#\n");
    return;

}
