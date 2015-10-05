#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcik0055.
//Functions testing GCLSend/Receive via GPRS connection with GCL

void tcik0055(void) {
    int ret;
    char *ptr;
    char Text[20 + 1];
    byte dReq[1024];
    tBuffer bReq;
    int idx;
    card len;
    byte b;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Gcl conn ...");
    CHECK(ret >= 0, lblKO);

    ptr = "0000|orange.fr|danfr|mq36noqt|82.247.161.69|6789";
    //ptr= "5670|futuraplus|mts|064|172.19.49.10|6789";
    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    CHECK(ret >= 0, lblKO);

    ret = utaGclStart(gclGprs, ptr);
    prtS("utaGclStart OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Gcl conn OK");
    CHECK(ret >= 0, lblKO);

    bufInit(&bReq, dReq, 1024);
    bufReset(&bReq);            //reset request buffer

    ret = bufApp(&bReq, (byte *) "\x00\x00\x00\x0B\x01\x01\x04\x00\x85\x0B\x03\x35\x56\x00\x01", 15);   //binairy xml
    CHECK(ret > 0, lblKO);

    ret = utaGclDial();
    prtS("utaGclDial ...");
    CHECK(ret >= 0, lblKO);
    ret = tmrStart(0, 600 * 100);
    CHECK(ret >= 0, lblKO);
    do {
        ret = utaGclDialInfo();
        switch (ret) {
          case gclStaPreDial:
              ptr = "PreDial...";
              break;
          case gclStaDial:
              ptr = "Dial...";
              break;
          case gclStaConnect:
              ptr = "Connect...";
              break;
          case gclStaLogin:
              ptr = "Login";
              break;
          case gclStaFinished:
              ptr = "Done";
              break;
          default:
              goto lblKO;
        }
        dspLS(2, ptr);
        if(ret == gclStaFinished)
            break;
    } while((ret != gclStaFinished) || (tmrGet(0)));
    prtS("utaGclDial OK");
    dspClear();
    ret = dspLS(3, "tmrGet ...");
    CHECK(ret >= 0, lblKO);
    CHECK(tmrGet(0), lblKO);

    ret = dspLS(3, "utaGclGprsSend..");
    CHECK(ret >= 0, lblKO);
    ret = utaGclSendBuf(bufPtr(&bReq), bufLen(&bReq));
    prtS("utaGclSendBuf OK");
    len = 0;
    for (idx = 0; idx < 4; idx++) {
        ret = utaGclRecv(&b, 10);   //receive characters
        CHECK(ret >= 0, lblKO);
        len *= 0x100;
        len += b;
    }
    prtS("receive length OK");
    prtS("length:");
    num2dec(Text, len, 4);
    prtS(Text);
    bufReset(&bReq);            //reset buffer
    for (idx = 0; idx < len; idx++) {
        ret = utaGclRecv(&b, 10);   //receive characters
        //CHECK(ret >= 0, lblKO);
        bufApp(&bReq, &b, sizeof(byte));
    }
    prtS("utaGclRecv OK");
    trcS("recvRsp: ");
    trcBAN(bufPtr(&bReq), bufLen(&bReq));
    trcS("\n");
    ret = dspLS(1, "Receiving done");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    prtS("KO");
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(1);
    utaGclStop();
    prtS("utaGclStop OK");
    prtStop();
    dspStop();
}
