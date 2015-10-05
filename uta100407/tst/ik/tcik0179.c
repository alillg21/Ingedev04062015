/** \file
 * Integration test case tcik0179
 * SSL connection with chn
 * Unitary test.
 * \sa
 *  - comStart()
 *  - comSet()
 *  - comRecvBuf()
 *  - comDial()
 *  - comSendBuf()
 */

//#include <string.h>
#include "sys.h"
#include "tst.h"

static const byte AuthMsg[] = { 0x00, 0x5A,
    0x60, 0x00, 0x01, 0x00, 0x00,   // TPDU
    0x02, 0x00,                 // Message Type
    0x30, 0x20, 0x05, 0x80, 0x20, 0xC0, 0x00, 0x04, // Bitmap
    0x00, 0x00, 0x00,           // Procs Code
    0x00, 0x00, 0x00, 0x12, 0x39, 0x12, // Amount
    0x00, 0x00, 0x02, 0x00, 0x22, 0x00, 0x06, 0x00, 0x37, 0x54,
    0x37, 0x71, 0x36, 0x00, 0x00, 0x00, 0x20, 0xD0, 0x30, 0x41,
    0x01, 0x00, 0x00, 0x05, 0x49, 0x00, 0x00, 0x1F, 0x38, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x32, 0x30, 0x30, 0x31,
    0x34, 0x39, 0x39, 0x30, 0x36, 0x30, 0x33, 0x34, 0x20, 0x20,
    0x20, 0x00, 0x12, 0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x38,
    0x31, 0x39, 0x39, 0x39, 0x39, 0x00
};

static int testDial(byte ctl) {
    const char *ptr;

    ptr = 0;
    switch (ctl) {
      case 0:
          //ptr = "10.17.32.127|0666";
          //ptr = "217.109.89.164|443";
          ptr = "10.10.203.231|443";
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

void tcik0179(void) {
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
    ret = comStart(chnSsl);
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
//    ret = makeMsg(ctl, tmp);
//    CHECK(ret >= 0, lblKO);
//    len = (byte) ret;
    len = sizeof(AuthMsg);
//    ret = comSendBuf(tmp, len);
    ret = comSendBuf(AuthMsg, len);
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
