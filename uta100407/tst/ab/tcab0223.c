/** \file
 * Unitary test case tcab0223.
 * ISO7816 non EMV card functions testing:
 * \sa
 *  - enterCard()
 *  - removeCard()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0223.c $
 *
 * $Id: tcab0223.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
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

void tcab0223(void) {
    int ret;
    byte buf[128 * 3];
    char mod;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "enterCard");
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, 128 * 3);
    ret = enterCard(buf, "ik");

    mod = (char) ret;
    switch (mod) {
      case 'i':
          dspLS(1, "chipCard");

          ret = prtS("ATR:");
          CHECK(ret >= 0, lblKO);

          ret = prtH(buf, 12);
          CHECK(ret >= 0, lblKO);

          break;

      case 'k':
          dspLS(1, "key pressed");

          ret = prtS("key:");
          CHECK(ret >= 0, lblKO);

          ret = prtH(buf, 1);
          CHECK(ret >= 0, lblKO);

          break;

      default:
          break;
    }

    tmrPause(3);

    if(mod == 'i') {
        dspLS(2, "removeCard");
        removeCard();
    }

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    prtStop();
    dspStop();                  //close resource
}
