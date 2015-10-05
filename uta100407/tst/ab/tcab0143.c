/** \file
 * Unitary test case tcab0143.
 * Input stream buffer
 * Functions testing:
 * \sa
 *  stmInit()
 *  stmClose()
 *  stmGet()
 *  stmPut()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0143.c $
 *
 * $Id: tcab0143.c 2616 2010-04-07 11:16:41Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"
#include "comcst.h"

#ifndef _ING_ARM_SDT
/*
static int fillNvm(void) {
    const char *htm =
#include "yeehaw.inc"
        ;
    int ret;

    ret = nvmSave(2, htm, 0, (word) (strlen(htm)));
    return 1;
}
*/

static int testPut(tStream * stm) {
    const char *htm = 0;

    //byte *htm = "\x01\x02" ;

//#include "yeehaw.inc"

    int ret;
    const byte *ptr;

    VERIFY(stm);
    ptr = (byte *) htm;

    if(ptr) {
        while(*ptr) {
            ret = stmPut(stm, ptr++, 1);
            if(ret != 1)
                return -1;
        }
    } else {
        ret = stmPut(stm, ptr, 1);
        if(ret != 1)
            return -1;
    }
    return 1;
}

static int testGet(tStream * stm) {
    int ret;
    char tmp[256];
    char dTag[256];
    char dAtr[256];
    char dVal[256];
    tBuffer tag, atr, val;

    VERIFY(stm);

    bufInit(&tag, (byte *) dTag, 256);
    bufInit(&atr, (byte *) dAtr, 256);
    bufInit(&val, (byte *) dVal, 256);
    while(1) {
        memset(tmp, 0, 256);
        ret = dspLS(2, "stmGet...");
        CHECK(ret >= 0, lblKO);
        ret = stmGet(stm, (byte *) tmp, 256, (byte *) ">\n");
        if(ret <= 0)
            break;
        if(*tmp == '\n')
            continue;
        ret = prtS(tmp);
        CHECK(ret >= 0, lblKO);
    }
    return 1;
  lblKO:return -1;
}

static int testFile(tStream * stm, tBuffer * buf) {
    int ret;
    char buffer[16];

    VERIFY(buf);

    ret = dspLS(1, "init w...");
    CHECK(ret >= 0, lblKO);
    ret = stmInit(stm, buf, "page0002.nvm", 'w');
    sprintf(buffer, "stmInit:%d", ret);
    prtS(buffer);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "testPut...");
    //CHECK(ret >= 0, lblKO);
    ret = testPut(stm);
    CHECK(ret >= 0, lblKO);

    ret = stmClose(stm);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "init r...");
    CHECK(ret >= 0, lblKO);
    ret = stmInit(stm, buf, "page0002.nvm", 'r');
    //CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "testGet...");
    //CHECK(ret >= 0, lblKO);
    ret = testGet(stm);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static int testRAM(tStream * stm, tBuffer * buf) {
    int ret;
    word idx;

    VERIFY(buf);

    ret = dspLS(1, "init w...");
    CHECK(ret >= 0, lblKO);
    ret = stmInit(stm, buf, 0, 'w');
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "testPut...");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 1024; idx++) {
        char chr;

        chr = (idx % 10) + '0';
        ret = stmPut(stm, (byte *) & chr, 1);
        CHECK(ret == 1, lblKO);
    }

    ret = stmClose(stm);
    CHECK(ret >= 0, lblKO);

    trcFS("buf=%s\n", bufPtr(buf));

    ret = dspLS(2, "init r...");
    CHECK(ret >= 0, lblKO);
    ret = stmInit(stm, buf, 0, 'r');
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "testGet...");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 1024; idx++) {
        char chr;

        ret = stmGet(stm, &chr, 1, 0);
        CHECK(ret == 1, lblKO);
        CHECK(chr == ((idx % 10) + '0'), lblKO);
    }

    ret = stmClose(stm);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

void tcab0143(void) {
    int ret;
    byte dat[4096];
    tBuffer buf;
    tStream stm;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, 4096);

    ret = testFile(&stm, &buf);
    CHECK(ret >= 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    bufReset(&buf);
    ret = testRAM(&stm, &buf);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(3, "KO");
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
}
#else
void tcab0143(void) {
}
#endif
