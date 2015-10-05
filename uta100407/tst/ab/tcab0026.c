/** \file
 * Unitary test case tcab0026.
 * Functions testing:
 * \sa
 *  - enterStr()
 *  - enterPhr()
 *  - enterPwd()
 *  - enterAmt()
 *  - enterTcp()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0026.c $
 *
 * $Id: tcab0026.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

void tcab0026(void) {
    int ret;
    char buf[20 + 1];

    trcS("tcab0026 Beg\n");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    dspClear();
    ret = dspLS(0, "enterAmt");
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "1");
    ret = enterAmt(1, buf, 2);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(0, "enterStr");
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, 20 + 1);
    ret = enterStr(1, buf, 20 + 1);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    dspClear();
    ret = dspLS(0, "enterPhr8");
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, 20 + 1);
    ret = enterPhr(1, buf, 8 + 1);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    dspClear();
    ret = dspLS(0, "enterPhr20");
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, 20 + 1);
    ret = enterPhr(1, buf, 20 + 1);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    dspClear();
    ret = dspLS(0, "enterPwd");
    CHECK(ret >= 0, lblKO);

    memset(buf, 0, dspW + 1);
    ret = enterPwd(1, buf, dspW + 1);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    dspClear();
    ret = dspLS(0, "enterTcp");
    CHECK(ret >= 0, lblKO);
    memset(buf, 0, dspW + 1);
    ret = enterTcp(1, buf, 12);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    dspStop();                  //close resource
    trcS("tcab0026 End\n");
}
