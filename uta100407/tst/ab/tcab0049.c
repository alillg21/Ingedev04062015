/** \file
 * Unitary test case tcab0049.
 * Functions testing:
 *   addStr
 *   subStr
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0049.c $
 *
 * $Id: tcab0049.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

void tcab0049(void) {
    int ret;
    char buf[dspW + 1];
    char *ptr1;
    char *ptr2;

    trcS("tcab0049 Beg\n");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "addStr");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== addStr ========");
    CHECK(ret >= 0, lblKO);

    ptr1 = "123456789";
    ptr2 = "54321";
    ret = addStr(buf, ptr1, ptr2);
    ret = dspLS(1, ptr1);
    ret = prtS(ptr1);
    ret = dspLS(2, ptr2);
    ret = prtS(ptr2);
    ret = dspLS(3, buf);
    ret = prtS(buf);
    CHECK(strcmp(buf, "123511110") == 0, lblKO);
    tmrPause(1);

    ret = dspLS(0, "addStr");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== addStr ========");
    CHECK(ret >= 0, lblKO);

    ptr1 = "123456789";
    ptr2 = "-54321";
    ret = addStr(buf, ptr1, ptr2);
    ret = dspLS(1, ptr1);
    ret = prtS(ptr1);
    ret = dspLS(2, ptr2);
    ret = prtS(ptr2);
    ret = dspLS(3, buf);
    ret = prtS(buf);
    CHECK(strcmp(buf, "123402468") == 0, lblKO);
    tmrPause(1);

    ret = dspLS(0, "addStr");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== addStr ========");
    CHECK(ret >= 0, lblKO);

    ptr1 = "987654321";
    ptr2 = "54321";
    ret = addStr(buf, ptr1, ptr2);
    ret = dspLS(1, ptr1);
    ret = prtS(ptr1);
    ret = dspLS(2, ptr2);
    ret = prtS(ptr2);
    ret = dspLS(3, buf);
    ret = prtS(buf);
    CHECK(strcmp(buf, "987708642") == 0, lblKO);
    tmrPause(1);

    ret = dspLS(0, "subStr");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== subStr ========");
    CHECK(ret >= 0, lblKO);

    ptr1 = "123456789";
    ptr2 = "54321";
    ret = subStr(buf, ptr1, ptr2);
    ret = dspLS(1, ptr1);
    ret = prtS(ptr1);
    ret = dspLS(2, ptr2);
    ret = prtS(ptr2);
    ret = dspLS(3, buf);
    ret = prtS(buf);
    CHECK(strcmp(buf, "123402468") == 0, lblKO);
    tmrPause(1);

    ret = dspLS(0, "subStr");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== subStr ========");
    CHECK(ret >= 0, lblKO);

    ptr1 = "54321";
    ptr2 = "123456789";
    ret = subStr(buf, ptr1, ptr2);
    ret = dspLS(1, ptr1);
    ret = prtS(ptr1);
    ret = dspLS(2, ptr2);
    ret = prtS(ptr2);
    ret = dspLS(3, buf);
    ret = prtS(buf);
    CHECK(strcmp(buf, "-123402468") == 0, lblKO);
    tmrPause(1);

    ret = dspLS(0, "addStr");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== addStr ========");
    CHECK(ret >= 0, lblKO);

    ptr1 = "0";
    ptr2 = "0";
    ret = addStr(buf, ptr1, ptr2);
    ret = dspLS(1, ptr1);
    ret = prtS(ptr1);
    ret = dspLS(2, ptr2);
    ret = prtS(ptr2);
    ret = dspLS(3, buf);
    ret = prtS(buf);
    CHECK(strcmp(buf, "0") == 0, lblKO);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
    prtStop();
    trcS("tcab0049 End\n");
}
