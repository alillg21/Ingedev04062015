/** \file
 * Unitary test case for sys component: timer processing.
 * \sa
 *  - tmrStart()
 *  - tmrStop()
 *  - tmrGet()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0001.c $
 *
 * $Id: tcab0001.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0001(void) {
    int ret;                    ///< integer value for return code
    byte n;
    char buf[prtW + 1];         //*< printing buffer */

    trcS("tcab0001 Beg\n");

    memset(buf, 0, prtW + 1);   //reset printing buffer

    ret = prtStart();           //open printing channel
    CHECK(ret >= 0, lblKO);

    ret = prtS("Waiting for 10 seconds...");
    CHECK(ret >= 0, lblKO);

    /*! TEST SEQUENCE :
     * \par Step 1: call tmrStart(byte tmr, int dly)
     * This function create unique timer identified by number tmr = 0.
     * Inside of this function psyTimeGet() returns the number of 10 milliseconds between 1/1/1970 and the current system time.
     * Value of dly is added to extracted value of milliseconds in goal to calculate the ending timestamp of this timer.
     */

    n = 10;
    ret = tmrStart(0, n * 100); //start timer 0 to wait for n seconds
    CHECK(ret > 0, lblKO);

    /*!
     * \par Step 2: call tmrGet(byte tmr)
     *  tmr is a identification number of timer
     * this function returns
     * - number of seconds of delay left, if the ending timestamp is not achieved
     * - zero, if the ending timestamp is bypassed
     */

    while(n) {
        ret = tmrGet(0);        //get the number of centiseconds left
        CHECK(ret >= 0, lblKO);
        if(ret / 100 > n)
            continue;
        strcat(buf, ".");
        prtS(buf);
        n--;
    }

    ret = prtS("Finished.");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:
    /*!
     * \par Step 3: call tmrStop(byte tmr)
     * this function reset internal value of ending timestamp of timer.
     *  ( tmr is a idetification number of timer)
     */
    tmrStop(0);                 //stop timer
    prtStop();                  //close printing channel
    trcS("tcab0001 End\n");
}
