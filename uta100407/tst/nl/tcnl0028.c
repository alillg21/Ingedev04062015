/** \file
 * Unitary test case for pri component: std.
 * \sa
 *  - trk2Lrc()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/nl/tcnl0028.c $
 *
 * $Id: tcnl0028.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"
#include "pri.h"

void tcnl0028(void) {
    int ret;
    byte lrc;
    //const char *trk2= ";5893440000001872=9912101000002?"; //LRC=3
    const char *trk2= ";4761739001010010=10122011143878089?"; //LRC=1: B4761739001010010D10122011143878089F1
    
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "trk2Lrc");
    CHECK(ret >= 0, lblKO);

    //Sunday is zero
    lrc = trk2Lrc(trk2, strlen(trk2));    //Lrc calculation includes STX and ETX     
    CHECK(lrc == '1', lblKO);

    dspLS(1, "lrc IS OK");

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
}
