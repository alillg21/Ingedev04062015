/** \file
 * Unitary test case for sys component: smart card ISO 7816 non EMV
 * \sa
 *  - iccStart()
 *  - iccStop()
 *  - iccCommand()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0207.c $
 *
 * $Id: tcab0207.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

static int prtH(const byte * buf, byte len) {
    int ret;
    char hex[512 + 1];
    char *ptr;
    word idx;

    VERIFY(buf);

    memset(hex, 0, 512 + 1);
    bin2hex(hex, buf, len);
    idx = (word) len + len;
    ptr = hex;
    while(idx) {
        ret = prtS(ptr);
        CHECK(ret >= 0, lblKO);
        if(idx < prtW)
            break;
        idx -= prtW;
        ptr += prtW;
    }
    return len;
  lblKO:
    return -1;
}

void tcab0207(void) {
    int ret;
    int len;
    byte rsp[260];
    char *ptr;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Insert a card...");
    CHECK(ret >= 0, lblKO);

    ret = tmrStart(0, 60 * 100);    //60 seconds timeout
    CHECK(ret >= 0, lblKO);

    ret = iccStart(0x10);       //Open associated channel and start waiting for a card 7816
    CHECK(ret >= 0, lblKO);

    len = 0;
    while(tmrGet(0)) {
        ret = iccCommand(0, 0, 0, rsp);
        if(ret == -iccCardRemoved)
            continue;
        CHECK(ret >= 0, lblKO);
        if(!ret)
            continue;
        len = ret;
        break;
    }

    if(!tmrGet(0))
        goto lblEnd;            //card is not inserted
    tmrStop(0);

    ret = prtS("ATR:");
    CHECK(ret >= 0, lblKO);

    ret = prtH(rsp, (byte) len);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "ATR printed");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "Wait...");
    CHECK(ret >= 0, lblKO);

    tmrPause(5);

    ret = iccCommand(0, (byte *) "\x03\x00\xA4\x04\x0C\x06", (byte *) "\x06" "\xFF" "TACHO", rsp);  //APPLICATION SELECT
    if(ret <= 0) {
        switch (-ret) {
          case 0:
          case iccCardRemoved:
              ptr = "Card removed";
              break;
          case iccCardMute:
              ptr = "Card mute";
              break;
          case iccCardPb:
              ptr = "Card Pb";
              break;
          case iccDriverPb:
              ptr = "Dirver Pb";
              break;
          case iccKO:
          default:
              ptr = "Unknown Pb";
              break;
        }
        dspLS(1, ptr);
    } else {
        len = ret;
        ret = prtS("APPLICATION SELECT:");
        CHECK(ret >= 0, lblKO);

        ret = prtH(rsp, (byte) len);
        CHECK(ret >= 0, lblKO);

        ret = dspLS(1, "APP SELECT OK");
        CHECK(ret >= 0, lblKO);
    }

    ret = dspLS(2, "Remove card...");
    CHECK(ret >= 0, lblKO);

    tmrPause(5);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    iccStop(0);                 //stop waiting card and close associated channel
    prtStop();
    dspStop();
}
