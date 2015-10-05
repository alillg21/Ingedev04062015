/** \file
 * Unitary test case tcab0057.
 * Functions testing:
 * \sa
 *  - xor()
 *  - stdParity(): set parity bits
 *  - stdDES(): single DES encryption
 *  - stdSED(): single DES decryption
 *  - stdEDE(): triple DES encryption
 *  - stdDED(): triple DES decryption
 *  - stdOWF(): One Way Function
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0057.c $
 *
 * $Id: tcab0057.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

static int testXOR(void) {
    int ret;
    byte arg1[8];
    byte arg2[8];
    byte dst[8];
    char buf[dspW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "xor");
    CHECK(ret >= 0, lblKO);
    ret = prtS("========== Xor =========");
    CHECK(ret >= 0, lblKO);
    memcpy(arg1, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8);
    memcpy(arg2, "\xFE\xDC\xBA\x98\x76\x54\x32\x10", 8);

    xor(arg1, arg2, dst, 8);

    bin2hex(buf, arg1, 8);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, arg2, 8);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, dst, 8);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(dst, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 8) == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int testParity(void) {
    int ret;
    byte dat[8];
    char buf[dspW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "stdParity");
    CHECK(ret >= 0, lblKO);
    memcpy(dat, "\x01\x11\x10\x00\x05\x55\xA0\xAA", 8);

    bin2hex(buf, dat, 8);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);

    stdParity(dat, 8, 8);

    bin2hex(buf, dat, 8);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(dat, "\x01\x10\x10\x01\x04\x54\xA1\xAB", 8) == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int testDES(void) {
    int ret;
    byte src[8];
    byte dst[8];
    byte key[8];
    char buf[dspW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "stdDES");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== stdDES ========");
    CHECK(ret >= 0, lblKO);
    memcpy(src, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8);
    memcpy(key, "\x11\x11\x11\x11\x11\x11\x11\x11", 8);

    stdDES(dst, src, key);

    bin2hex(buf, src, 8);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, key, 8);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, dst, 8);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(dst, "\x8A\x5A\xE1\xF8\x1A\xB8\xF2\xDD", 8) == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int testSED(void) {
    int ret;
    byte src[8];
    byte dst[8];
    byte key[8];
    char buf[dspW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "stdSED");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== stdSED ========");
    CHECK(ret >= 0, lblKO);
    memcpy(src, "\x8A\x5A\xE1\xF8\x1A\xB8\xF2\xDD", 8);
    memcpy(key, "\x11\x11\x11\x11\x11\x11\x11\x11", 8);

    stdSED(dst, src, key);

    bin2hex(buf, src, 8);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, key, 8);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, dst, 8);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(dst, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8) == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int testEDE(void) {
    int ret;
    byte src[8];
    byte dst[8];
    byte key[8 + 8];
    char buf[dspW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "stdEDE");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== stdEDE ========");
    CHECK(ret >= 0, lblKO);
    memcpy(src, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8);
    memcpy(key,
           "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",
           8 + 8);

    stdEDE(dst, src, key);

    bin2hex(buf, src, 8);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, key, 8);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, dst, 8);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(dst, "\x8A\x5A\xE1\xF8\x1A\xB8\xF2\xDD", 8) == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int testDED(void) {
    int ret;
    byte src[8];
    byte dst[8];
    byte key[8 + 8];
    char buf[dspW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "stdDED");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== stdDED ========");
    CHECK(ret >= 0, lblKO);
    memcpy(src, "\x8A\x5A\xE1\xF8\x1A\xB8\xF2\xDD", 8);
    memcpy(key,
           "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11",
           8 + 8);

    stdDED(dst, src, key);

    bin2hex(buf, src, 8);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, key, 8);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, dst, 8);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(dst, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8) == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int testOWF(void) {
    int ret;
    byte src[8];
    byte dst[8];
    byte key[8];
    char buf[dspW + 1];

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "stdOWF");
    CHECK(ret >= 0, lblKO);
    ret = prtS("======== stdOWF ========");
    CHECK(ret >= 0, lblKO);
    memcpy(src, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8);
    memcpy(key, "\x11\x11\x11\x11\x11\x11\x11\x11", 8);

    stdOWF(dst, src, key);

    bin2hex(buf, src, 8);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, key, 8);
    ret = dspLS(2, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    bin2hex(buf, dst, 8);
    ret = dspLS(3, buf);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);
    CHECK(memcmp(dst, "\xDF\x0E\xD4\xDD\x95\xBD\x95\x74", 8) == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

void tcab0057(void) {
    int ret;

    trcS("tcab0057 Beg\n");

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = testParity();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = testXOR();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = testDES();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = testSED();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = testEDE();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = testDED();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = testOWF();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
    prtStop();
    trcS("tcab0057 End\n");
}
