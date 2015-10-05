#include <string.h>
#include <unicapt.h>

#include "sys.h"
#include "tst.h"

//Unitary test case tcik0001.
//Testing GPRS connection with choosed IP adr.

void tcik0003(void) {
    int ret;
    char *ptr;
    tBuffer buf;
    byte dat[1024];

    //char temp[16];
    //int i;

    bufInit(&buf, dat, 1024);
    bufReset(&buf);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    trcMode(0xFF);
    trcS("GPRS: Start\n");
    //ptr= "0000|orange.fr|danfr|mq36noqt|82.124.225.95|110";
    ptr = "0000|orange.fr|danfr|mq36noqt|82.247.161.69|6789";
    //ptr= "0000|orange.fr|danfr|mq36noqt|212.27.42.12|110";
    trcFS("valptr: ptr=%s\n", ptr);

    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);
    ret = utaGclStart(gclGprs, ptr);
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
    ret = prtS("utaGclGprsDial OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "Send prepare");
    CHECK(ret >= 0, lblKO);
    bufReset(&buf);
    bufApp(&buf, (byte *)
           "\x00\x00\x00\x27\x01\x01\x04\x00\x85\x05\x03\x43\x00\x01\x00\x06\x03\x44\x45\x00\x07\x03\x33\x38\x31\x36\x34\x31\x32\x34\x33\x31\x34\x35\x00\x08\x03\x31\x30\x30\x0D\x0A",
           43);
    //ret= prtS(buf);CHECK(ret>=0,lblKO);
    ret = prtS("Send ...");
    CHECK(ret >= 0, lblKO);
    ret = utaGclSendBuf(bufPtr(&buf), bufLen(&buf));
    //CHECK(ret==bufLen(&buf),lblKO);
    ret = dspLS(3, "utaGclGprsSend OK");
    CHECK(ret >= 0, lblKO);
    ret = utaGclRecvBuf(&buf, 0, 100);
    ret = prtS((char *) buf.ptr);   //CHECK(ret>=0,lblKO);
    ret = dspLS(3, "Receiving done");   //CHECK(ret>=0,lblKO);
/*
	//1Ko data send
	ret= dspLS(1,"Buffer prepare for 1Ko"); CHECK(ret>=0,lblKO);
	ret= prtS("Buffer prepare for 1Ko");CHECK(ret>=0,lblKO);
    bufReset(&buf);
    for (i=0;i<1000;i++){
     		bufAppStr(&buf,(byte *)"01010101");
    }
     //ret= prtS(&buf);CHECK(ret>=0,lblKO);
     ret= dspLS(3,"utaGclGprsSend.."); CHECK(ret>=0,lblKO);
     ret= utaGclSendBuf(bufPtr(&buf),bufLen(&buf));
     CHECK(ret==bufLen(&buf),lblKO);
     ret= utaGclRecvBuf(&buf,0,100);
     ret= prtS((char *)buf.ptr);CHECK(ret>=0,lblKO);
     ret= dspLS(3,"Receiving done");
     */
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = utaGclDialErr();
    trcFN("valGclDial: ret=%d\n", ret);
    switch (ret) {
      case gclErrCancel:
          ptr = "User cancelled";
          break;
      case gclErrDial:
          ptr = "Dial error";
          break;
      case gclErrBusy:
          ptr = "BUSY";
          break;
      case gclErrNoDialTone:
          ptr = "NO DIAL TONE";
          break;
      case gclErrNoCarrier:
          ptr = "NO CARRIER";
          break;
      case gclErrNoAnswer:
          ptr = "NO ANSWER";
          break;
      case gclErrLogin:
          ptr = "LOGIN PBM";
          break;
      default:
          ptr = "KO!";
          break;
    }
    dspLS(0, ptr);
  lblEnd:
    tmrPause(2);
    ret = dspLS(0, "utaGclGprsStop");
    utaGclStop();
    prtStop();
    dspStop();
    trcS("GPRS: End\n");
}
