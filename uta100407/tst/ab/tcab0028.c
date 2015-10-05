/** \file
 * Unitary test case tcab0028.
 * Functions testing:
 * \sa
 *  - info()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0028.c $
 *
 * $Id: tcab0028.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

void tcab0028(void) {
    int ret;
    tInfo inf;

    trcS("tcab0028 Beg\n");
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    //show information screen for 5 seconds or until a key is pressed
    inf.cmd = 's';
    strcpy(inf.msg[0], "SHOW 0");
    strcpy(inf.msg[1], "SHOW 1");
    strcpy(inf.msg[2], "SHOW 2");
    strcpy(inf.msg[3], "SHOW 3");
    inf.dly = 5;
    ret = info(&inf);
    CHECK(ret >= 0, lblKO);

    //show information screen with beeping for 5 seconds or until a key is pressed
    inf.cmd = 'b';
    strcpy(inf.msg[0], "");     //the line from previous screen holds
    strcpy(inf.msg[1], "BEEP 1");
    strcpy(inf.msg[2], " ");    //the line is cleared
    strcpy(inf.msg[3], "BEEP 3");
    inf.dly = 5;
    ret = info(&inf);
    CHECK(ret >= 0, lblKO);

    //the information screen below should not be shown: the command is to ignore it
    inf.cmd = 'i';
    strcpy(inf.msg[0], "IGNORE 0");
    strcpy(inf.msg[1], "IGNORE 1");
    strcpy(inf.msg[2], "IGNORE 2");
    strcpy(inf.msg[3], "IGNORE 3");
    inf.dly = 5;
    ret = info(&inf);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    dspStop();                  //close resource
    trcS("tcab0028 End\n");
}
