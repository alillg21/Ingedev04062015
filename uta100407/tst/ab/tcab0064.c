/** \file
 * Integration test case tcab0064.
 * Functions testing:
 * \sa
 *  - getTrkFromIdle()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0064.c $
 *
 * $Id: tcab0064.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0064(const char *buf) {
    int ret;
    char trk1[128];
    char trk2[128];
    char trk3[128];

    memset(trk1, 0, 128);
    memset(trk2, 0, 128);
    memset(trk3, 0, 128);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = prtS("tcab0064");
    CHECK(ret >= 0, lblKO);

    //ret= getTrkFromIdle(trk1,trk2,trk3,buf); //perform the call
    //CHECK(ret>0,lblKO);
    memcpy(trk1, buf, 128);
    buf += 128;
    memcpy(trk2, buf, 128);
    buf += 128;
    memcpy(trk3, buf, 128);

    ret = prtS("Track1:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(trk1);
    CHECK(ret >= 0, lblKO);

    ret = prtS("Track2:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(trk2);
    CHECK(ret >= 0, lblKO);

    ret = prtS("Track3:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(trk3);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Tracks:");
    CHECK(ret >= 0, lblKO);
    trk1[dspW] = 0;
    trk2[dspW] = 0;
    trk3[dspW] = 0;
    ret = dspLS(1, trk1);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, trk2);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(3, trk3);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
}
