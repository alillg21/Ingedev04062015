/** \file
 * Unitary test case tcgs0033.
 * Functions testing:
 * \sa
 *  - cryEraseKeysArray()
 */

#include <string.h>
#include <stdio.h>
#include "sys.h"

#define CHK CHECK(ret>=0,lblKO)

void tcgs0033(void) {
    int ret;
    byte loc;
    byte crt[4];

    trcS("tcgs0033 Beg\n");

    ret = cryStart('M');
    CHK;
    ret = dspStart();
    CHK;
    ret = dspClear();
    CHK;

    ret = dspLS(0, "Load test key");
    CHK;
    ret = dspLS(1, "Array:2 Slot: 0");
    CHK;
    tmrPause(1);
#ifdef __UNICAPT__
    loc = 0x20;
#endif
#ifdef __TELIUM__
    loc = 1008 / 8;
#endif
    ret = cryLoadTestKey(loc);
    CHK;
    ret = cryVerify(loc, crt);
    CHK;
    ret = dspLS(2, "OK");
    CHK;
    tmrPause(1);

    ret = dspClear();
    CHK;
    ret = dspLS(0, "Load test key");
    CHK;
    ret = dspLS(1, "Array:2 Slot: 2");
    CHK;
    tmrPause(1);
#ifdef __UNICAPT__
    loc = 0x22;
#endif
#ifdef __TELIUM__
    loc = (1008 - 16) / 8;
#endif
    ret = cryLoadTestKey(loc);
    CHK;
    ret = cryVerify(loc, crt);
    CHK;
    ret = dspLS(2, "OK");
    CHK;
    tmrPause(1);

    ret = dspClear();
    CHK;
    ret = dspLS(0, "Load test key");
    CHK;
    ret = dspLS(1, "Array:3 Slot: 4");
    CHK;
    tmrPause(1);
#ifdef __UNICAPT__
    loc = 0x34;
#endif
#ifdef __TELIUM__
    loc = 800 / 8;
#endif
    ret = cryLoadTestKey(loc);
    CHK;
    ret = cryVerify(loc, crt);
    CHK;
    ret = dspLS(2, "OK");
    CHK;
    tmrPause(1);

    ret = dspClear();
    CHK;
    ret = dspLS(0, "Erase test keys");
    CHK;
    ret = dspLS(1, "Array:2");
    CHK;
    tmrPause(1);
#ifdef __UNICAPT__
    loc = 0x20;                 //only the arrray is important, so it would be 0x22 too
#endif
#ifdef __TELIUM__
    loc = 1008 / 8;
#endif
    ret = cryEraseKeysArray(loc / 16);
    CHK;
    ret = cryVerify(loc, crt);
    CHECK(ret == -1, lblKO);    // it is deleted
#ifdef __UNICAPT__
    loc = 0x22;
#endif
#ifdef __TELIUM__
    loc = (1008 - 16) / 8;
#endif
    ret = cryVerify(loc, crt);
    CHECK(ret == -1, lblKO);    // it is deleted
#ifdef __UNICAPT__
    loc = 0x34;
#endif
#ifdef __TELIUM__
    loc = 800 / 8;
#endif
    ret = cryVerify(loc, crt);
#ifdef __UNICAPT__
    CHK;                        //it is not deleted
#endif
#ifdef __TELIUM__
    CHECK(ret == -1, lblKO);    // it is deleted also
#endif

    ret = dspLS(2, "OK");
    CHK;
    tmrPause(1);

    goto lblEnd;

  lblKO:
    Beep();
    dspLS(2, "KO!");
    tmrPause(3);
    goto lblEnd;
  lblEnd:
    dspClear();
    dspStop();
    cryStop('M');
    trcS("tcgs0033 End\n");
}
