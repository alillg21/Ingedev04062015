/** \file
 * Unitary test case tcab0024.
 * Functions testing:
 * \sa
 *  - fmtSbs()
 *  - fmtTok()
 *  - fmtPad()
 *  - fmtCnt()
 *  - fmtAmt()
 *  - fmtMut()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0024.c $
 *
 * $Id: tcab0024.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

//to do: testing the limits and invalid data

static int testSbs(void) {
    int ret;
    char buf[prtW + 1];
    char *ptr;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtSbs");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== fmtSbs ========");
    CHECK(ret >= 0, lblKO);

    ptr = "0123456789ABCDEF";
    ret = prtS(ptr);
    CHECK(ret >= 0, lblKO);

    ret = fmtSbs(buf, ptr, 1, 8);   //extract substring
    CHECK(ret == 8, lblKO);
    CHECK(strcmp(buf, "12345678") == 0, lblKO);

    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int testTok(void) {
    int ret;
    char buf[prtW + 1];
    char *ptr;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtTok");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== fmtTok ========");
    CHECK(ret >= 0, lblKO);

    ptr = "0|7E1200|PHONE";     //token string
    ret = prtS(ptr);
    CHECK(ret >= 0, lblKO);

    ret = fmtTok(buf, ptr, "|");    //extract first token
    CHECK(ret == 1, lblKO);
    CHECK(strcmp(buf, "0") == 0, lblKO);

    ptr += ret;                 //skip token extracted
    CHECK(*ptr == '|', lblKO);
    ptr++;                      //skip separator

    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtTok(buf, ptr, "|");    //extract next token
    CHECK(ret == 6, lblKO);
    CHECK(strcmp(buf, "7E1200") == 0, lblKO);

    ptr += ret;                 //skip token extracted
    CHECK(*ptr == '|', lblKO);
    ptr++;                      //skip separator

    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtTok(buf, ptr, "|");    //extract last token
    CHECK(ret == 5, lblKO);
    CHECK(strcmp(buf, "PHONE") == 0, lblKO);

    ptr += ret;                 //skip token extracted
    CHECK(*ptr == 0, lblKO);

    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int testPad(void) {
    int ret;
    char buf[prtW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtPad");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== fmtPad ========");
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "12345678");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    ret = fmtPad(buf, 12, '>'); //right pad by '>'
    CHECK(ret == 12, lblKO);
    CHECK(strcmp(buf, "12345678>>>>") == 0, lblKO);

    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtPad(buf, -16, '<');    //left pad by '>'
    CHECK(ret == 16, lblKO);
    CHECK(strcmp(buf, "<<<<12345678>>>>") == 0, lblKO);

    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int testCnt(void) {
    int ret;
    char buf[prtW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtCnt");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== fmtCnt ========");
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "12345678");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    ret = fmtCnt(buf, 12, '.'); //center by '.'
    CHECK(ret == 12, lblKO);
    CHECK(strcmp(buf, "..12345678..") == 0, lblKO);

    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = fmtCnt(buf, 16, '*'); //center by '*'
    CHECK(ret == 16, lblKO);
    CHECK(strcmp(buf, "**..12345678..**") == 0, lblKO);

    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int testAmt(void) {
    int ret;
    char buf[prtW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtAmt");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== fmtAmt ========");
    CHECK(ret >= 0, lblKO);

    ret = prtS("1");
    CHECK(ret >= 0, lblKO);
    ret = fmtAmt(buf, "1", 2, 0);   //format 2-decimal amount
    CHECK(ret == 4, lblKO);
    CHECK(strcmp(buf, "0.01") == 0, lblKO);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = prtS("123");
    CHECK(ret >= 0, lblKO);
    ret = fmtAmt(buf, "123", 3, 0); //format 3-decimal amount
    CHECK(ret == 5, lblKO);
    CHECK(strcmp(buf, "0.123") == 0, lblKO);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = prtS("12345");
    CHECK(ret >= 0, lblKO);
    ret = fmtAmt(buf, "12345", 0, 0);   //format amount without decimals
    CHECK(ret == 5, lblKO);
    CHECK(strcmp(buf, "12345") == 0, lblKO);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int testMut1(void) {
    int ret;
    char buf[prtW + 1];
    const char *fgd = "20040902164700       ";
    const char *bgd = "-: ";
    const char *ctl = "ghAefAcdCCCCCCCCijBkl";

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtMut1");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== fmtMut 1 ========");
    CHECK(ret >= 0, lblKO);

    ret = prtS(fgd);
    CHECK(ret >= 0, lblKO);
    ret = prtS(bgd);
    CHECK(ret >= 0, lblKO);
    ret = prtS(ctl);
    CHECK(ret >= 0, lblKO);
    ret = fmtMut(buf, fgd, bgd, ctl);
    CHECK(ret == (int) strlen(ctl), lblKO);

    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(buf, "02-09-04        16:47") == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int testMut2(void) {
    int ret;
    char buf[prtW + 1];
    const char *fgd = "20040902164700";
    const char *bgd = "-: ";
    const char *ctl = "ghAefAcdCCCCCCCCijBkl";

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtMut2");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== fmtMut 2 ========");
    CHECK(ret >= 0, lblKO);
    ret = prtS(fgd);
    CHECK(ret >= 0, lblKO);
    ret = prtS(bgd);
    CHECK(ret >= 0, lblKO);
    ret = prtS(ctl);
    CHECK(ret >= 0, lblKO);

    ret = fmtMut(buf, fgd, bgd, ctl);
    CHECK(ret == (int) strlen(ctl), lblKO);

    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(buf, "02-09-04        16:47") == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int testMut3(void) {
    int ret;
    char buf[prtW + 1];
    const char *fgd = "12";
    const char *bgd = "";
    const char *ctl = "****";

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtMut3");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== fmtMut 3 ========");
    CHECK(ret >= 0, lblKO);
    ret = prtS(fgd);
    CHECK(ret >= 0, lblKO);
    ret = prtS(bgd);
    CHECK(ret >= 0, lblKO);
    ret = prtS(ctl);
    CHECK(ret >= 0, lblKO);

    ret = fmtMut(buf, fgd, bgd, ctl);
    CHECK(ret == (int) strlen(fgd), lblKO);

    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(buf, "**") == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int testMut4(void) {
    int ret;
    char buf[prtW * 2 + 1];
    const char *fgd = "000013000001                                ";
    const char *bgd = "BATCH XXXXXX TRACE YYYYYY                   ";
    const char *ctl = "ABCDEFabcdefMMMMMMMMMMMMMMMMMMMMNOPQRSghijkl";

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "fmtMut4");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== fmtMut 4 ========");
    CHECK(ret >= 0, lblKO);
    ret = prtS(fgd);
    CHECK(ret >= 0, lblKO);
    ret = prtS(bgd);
    CHECK(ret >= 0, lblKO);
    ret = prtS(ctl);
    CHECK(ret >= 0, lblKO);

    ret = fmtMut(buf, fgd, bgd, ctl);
    CHECK(ret == (int) strlen(fgd), lblKO);

    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(strcmp(buf, "BATCH 000013                    TRACE 000001") == 0,
          lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcab0024(void) {
    int ret;

    trcS("tcab0024 Beg\n");

    RUN(dspStart);
    RUN(prtStart);

    RUN(testSbs);
    RUN(testTok);
    RUN(testPad);
    RUN(testCnt);
    RUN(testAmt);
    RUN(testMut1);
    RUN(testMut2);
    RUN(testMut3);
    RUN(testMut4);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
    prtStop();
    trcS("tcab0024 End\n");
}
