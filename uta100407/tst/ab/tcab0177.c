/** \file
 * Unitary test case tcab0177.
 * Testing gclSck
 * \sa
 *  - utaGclDial()
 *  - utaGclDialInfo()
 *  - utaGclSendBuf()
 *  - utaGclDialErr()
 *  - utaGclStart()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0177.c $
 *
 * $Id: tcab0177.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

static int testStart(byte ctl) {
    const char *ptr;

    ptr = 0;
    switch (ctl) {
      case 0:
      case 3:
          ptr = "127.0.0.1|6789";
          break;
      case 1:
          //ptr= "10.10.59.27|2000";
          ptr = "10.2.4.189|6789";
          break;
      case 2:
          //ptr = "82.247.161.69|6789";
          ptr = "91.121.18.221|6789";
          break;
      case 4:
          ptr = "91.121.18.221|10101";
          break;
      default:
          break;
    }
    return utaGclStart(gclSck, ptr);
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
      case 3:
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
      case 4:
          bufApp(&buf, (byte *) "\x03\x01\x03\x00", 4); //bml header
          bufApp(&buf, (byte *) "\x85", 1); //tag req with attributes
          appAtr(&buf, 0x05, "7T"); //app
          appAtr(&buf, 0x06, "handshake");  //id
          appAtr(&buf, 0x07, "20070604111010"); //dt
          appAtr(&buf, 0x08, "1");  //tid
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
    card len;

    bufInit(&buf, dat, 256);
    ret = -1;
    switch (ctl) {
      case 0:
      case 1:
          ret = utaGclRecvBuf(&buf, (byte *) "!", 20);
          CHECK(ret == 6, lblKO);
          CHECK(memcmp("HELLO!", bufPtr(&buf), 6) == 0, lblKO);
          break;
      case 2:
      case 3:
      case 4:
          bufInit(&buf, dat, 4);
          ret = utaGclRecvBuf(&buf, 0, 200);
          CHECK(ret == 4, lblKO);

          len = 0;
          //len+= dat[0];
          //len*= 0x100;
          //len+= dat[1];
          //len*= 0x100;
          //len+= dat[2];
          //len*= 0x100;
          len += dat[3];
          bufInit(&buf, dat, (word) len);
          ret = utaGclRecvBuf(&buf, 0, 200);
          CHECK(ret == len, lblKO);
          break;
      default:
          break;
    }
    return ret;
  lblKO:
    return -1;
}

void tcab0177(void) {
    int ret;
    byte ctl;
    byte len;
    byte tmp[256];
    const char *ptr;

    //ctl= 0; //local echo SIMELITE
    //ctl= 1; //local echo
    //ctl = 2;                    //DVK
    //ctl= 3; //SIMELITE local echo bml
    ctl = 4;                    //BUS

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "utaGclStart");
    CHECK(ret >= 0, lblKO);
    ptr = 0;
    switch (ctl) {
      case 0:
          ptr = "locSIM";
          break;
      case 1:
          ptr = "echo";
          break;
      case 2:
          ptr = "DVK";
          break;
      case 3:
          ptr = "echo bml";
          break;
      case 4:
          ptr = "BUS";
          break;
      default:
          break;
    }
    VERIFY(ptr);
    dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);
    ret = testStart(ctl);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "utaGclDial...");
    CHECK(ret >= 0, lblKO);
    ret = utaGclDial();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "utaGclDial OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

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
        dspLS(3, ptr);
    } while(ret != gclStaFinished);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "utaGclSend..");
    CHECK(ret >= 0, lblKO);
    ret = makeMsg(ctl, tmp);
    len = (byte) ret;
    ret = utaGclSendBuf(tmp, len);
    ret = dspLS(0, "utaGclSend OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(1, "utaGclRecv..");
    CHECK(ret >= 0, lblKO);
    ret = testRecv(ctl);
    CHECK(ret > 0, lblKO);
    ret = dspLS(1, "utaGclRecv OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = utaGclDialErr();
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
    tmrPause(3);
    utaGclStop();
    prtStop();
    dspStop();
}
