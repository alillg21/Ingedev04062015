/** \file
 * Integration test case tcik0109.
 * Testing a handshake with SIMTEL Rwanda
 * \sa
 *  - comSendReq()
 *  - comRecvBuf()
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

static int waitForEnq(void) {
    byte trm[] = { cENQ, 0 };
    return comRecvRsp(0, trm, 60);  //wait for ENQ
}

#define CHK CHECK(ret>=0,lblKO)
static int makeHdr(tBuffer * req, const char *sub, const char *cod,
                   const char *prcflg) {
    int ret;
    char dt[12 + 1];

    ret = bufApp(req, (byte *) "9.", 0);
    CHK;                        //Device Type
    ret = bufApp(req, (byte *) "00", 0);
    CHK;                        //Transmission Number
    //ret = bufApp(req, (byte *) "1010000301      ", 0);
    ret = bufApp(req, (byte *) "0100009001      ", 0);
    CHK;                        //Terminal ID
    ret = bufApp(req, (byte *) "000001", 0);
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
    ret = bufApp(req, (byte *) prcflg, 0);
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
    bufReset(&bReq);
    ret = bufApp(&bReq, bPtr(cSTX), 1);
    CHK;
    ret = makeHdr(&bReq, "A", "95", "000");
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

static int build00(tBuffer * req) {
    int ret;
    byte lrc;

    //byte dReq[512];
    //tBuffer bReq;

    //bufInit(&bReq, dReq, 512);
    ret = bufApp(req, bPtr(cSTX), 1);
    CHK;
    ret = makeHdr(req, "F", "00", "050");
    CHK;

    //9x2E00000000011.............041117150026FO00000000
    //x1CB123
    //x1Ch0010010011
    //x1Cq;4568941111111119=04122011788837300000?
    //x1CS00000001
    //x03

    ret = bufApp(req, bPtr(cFS), 1);
    CHK;
    ret = bufApp(req, (byte *) "B", 0); //Amount
    CHK;
    ret = bufApp(req, (byte *) "25000", 0);
    CHK;

    ret = bufApp(req, bPtr(cFS), 1);
    CHK;
    ret = bufApp(req, (byte *) "d", 0); //Retailer ID
    CHK;
    //ret = bufApp(&bReq, (byte *) "10100003", 0);
    ret = bufApp(req, (byte *) "00100009", 0);
    CHK;

    ret = bufApp(req, bPtr(cFS), 1);    //sequence number
    CHK;
    ret = bufApp(req, (byte *) "h", 0);
    CHK;
    ret = bufApp(req, (byte *) "0000000001", 0);
    CHK;

    ret = bufApp(req, bPtr(cFS), 1);
    CHK;
    ret = bufApp(req, (byte *) "q", 0); // Track2
    CHK;
    ret = bufApp(req, (byte *) ";4132370031001019=07106061004353800000?", 0);
    //ret = bufApp(&bReq, (byte *) ";4132370031001019=071006061004353800000", 0);
    CHK;

    ret = bufApp(req, bPtr(cFS), 1);
    CHK;
    ret = bufApp(req, (byte *) "6", 0);
    CHK;

    ret = bufApp(req, bPtr(cRS), 1);
    CHK;
    ret = bufApp(req, (byte *) "E", 0); //terminal entry mode
    CHK;
    ret = bufApp(req, (byte *) "051", 0);
    CHK;

    ret = bufApp(req, bPtr(cRS), 1);
    CHK;
    ret = bufApp(req, (byte *) "I", 0); //transaction currency code
    CHK;
    ret = bufApp(req, (byte *) "646", 0);
    CHK;

    ret = bufApp(req, bPtr(cRS), 1);
    CHK;
    ret = bufApp(req, (byte *) "O", 0); //EMV request data
    CHK;
    ret = bufApp(req, (byte *)
                 "0080646071005FEB8BA489DA7F8805C000065CDDF93DE00000080000006010A03A42000",
                 0);
    CHK;

    ret = bufApp(req, bPtr(cRS), 1);
    CHK;
    ret = bufApp(req, (byte *) "P", 0); //EMV additional request data
    CHK;
    ret = bufApp(req, (byte *) "000122", 0);
    CHK;

    ret = bufApp(req, bPtr(cFS), 1);
    CHK;

    ret = bufApp(req, bPtr(cETX), 1);
    CHK;

    lrc = stdLrc(bufPtr(req) + 1, bufLen(req) - 1); //calculate LRC
    ret = bufApp(req, &lrc, 1);
    CHK;                        //append LRC
    /*
       ret = comSendReq(bufPtr(&bReq), bufLen(&bReq)); //send all the pack STX-DATA-ETX-LRC
       CHECK(ret == bufLen(&bReq), lblKO);
     */
    return bufLen(req);
  lblKO:
    return -1;
}

static int send90(void) {
    int ret;
    byte lrc;
    byte dReq[512];
    tBuffer bReq;

    bufInit(&bReq, dReq, 512);
    bufReset(&bReq);
    ret = bufApp(&bReq, bPtr(cSTX), 1);
    CHK;
    ret = makeHdr(&bReq, "A", "90", "080");
    CHK;

    //x029x2E3901005284...........
    //...060118145451AO90080..
    //.x1CV001000000000000x03~

    ret = bufApp(&bReq, bPtr(cFS), 1);
    CHK;
    ret = bufApp(&bReq, (byte *) "V", 0);
    CHK;
    ret = bufApp(&bReq, (byte *) "1001000000000000", 0);    //download key
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
    byte trm[] = { cEOT, cETX, cNAK, 0x80 | cETX, 0x80 | cNAK, 0 }; //take in account parity bit

    //ret = waitForEnq();
    //CHECK(ret > 0, lblKO);

    bufInit(&bRsp, dRsp, 512);

    ret = comRecvBuf(&bRsp, trm, 120);
    //ret = comRecvStxDataEtxLrc(&bRsp, 120);  //receive package
    CHECK(ret >= 0, lblKO);

    rsp = (byte *) bufPtr(&bRsp);
    len = bufLen(&bRsp);
    /*
       while(len--)
       *rsp++ &= 0x7F;
     */
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

static int test00(tBuffer * req) {
    int ret;

    ret = dspLS(0, "PURCHASE");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "send req");
    CHECK(ret >= 0, lblKO);
    ret = comSendReq(bufPtr(req), bufLen(req)); //send all the pack STX-DATA-ETX-LRC
    CHECK(ret == bufLen(req), lblKO);
    //ret = send00();
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

static int test90(void) {
    int ret;

    ret = dspLS(0, "DOWNLOAD");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "send req");
    CHECK(ret >= 0, lblKO);
    ret = send90();
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

void tcik0109(void) {
    int ret;
    char *ptr;
    byte dReq[512];
    tBuffer bReq;

    bufInit(&bReq, dReq, 512);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = 0;
    switch (ret) {
      case 0:
          ret = build00(&bReq);
          break;
      case 95:
          //ret = build95(&bReq);
          break;
      case 90:
          //ret = build90(&bReq);
          break;
      default:
          ret = -1;
          break;
    }
    CHECK(ret > 0, lblKO);

    ret = dspLS(0, "comStart...");
    CHECK(ret >= 0, lblKO);
    ret = comStart(chnMdm);
    CHECK(ret >= 0, lblKO);
    ret = comSet(".8N19600");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "comSet OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial...");
    CHECK(ret >= 0, lblKO);
    ptr = "0-00250591642";      //SIMTEL Rwanda
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

    ret = dspLS(2, "wait for ENQ");
    CHECK(ret >= 0, lblKO);
    ret = waitForEnq();
    CHECK(ret > 0, lblKO);
    ret = dspLS(2, "ENQ OK");
    CHECK(ret >= 0, lblKO);

    ret = 00;
    switch (ret) {
      case 0:
          ret = test00(&bReq);
          break;
      case 95:
          ret = test95();
          break;
      case 90:
          ret = test90();
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
    //comHangStart();
    //comHangWait();
    tmrPause(3);
  lblEnd:
    tmrPause(2);
    comStop();
    prtStop();
    dspStop();
}
