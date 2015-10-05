#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcab0062.
//Functions testing for ethernet:
//  comStart
//  comSet
//  comStop
//  comDial
//  comHangStart
//  comHangWait
//  comSendBuf
//  comRecvBuf

static int testDial(byte ctl) {
    const char *ptr;

    ptr = 0;
    switch (ctl) {
      case 0:
          ptr = "127.0.0.1|6789";
          break;
      case 1:
          ptr = "10.10.59.27|2000";
          break;
      case 2:
          ptr = "82.247.161.69|6789";
          break;
      default:
          break;
    }
    VERIFY(ptr);
    return comDial(ptr);
}

static int appAtr(tBuffer * buf, byte atr, const char *val) {
    int ret;

    VERIFY(buf);
    ret = bufApp(buf, &atr, 1);
    ret = bufApp(buf, bPtr(3), 1);
    ret = bufAppStr(buf, val);
    ret = bufApp(buf, bPtr(0), 1);
    return ret;
}

static int makeMsg(byte ctl, byte * msg) {
    int ret;
    card len;
    tBuffer buf;
    byte dat[256];

    bufInit(&buf, dat, 256);
    ret = -1;
    switch (ctl) {
      case 0:
      case 1:
          strcpy((char *) msg, "HELLO!");
          ret = strlen((char *) msg);
          break;
      case 2:
          //x00 x00 x00 3 x01 x01 x04 x00 x85 x0B x03 5 V x00 x05 x03 h a n d s h a k e x00 x1D x03 2 0 0 7 0 6 0 4 1 1 1 0 1 0 x00 x06 x03 x00 x0F x03 0 x00 x14 x03 0 x00 x01 
          bufApp(&buf, (byte *) "\x01\x01\x04\x00", 4); //bml header
          bufApp(&buf, (byte *) "\x85", 1); //tag req with attributes
          appAtr(&buf, '\x0B', "5V");   //app
          appAtr(&buf, '\x05', "handshake");    //id
          appAtr(&buf, '\x1D', "20070604111010");   //dt
          appAtr(&buf, '\x06', "1");    //tid
          appAtr(&buf, 0x0F, "0");  //mid
          appAtr(&buf, 0x14, "0");  //aut
          bufApp(&buf, (byte *) "\x01", 1); //bmlEND

          len = bufLen(&buf);
          num2bin(msg, len, 4);
          memcpy(msg + 4, bufPtr(&buf), bufLen(&buf));
          ret = 4 + bufLen(&buf);
          break;
      default:
          break;
    }
    return ret;
}

static int testRecv(byte ctl) {
    int ret;
    tBuffer buf;
    byte dat[256];

    bufInit(&buf, dat, 256);
    switch (ctl) {
      case 0:
      case 1:
          ret = comRecvBuf(&buf, (byte *) "!", 20);
          CHECK(ret == 6, lblKO);
          CHECK(memcmp("HELLO!", bufPtr(&buf), 6) == 0, lblKO);
          break;
      case 2:
          bufInit(&buf, dat, 4);
          ret = comRecvBuf(&buf, 0, 200);
          CHECK(ret == 4, lblKO);
          break;
      default:
          ret = -1;
          break;
    }
    return ret;
  lblKO:
    return -1;
}

void tcab0062(void) {
    int ret;
    byte ctl;
    byte len;
    byte tmp[256];

    //ctl= 0; //local echo SIMELITE
    //ctl= 1; //local echo
    ctl = 2;                    //DVK

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
    ret = testDial(ctl);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "comDial OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(2, "comSend...");
    ret = makeMsg(ctl, tmp);
    CHECK(ret >= 0, lblKO);
    len = (byte) ret;
    ret = comSendBuf(tmp, len);
    CHECK(ret == len, lblKO);
    ret = dspLS(2, "comSend OK");
    tmrPause(1);

    ret = dspLS(3, "comRecv...");
    ret = testRecv(ctl);
    CHECK(ret > 0, lblKO);
    ret = dspLS(3, "comRecv OK");
    tmrPause(1);

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
    tmrPause(3);
  lblEnd:
    tmrPause(3);
    comStop();
    prtStop();
    dspStop();
}
