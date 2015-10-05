/** \file
 *  Unitary test case for sys component: emvInit function testing.
 * \sa
 *  - emvInit()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0011.c $
 *
 * $Id: tcab0011.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

int tcab0011(int stop, byte * dat) {
#ifdef __EMV__
#define L_BUF 2048
    int ret;
    tQueue que;
    tBuffer buf;
    byte aid[1 + 16];
    static byte num = 0;

    trcS("tcab0011 Beg\n");
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    bufInit(&buf, dat, L_BUF);
    queInit(&que, &buf);

    /*! TEST SEQUENCE :
     * \par Step 1: Set a queue that contains all the EMV AIDs that are accepted by the terminal application
     * "\x07\xA0\x00\x00\x00\x03\x10\x10" \n
     * - first byte is AID length (AID - Application Identifier)
     * - next bytes are AID name
     * \n
     * The AID A0 00 00 00 03 10 10 means VSDC
     */
    memcpy(aid, "\x07\xA0\x00\x00\x00\x03\x10\x10", 1 + 7);

    ret = quePut(&que, aid + 1, *aid);
    CHECK(ret == *aid, lblKO);

    dspLS(0, "emvInit...");
    if(num++) {
        dspLS(1, "Already done");
        ret = 1;
        goto lblEnd;
    }
    /*!
     * \par Step 2: call emvInit (tQueue * que)
     * This function is a wrapper around the function amgSelectionCriteriaAdd.
     * Using this function an application informs EMV Kernel that it can accept a number of EMV AIDs.
     * Normally this function should be used only at the POWERON event processing.
     * Following steps are performed inside this function:\n
     *  - getPid()
     *  - getAppName()
     *  - AID length and name are retrieved from que
     *  - priority index and indicator are set to 1 in this wrapper
     *  - amgSelectionCriteriaAdd() from unicapt32
     */
    ret = emvInit(&que);
    CHECK(ret == 1, lblKO);
    dspLS(3, "OK");
    tmrPause(1);

    ret = 1;
    trcS("tcab0011 End\n");
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(3, "KO!");
    Beep();
    tmrPause(3);
    ret = -1;
  lblEnd:
    tmrPause(1);
    if(stop) {
        dspStop();
        prtStop();
    }
    return ret;
    /*!
     * \par Step 5: Analyse of traces
     * Trace file content:
     * \n
     * \include tcab0011.trc
     * \n
     */
#else
    return -1;
#endif
}
