/** \file
 * Integration test case tcab0063.
 * Functions testing:
 * \sa
 *  - getTrkFromIdle()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0063.c $
 *
 * $Id: tcab0063.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

int tcab0063(const char *buf) {
    int ret;
    char trk2[128];
    tInfo inf;

    memset(trk2, 0, 128);

    //No user interface in this processing
    //ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = prtS("tcab0063");
    CHECK(ret >= 0, lblKO);

    //ret= getTrkFromIdle(0,trk2,0,buf); //perform the call
    //CHECK(ret>0,lblKO);
    memcpy(trk2, buf + 128, 128);

    ret = prtS("Track2:");
    CHECK(ret >= 0, lblKO);
    ret = prtS(trk2);
    CHECK(ret >= 0, lblKO);
    //ret= dspLS(0,"Track2:"); CHECK(ret>=0,lblKO);

    trk2[dspW] = 0;

    inf.cmd = 's';
    strcpy(inf.msg[0], trk2);
    strcpy(inf.msg[1], "Accept?");
    strcpy(inf.msg[2], "");
    strcpy(inf.msg[3], "");
    inf.dly = 5;
    //ret= info(&inf);
    //if(ret==kbdVAL) ret= 1; else ret= 0;

    if(*trk2 == '4')
        ret = 1;
    else
        ret = 0;                //accept only VISA cards

    goto lblEnd;

  lblKO:
    //dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
    prtStop();
    //dspStop();
    return ret;
}
