#include <string.h>
#include "sys.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcik0054.
//Functions testing of GPRS connection via GCL:
//Function testing of GPRS connection, second part of test:
//      GPRS is connected yet
//      Dial to server
//      send message via TCP/IP to server
//      GPRS disconnection
//First part of test tcik0053

void tcik0054(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[1024];

    VERIFY(&buf);
    bufInit(&buf, dat, 1024);
    bufReset(&buf);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "utaGclGprsDial...");
    CHECK(ret >= 0, lblKO);
    ret = utaGclDial();
    CHECK(ret >= 0, lblKO);
    do {
        ret = utaGclDialInfo();
        switch (ret) {
          case gclStaPreDial:
              ptr = "PreDial";
              break;
          case gclStaDial:
              ptr = "Dial";
              break;
          case gclStaConnect:
              ptr = "Connect";
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
    } while(ret != gclStaFinished);
    ret = dspLS(2, "utaGclGprsDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "utaGclGprsRecv..");
    CHECK(ret >= 0, lblKO);
    ret = utaGclRecvBuf(&buf, 0, 100);
    ret = prtS((char *) buf.ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Receiving done");
    CHECK(ret >= 0, lblKO);

    bufReset(&buf);
    bufAppStr(&buf, (byte *) "\x00\x0B\x85\x05\x03MMMMMM\x00\x01");
    ret = prtS((char *) buf.ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "utaGclGprsSend..");
    CHECK(ret >= 0, lblKO);
    ret = utaGclSendBuf(bufPtr(&buf), bufLen(&buf));
    CHECK(ret == bufLen(&buf), lblKO);

    ret = utaGclRecvBuf(&buf, 0, 100);
    //ret= prtS((char *)buf.ptr);CHECK(ret>=0,lblKO);
    ret = dspLS(0, "Receiving done");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    ret = dspLS(3, "utaGclGprsStop");
    CHECK(ret >= 0, lblKO);
    utaGclStop();
    prtStop();
    dspStop();
}
