// tcgs0021
//
// author:      Gábor Sárospataki
// date:        01.02.2006
// description: test case to bufDel combo

#include "pri.h"
#include <string.h>
#define CHK CHECK(ret>=0,lblKO)

void tcgs0021(void) {
    int ret;
    byte dat[100];
    tBuffer buf;

    bufInit(&buf, dat, sizeof(dat));
    //delete a little part of the buffer
    ret = bufAppStr(&buf, "01234567890123456789");
    CHK;
    ret = bufDel(&buf, 3, 7);
    CHK;
    CHECK(bufLen(&buf) == 20 - 7, lblKO);
    CHECK(memcmp(bufPtr(&buf), "0120123456789", bufLen(&buf)) == 0, lblKO);
    //delete a biger part of the buffer
    bufReset(&buf);
    ret = bufAppStr(&buf, "01234567890123456789");
    CHK;
    ret = bufDel(&buf, 3, 15);
    CHK;
    CHECK(bufLen(&buf) == 20 - 15, lblKO);
    CHECK(memcmp(bufPtr(&buf), "01289", bufLen(&buf)) == 0, lblKO);
    //if the len>bufLen then erase until the end of the buffer
    bufReset(&buf);
    ret = bufAppStr(&buf, "01234567890123456789");
    CHK;
    ret = bufDel(&buf, 3, 700);
    CHK;
    CHECK(bufLen(&buf) == 3, lblKO);
    CHECK(memcmp(bufPtr(&buf), "012", bufLen(&buf)) == 0, lblKO);
    //if the len == 0 then erase until the end of the buffer
    bufReset(&buf);
    ret = bufAppStr(&buf, "01234567890123456789");
    CHK;
    ret = bufDel(&buf, 3, 0);
    CHK;
    CHECK(bufLen(&buf) == 3, lblKO);
    CHECK(memcmp(bufPtr(&buf), "012", bufLen(&buf)) == 0, lblKO);
    goto lblEnd;

  lblKO:
    dspLS(0, "KO!");
    tmrPause(2);
  lblEnd:
    dspStop();
}
