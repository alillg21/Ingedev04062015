/** \file
 * Unitary test case tcab0053.
 * Functions testing:
 * \sa
 *  - stdLuhnCheck()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0053.c $
 *
 * $Id: tcab0053.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

void tcab0053(void) {
    int ret;
    char *pan;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "stdLuhnCheck");
    CHECK(ret >= 0, lblKO);
    ret = prtS("===== stdLuhnCheck =====");
    CHECK(ret >= 0, lblKO);

    pan = "48951313";
    ret = dspLS(1, pan);
    ret = prtS(pan);
    ret = stdLuhnCheck(pan);
    CHECK(ret, lblKO);
    //tmrPause(1);

    pan = "6280560600004114";
    //pan = "4568941111111119";

    ret = dspLS(2, pan);
    ret = prtS(pan);
    ret = stdLuhnCheck(pan);
    CHECK(ret, lblKO);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
    prtStop();
}
