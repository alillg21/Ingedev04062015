#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcvp0008.
//Functions testing:
//  comConnect
//  comDisconnect
//  comRecvPat

void tcvp0008(void) {
    int ret;
    char *ptr1, *ptr2;
    tBuffer bPat;
    byte dPat[16];
    tBuffer bRsp;
    byte dRsp[256];

	trcS("tcvp0008 Beg\n");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    //Do something...
    ret = dspLS(1, "Press a key...");

    ret = kbdStart(1);          //start keyboard waiting
    CHECK(ret >= 0, lblKO);

    ret = tmrStart(0, 5 * 100);
    while(tmrGet(0)) {
        ret = kbdKey();         //retrieve a key if pressed
        if(ret)
            break;              //quit the loop if a key pressed
    }
    ret = kbdStop();            //stop keyboard waiting

    ret = dspLS(0, "Setting com..."); CHECK(ret >= 0, lblKO);
    ptr1 = "?8N12400";
    ret = dspLS(1, ptr1);
	ret = dspLS(2, "comDial..."); CHECK(ret >= 0, lblKO);
    ptr2 = "0-0836065555";       //ATOS
    ret = dspLS(3, ptr2);
    CHECK(ret >= 0, lblKO);
    ret = comConnect('M', ptr1, ptr2);
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

    ret = dspLS(1, "comDial OK");
    CHECK(ret >= 0, lblKO);

    // Wait for TRANSPAC
	bufInit(&bRsp, dRsp, 256);
    bufInit(&bPat, dPat, 16);
    bufApp(&bPat, (byte *) "TRANSPAC", 8);
    ret = comRecvPat(&bRsp, &bPat, 60);

    ret = dspLS(3, "Receiving done");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    comHangStart();
    comHangWait();
    comStop();
    tmrPause(3);
    dspStop();
	trcS("tcvp0008 End\n");
}
