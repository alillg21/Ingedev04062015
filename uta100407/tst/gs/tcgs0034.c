/** \file
 * Unitary test case for tcgs0034
 * \sa
 *  - hex2bin()
 */

#include <string.h>
#include <stdio.h>
#include "sys.h"

#define CHK CHECK(ret>=0,lblKO)

void tcgs0034(void) {
    int ret;
    char ascii[1024 + 1];
    byte bin[512];

    trcS("tcgs0034 Beg\n");

    dspStart();
    dspClear();

    memset(ascii, 'A', sizeof(ascii) - 1);
    ascii[1024] = 0;
    ret = hex2bin(bin, ascii, 0);
    CHECK(ret == 512, lblKO);
    dspLS(0, "Test is OK.");
    tmrPause(2);
    goto lblEnd;

  lblKO:
    Beep();
    dspLS(2, "KO!");
    tmrPause(3);
    goto lblEnd;
  lblEnd:
    dspClear();
    dspStop();
    trcS("tcgs0034 End\n");
}
