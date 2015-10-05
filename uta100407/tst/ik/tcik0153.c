/** \file 
 * Unitary test case for pri component(tcab0008.c): Ethernet TCP IP  and PPP testing.
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

void tcik0153(void) {
    int ret;
    int idx;
    byte b;
    char *ptr = 0;
    char msg[dspW + 1];
    tBuffer buf;
    byte dat[256];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    memset(msg, 0, dspW + 1);

    ret = comStart(chnTcp5100);
    //ret = comStart(chnPpp);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet...");
    CHECK(ret >= 0, lblKO);

    //ptr = "10.110.1.128|16000|||10.110.1.127|"; //NCC, force local adress
    //ptr = "10.110.1.128|16000||||"; //NCC, empty local adress, DHCP option is set in Teluim manager
   // ptr = "91.121.18.221|6789||||"; //DVK TOP
   // ptr = "010.017.032.130|2000||||";   //JJ
    //ptr = "82.247.161.69|6789||||";   //
    //ptr = "010.010.203.230|1000||||";   //

    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

    TRCDT("comSet...");
    
    ptr = ""; //DHCP

    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);
    TRCDT("comSet done");

    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial...");
    CHECK(ret >= 0, lblKO);

    ptr = "82.247.161.69|6789";

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

    ret = dspLS(0, "comSend..");
    CHECK(ret >= 0, lblKO);
    ret = comSendBuf((const byte *) "coucou", 6);
    ret = dspLS(0, "comSend..OK");

    ret = dspLS(3, "Receiving per char");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 6; idx++) {
        ret = comRecv(&b, 20);  //receive characters
        if(ret < 0)
            break;
        bin2hex(msg + idx * 2, &b, 2);
        dspLS(2, msg);
    }
    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSend..");
    CHECK(ret >= 0, lblKO);
    ret = comSendBuf((const byte *) "moumouC", 6);
    ret = dspLS(0, "comSend..OK");

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
