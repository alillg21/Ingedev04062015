#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcik0176.c
//Functions testing for SSL of CMM module:

static int testDial(byte ctl) {
    const char *ptr;

    ptr = 0;
    switch (ctl) {
      case 0:
          ptr = "10.17.32.127|0666";
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
          bufReset(&buf);
          ret = comRecvBuf(&buf, (byte *) "\x00", 50);
          break;
      default:
          ret = -1;
          break;
    }
    return ret;
//  lblKO:
//    return -1;
}

void tcik0176(void) {
    int ret;
    byte ctl;
    byte len;
    byte tmp[256];

    ctl = 0;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

#define __CHN_SSL__

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

#undef __CHN_SSL__

    comStop();
    prtStop();
    dspStop();
}
