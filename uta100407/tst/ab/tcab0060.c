#include <string.h>
#include "pri.h"
#include "tst.h"

//Integration test case tcab0060.
//CISCO dialog

static int waitForStar(void) {
/*
<-> x00x00CONNECT.57600/NONE/1200x0Dx00
<-- x7Fcisco.PAD.v1x2E0bx0Dx0AGBC.Rtx0Dx0Ax0Dx0Dx0A*
*/

    byte trm[] = { '*', 0 };
    return comRecvRsp(0, trm, 60);  //wait for *
}

void tcab0060(void) {
    int ret;
    int idx;
    byte b;
    char *ptr;
    char msg[dspW + 1];

    memset(msg, 0, dspW + 1);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comSet...");
    CHECK(ret >= 0, lblKO);
    ptr = "7E11200";            //ERSTE
    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

    ret = comStart(chnMdm);
    CHECK(ret >= 0, lblKO);
    ret = comSet(ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial...");
    CHECK(ret >= 0, lblKO);
    ptr = "0-00-36-14212700";   //ERSTE
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
    ret = waitForStar();
    CHECK(ret > 0, lblKO);
    ptr = "N123456789123456789-12121212306\x0D\x0A";
    ret = comSendReq((byte *) ptr, (word) strlen(ptr));
    CHECK(ret > 0, lblKO);

    for (idx = 0; idx < 64; idx++) {
        ret = comRecv(&b, 20);  //receive characters
        if(ret < 0)
            break;
        if('A' <= b && b <= 'Z') {
            msg[idx % dspW] = (char) b; //display ascii characters
            dspLS(2, msg);
        }
    }

    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    ret = comHangStart();
    CHECK(ret >= 0, lblKO);

    ret = comHangWait();
    CHECK(ret >= 0, lblKO);

    ret = comStop();            //close the port
    CHECK(ret >= 0, lblKO);

    tmrPause(3);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    tmrPause(2);
    comStop();
    dspStop();
}
