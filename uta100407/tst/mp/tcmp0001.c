/** \file
 * Unitary test case tcmp0001.
 * Functions testing:
 * \sa
 *  - getTS()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"



static int testGetTS(void) {
    int ret;
    char dateTime[12 + 1];
    char buf[dspW + 1];
    byte idx;

    *dateTime = 0;

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Time stamps A:");
    ret = getTS('A');
    num2dec(buf, ret, 0);
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(0, "Time stamps c:");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 5; idx++) {
        ret = getTS('c');
        CHECK(ret >= 0, lblKO);
        num2dec(buf, ret, 0);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        tmrPause(1);
    }

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(0, "Time stamps C:");
    CHECK(ret >= 0, lblKO);
    for (idx = 0; idx < 5; idx++) {
        ret = getTS('C');
        CHECK(ret >= 0, lblKO);
        num2dec(buf, ret, 0);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        tmrPause(1);
    }

    ret = getDateTime(dateTime);
    CHECK(ret >= 0, lblKO);

    ret = setDateTime("241019000000");
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "c:");
    ret = getTS('c');
    //CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, (card) ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "C:");
    ret = getTS('C');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "s:");
    ret = getTS('s');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "S:");
    ret = getTS('S');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "m:");
    ret = getTS('m');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "M:");
    ret = getTS('M');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "h:");
    ret = getTS('h');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "H:");
    ret = getTS('H');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "d:");
    ret = getTS('d');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "D:");
    ret = getTS('D');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "w:");
    ret = getTS('w');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    strcpy(buf, "W:");
    ret = getTS('W');
    CHECK(ret >= 0, lblKO);
    num2dec(buf + 2, ret, 0);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
    goto lblEnd;
  lblEnd:
    setDateTime(dateTime);
    return ret;
}

void tcmp0001(void) {
    int ret;

    trcS("tcmp0001 Beg\n");
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = testGetTS();
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
    prtStop();
    trcS("tcmp0001 End\n");
}
