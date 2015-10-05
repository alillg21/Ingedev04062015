#include <string.h>
#include "pri.h"
#include "tst.h"

//Integration test case tcab0068.
//Testing a handshake with ACI by TCP

static int waitForEnq(void) {
    byte trm[] = { cENQ, 0 };
    return comRecvRsp(0, trm, 60);  //wait for ENQ
}

#define CHK CHECK(ret>=0,lblKO)
static int makeHdr(tBuffer * req, const char *sub, const char *cod) {
    int ret;
    char dt[12 + 1];

    ret = bufApp(req, (byte *) "9.", 0);
    CHK;                        //Device Type
    ret = bufApp(req, (byte *) "00", 0);
    CHK;                        //Transmission Number
    ret = bufApp(req, (byte *) "12345000        ", 0);
    CHK;                        //Terminal ID
    ret = bufApp(req, (byte *) "      ", 0);
    CHK;                        //Employee ID
    ret = getDateTime(dt);
    ret = bufApp(req, (byte *) dt, 0);
    CHK;                        //Current Date/Time
    ret = bufApp(req, (byte *) sub, 0);
    CHK;                        //Message Type
    ret = bufApp(req, (byte *) "O", 0);
    CHK;                        //Message SubType
    ret = bufApp(req, (byte *) cod, 0);
    CHK;                        //Transaction Code
    ret = bufApp(req, (byte *) "000", 0);
    CHK;                        //Processing Flags
    ret = bufApp(req, (byte *) "000", 0);
    CHK;                        //Response Code
    return bufLen(req);
  lblKO:
    return -1;
}

static int send95(void) {
    int ret;
    byte lrc;
    byte dReq[512];
    tBuffer bReq;

    bufInit(&bReq, dReq, 512);
    ret = bufApp(&bReq, bPtr(cSTX), 1);
    CHK;
    ret = makeHdr(&bReq, "A", "95");
    CHK;
    ret = bufApp(&bReq, bPtr(cETX), 1);
    CHK;

    lrc = stdLrc(bufPtr(&bReq) + 1, bufLen(&bReq) - 1); //calculate LRC
    ret = bufApp(&bReq, &lrc, 1);
    CHK;                        //append LRC
    ret = comSendReq(bufPtr(&bReq), bufLen(&bReq)); //send all the pack STX-DATA-ETX-LRC
    CHECK(ret == bufLen(&bReq), lblKO);
    return bufLen(&bReq);
  lblKO:
    return -1;
}

static int send00(void) {
    int ret;
    byte lrc;
    byte dReq[512];
    tBuffer bReq;

    bufInit(&bReq, dReq, 512);
    ret = bufApp(&bReq, bPtr(cSTX), 1);
    CHK;
    ret = makeHdr(&bReq, "F", "00");
    CHK;

    //9x2E00000000011.............041117150026FO00000000
    //x1CB123
    //x1Ch0010010011
    //x1Cq;4568941111111119=04122011788837300000?
    //x1CS00000001
    //x03

    ret = bufApp(&bReq, bPtr(cFS), 1);
    CHK;
    ret = bufApp(&bReq, (byte *) "B", 0);
    CHK;
    ret = bufApp(&bReq, (byte *) "123", 0);
    CHK;

    ret = bufApp(&bReq, bPtr(cFS), 1);
    CHK;
    ret = bufApp(&bReq, (byte *) "h", 0);
    CHK;
    ret = bufApp(&bReq, (byte *) "0010010011", 0);
    CHK;

    ret = bufApp(&bReq, bPtr(cFS), 1);
    CHK;
    ret = bufApp(&bReq, (byte *) "q", 0);
    CHK;
//    ret= bufApp(&bReq,(byte *)"4568941111111119=04122011788837300000?",0); CHK;
//    ret= bufApp(&bReq,(byte *)"M4567890000000118=0701?",0); CHK;
    ret = bufApp(&bReq, (byte *) ";4761739001010010=10122011143878089?", 0);
    CHK;

    ret = bufApp(&bReq, bPtr(cFS), 1);
    CHK;
    ret = bufApp(&bReq, (byte *) "S", 0);
    CHK;
    ret = bufApp(&bReq, (byte *) "00000001", 0);
    CHK;

    ret = bufApp(&bReq, bPtr(cETX), 1);
    CHK;

    lrc = stdLrc(bufPtr(&bReq) + 1, bufLen(&bReq) - 1); //calculate LRC
    ret = bufApp(&bReq, &lrc, 1);
    CHK;                        //append LRC
    ret = comSendReq(bufPtr(&bReq), bufLen(&bReq)); //send all the pack STX-DATA-ETX-LRC
    CHECK(ret == bufLen(&bReq), lblKO);
    return bufLen(&bReq);
  lblKO:
    return -1;
}

static int recvRsp(void) {
    int ret;
    byte dRsp[512];
    tBuffer bRsp;
    byte *rsp;
    word len;

    ret = waitForEnq();
    CHECK(ret > 0, lblKO);

    bufInit(&bRsp, dRsp, 512);
    ret = comRecvStxDataEtxLrc(&bRsp, 60);  //receive package
    CHECK(ret >= 0, lblKO);

    rsp = (byte *) bufPtr(&bRsp);
    len = bufLen(&bRsp);
    while(len--)
        *rsp++ &= 0x7F;

    trcFS("rsp= %s\n", (char *) bufPtr(&bRsp));
    return bufLen(&bRsp);
  lblKO:
    return -1;
}

static int test95(void) {
    int ret;

    ret = dspLS(0, "HANDSHAKE");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "send req");
    CHECK(ret >= 0, lblKO);
    ret = send95();
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "req OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "wait for rsp");
    CHECK(ret >= 0, lblKO);
    ret = recvRsp();
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "rsp OK");
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

static int test00(void) {
    int ret;

    ret = dspLS(0, "PURCHASE");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "send req");
    CHECK(ret >= 0, lblKO);
    ret = send00();
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "req OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "wait for rsp");
    CHECK(ret >= 0, lblKO);
    ret = recvRsp();
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "rsp OK");
    CHECK(ret >= 0, lblKO);

    return 1;
  lblKO:
    return -1;
}

void tcab0068(void) {
    int ret;
    char *ptr;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    ret = comStart(chnTcp5100);
    CHECK(ret >= 0, lblKO);
    ret = comSet("");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial...");
    CHECK(ret >= 0, lblKO);
    ptr = "206.201.21.23-21002";    //ACI
    ret = dspLS(3, ptr);
    CHECK(ret >= 0, lblKO);

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
    ret = dspLS(1, "comDial OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

//    ret= dspLS(2,"wait for ENQ"); CHECK(ret>=0,lblKO); 
//    ret= waitForEnq(); CHECK(ret>0,lblKO); 
//    ret= dspLS(2,"ENQ OK"); CHECK(ret>=0,lblKO); 

    ret = 0;
    switch (ret) {
      case 0:
          ret = test00();
          break;
      case 95:
          ret = test95();
          break;
      default:
          ret = -1;
          break;
    }
    CHECK(ret > 0, lblKO);

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
    prtStop();
    dspStop();
}
