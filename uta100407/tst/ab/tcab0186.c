/** \file
 * Unitary test case for sys component)(tcab0186.c): Sequential DFS access
 * \sa
 *  - nvmSeqOpen()
 *  - nvmSeqClose()
 *  - nvmSeqGet()
 *  - nvmSeqPut()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0186.c $
 *
 * $Id: tcab0186.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0186(void) {
    int ret;
    word idx;
    char buf[256];
    word len;
    card hdl = 0;
    const char *str = "abcdefghijklmnopqrstuvwxyz";

    trcS("tcab0186 Beg\n");
    nvmStart();                 //initialise non-volatile memory

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "nvmSeqOpen w...");
    CHECK(ret >= 0, lblKO);
    hdl = nvmSeqOpen("tcab0186.tst", 'w');
    CHECK(hdl, lblKO);

    ret = dspLS(1, "nvmSeqPut...");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 1000; idx++) {
        num2dec(buf, idx, 4);
        ret = dspLS(2, buf);
        CHECK(ret >= 0, lblKO);
        ret = nvmSeqPut(hdl, str, strlen(str));
        CHECK(ret >= 0, lblKO);
    }
    ret = dspLS(1, "nvmSeqClose...");
    CHECK(ret >= 0, lblKO);
    nvmSeqClose(hdl);
    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "nvmSeqOpen r...");
    CHECK(ret >= 0, lblKO);
    hdl = nvmSeqOpen("tcab0186.tst", 'r');
    CHECK(hdl, lblKO);

    ret = dspLS(1, "nvmSeqGet...");
    CHECK(ret >= 0, lblKO);
    len = strlen(str) * 1000;
    while(len) {
        num2dec(buf, len, 5);
        ret = dspLS(2, buf);
        CHECK(ret >= 0, lblKO);
        memset(buf, 0, 256);
        ret = nvmSeqGet(hdl, buf, strlen(str));
        CHECK(ret >= 0, lblKO);
        CHECK(strcmp(buf, str) == 0, lblKO);
        len -= strlen(str);
    }
    ret = dspLS(1, "nvmSeqClose...");
    CHECK(ret >= 0, lblKO);
    nvmSeqClose(hdl);
    tmrPause(1);
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    if(hdl)
        nvmSeqClose(hdl);
    dspStop();
    trcS("tcab0186 End\n");
}
