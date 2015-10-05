/** \file
 * Unitary test case tcab0006.
 * \sa
 * - magStart()
 * - magStop()
 * - magGet()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0006.c $
 *
 * $Id: tcab0006.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0006(void) {
    int ret;
    char trk1[128];
    char trk2[128];
    char trk3[128];
    byte test;

    trcS("tcab0006 Beg\n");

    memset(trk1, 0, 128);
    memset(trk2, 0, 128);
    memset(trk3, 0, 128);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Swipe a card...");
    CHECK(ret >= 0, lblKO);

    ret = tmrStart(0, 10 * 100);    //10 seconds timeout
    CHECK(ret >= 0, lblKO);

    /*! TEST SEQUENCE :
     * \par Step 1: call magStart()
     * This function start the magstripe perypheral.
     * Open the associated channel.
     * Start waiting the magnetic stripe card.
     * Normally this function should be called just before magnetic card acquisition.
     * UTA support all iso magnetic card, it is define by internal call of magReadReq(uint32  handle, uint32  tracksWanted) function
     * Possible value for tracksWanted of this internal function (used value is MAG_ALL_ISO):
     * \htmlonly
     <table border=1 cellspacing=0 cellpadding=0>
     <tr>               <td>Track symbol</td>       <td>Track name</td>                </tr>
     <tr>               <td>MAG_ALL_AVAILABLE</td> <td>All tracks that the current hardware is physically able\n
     to transfer from the card</td>           </tr>
     <tr>               <td>MAG_ISO1</td>             <td>Track ISO1 (Rear Top track)</td>        </tr>
     <tr>               <td>MAG_ISO2</td>           <td>Track ISO2 (Rear Middle Track)</td>       </tr>
     <tr>               <td>MAG_ISO3</td>             <td>Track ISO3 (Rear Bottom Track)</td>     </tr>
     <tr>               <td>MAG_ISOJIS</td>             <td>Track JIS (Front Middle Track)</td>     </tr>
     </table>
     \endhtmlonly
     */
    ret = magStart();           //Open associated channel and start waiting for a card
    CHECK(ret >= 0, lblKO);
    test = 0;
    while(tmrGet(0)) {
        /*!
         * \par Step 2: call magGet(char * trk1, char * trk2, char * trk3)
         * This function get magstripe tracks. First it verify whether a card is swiped;
         * if so, copy its tracks into trk1, trk2, trk3 buffers.
         * If a pointer trk1, trk2, or trk3 is zero, the related track data will be ignored.
         * The size of buffers pointed by trk1, trk2, trk3 should be 128.
         * - trk1  (O) Buffer to capture track1, this size of the buffer should be 128
         * - trk2  (O) buffer to capture track2, this size of the buffer should be 128
         * - trk3  (O) Buffer to capture track3, this size of the buffer should be 128
         * \n
         * Inside of this function it calls magResultGet() which precise tracks and ASCII format for retrieved data
         */
        ret = magGet(trk1, 0, trk3);    //look for a result; if a card is swiped, capture track1 and track3
        if(ret != 0)
            test++;
        ret = magGet(0, trk2, 0);   //look for a result; if a card is swiped, capture track2
        if(ret != 0)
            test++;
        if(test > 0)
            break;
    }

    /*!
     * \par Step 3: call magStop()
     * This function stop the magstripe perypheral.
     * Stop waiting the magnetic stripe card.
     * Close the associated channel.
     * Normally this function should be called just after magnetic card acquisition.
     */
    ret = magStop();            //stop waiting card and close associated channel
    CHECK(ret >= 0, lblKO);

    if((!tmrGet(0)) && (!test))
        goto lblEnd;
    tmrStop(0);

    prtS("Track1:");
    prtS(trk1);

    prtS("Track2:");
    prtS(trk2);

    prtS("Track3:");
    prtS(trk3);

    ret = dspLS(0, "Tracks:");
    CHECK(ret >= 0, lblKO);

    trk1[dspW] = 0;
    ret = dspLS(1, trk1);
    CHECK(ret >= 0, lblKO);

    trk2[dspW] = 0;
    ret = dspLS(2, trk2);
    CHECK(ret >= 0, lblKO);

    trk3[dspW] = 0;
    ret = dspLS(3, trk3);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    magStop();
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();
    trcS("tcab0006 End\n");
}
