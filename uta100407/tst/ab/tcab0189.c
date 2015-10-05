/** \file tcab0189.c
 * Unitary test case for pri/stm component: LZW compression
 * http://marknelson.us/attachments/lzw-data-compression/lzw.c
 * It is the next step after tcab0187.
 * The unicapt32 calls are replaced by UTA sys functions.
 * malloc is removed
 * Other modifications are done to simplify the code and be closer to UTA style.
 * To run this test case a file tcab0188.src should be in DFS of the application
 * It will be compressed into tcab0188.lzw
 * Then it will be decompressed to tcab0188.dst
 * If the file tcab0188.src is the same as tcab0187.src
 * then the file tcab0188.lzw should be the same as tcab0187.lzw
 * and tcab0188.dst should be the same as tcab0187.dst
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0189.c $
 *
 * $Id: tcab0189.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include <pri.h>

static card prtTS(const char *msg, card beg) {
    char tmp[prtW + 1];
    char buf[prtW + 1];
    card ts;

    strcpy(buf, msg);
    ts = getTS(1);
    ts -= beg;
    num2dec(tmp, ts, 0);
    strcat(buf, tmp);
    prtS(buf);
    return ts + beg;
}

void tcab0189(void) {
    int ret;
    byte sDat[512];
    byte dDat[512];
    tBuffer sBuf;
    tBuffer dBuf;
    tStream src;
    tStream dst;

    bufInit(&sBuf, sDat, 512);
    bufInit(&dBuf, dDat, 512);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "compressing...");
    CHECK(ret >= 0, lblKO);
    prtTS("compress beg ", 0);
    ret = stmInit(&src, &sBuf, "tcab0189.src", 'r');
    CHECK(ret >= 0, lblKO);
    ret = stmInit(&dst, &dBuf, "tcab0189.lzw", 'w');
    CHECK(ret >= 0, lblKO);
    ret = lzwCompress(&src, &dst);  //the compression of 588434 bytes to 222553 bytes takes 99.27 secs in I8550
    CHECK(ret >= 0, lblKO);
    stmClose(&dst);
    stmClose(&src);
    prtTS("compress end ", 0);
    ret = dspLS(1, "OK");
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "expanding...");
    CHECK(ret >= 0, lblKO);
    prtTS("expand beg ", 0);
    ret = stmInit(&src, &sBuf, "tcab0189.lzw", 'r');
    CHECK(ret >= 0, lblKO);
    ret = stmInit(&dst, &dBuf, "tcab0189.dst", 'w');
    CHECK(ret >= 0, lblKO);
    ret = lzwExpand(&src, &dst);    //the backward expansion takes 149.94 secs in I8550
    CHECK(ret >= 0, lblKO);
    stmClose(&dst);
    stmClose(&src);
    prtTS("expand end ", 0);
    ret = dspLS(3, "OK");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
    prtStop();
}
