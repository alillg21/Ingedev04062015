/** \file
 * Integration test case tcab0065.
 * Functions testing: Testing Emv event processing
 * \sa
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0065.c $
 *
 * $Id: tcab0065.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0065(const byte * buf) {
    int ret;
    char hex[512 + 1];

    trcS("tcab0065 Beg\n");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    memset(hex, 0, 512 + 1);
    bin2hex(hex, buf, 12);

    ret = prtS("FinSel Resp:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(hex);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "FinSel Resp:");
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, hex);
    CHECK(ret >= 0, lblKO);

    trcS("tcab0065 End\n");
    goto lblEnd;

  lblKO:
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
}
