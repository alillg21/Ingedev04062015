/** \file tcnl0029.c
 * Unitary test case for sys component: EMV select processing testing.
 * \sa
 *  - emvStart()
 *  - emvStop()
 *  - emvSelect()
 *  - emvSelectStatus()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"



int tcnl0029(int stop, byte * dat, byte * tmp) {
    int ret;
    tQueue que;
    tBuffer buf;
    word isCardSta;

    trcS("tcnl0029 Beg\n");

    VERIFY(dat);
    VERIFY(tmp);

    /*! TEST SEQUENCE :
     * \par Step 1: Call tcab0011 for EMV initialisation (set AIDs)
     * \sa  tcab0011.c
     */
    ret = tcab0011(0, dat);     //To be sure that emvInit is called
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Insert a card...");
    CHECK(ret >= 0, lblKO);

    ret = tmrStart(0, 60 * 100);    //60 seconds timeout
    CHECK(ret >= 0, lblKO);

    ret = iccStart(0);          //Open associated channel and start waiting for a card
    CHECK(ret >= 0, lblKO);

    while(tmrGet(0)) {
        ret = iccCommand(0, (byte *) 0, (byte *) 0, tmp);   //look for a result; if a card is insertes, capture ATR
        if(ret == -iccCardRemoved)
            continue;
        CHECK(ret >= 0, lblKO);
        if(!ret)
            continue;
        break;
    }

    iccStop(0);

    if(!tmrGet(0))
        goto lblEnd;            //card is not inserted
    tmrStop(0);

    /*!
     * \par Step 2: call emvStart()
     * This function is a wrapper around the function amgInitPayment.
     * This function should be called before starting EMV processing.
     * this wrapper dans l'information about the means of payment set following parameters :\n
     * - msgType = MSG_SMART_CARD
     * - msgLength = 0
     * - receiverPid = 0xFFFF (not used)
     * - callerTaskId = 0xFFFF (not used)
     */
    ret = emvStart();
    CHECK(ret >= 0, lblKO);

    bufInit(&buf, dat, L_BUF);
    queInit(&que, &buf);

    dspLS(0, "emvSelect...");
    /*!
     * \par Step 3: call emvSelect(tQueue * que)
     * This function is a wrapper around the function amgAskSelection.
     * Wrapper around the function amgAskSelection.
     * Creates a list of AIDs from the card that are accepted by the Kernel (see emvInit function).
     * Returns a queue of (AID, application name,priority indicator) triples sorted according to the priority.
     * This queue should be used by the application to select the first element of the queue,
     * or to organise a menu of AIDs, ask for the confirmation or just perform a final select call,
     * depending on the application capabilities. According the EMV specification,
     * the confirmation should be asked if the highest bit of priority indicator is set.
     * For an idle application manipulating with several EMV applications this wrapper is not suitable:
     * the association between AIDs and PIDs is removed.
     */
    ret = emvSelect(&que);      //ask EMV Kernel to prepare a candidate list
    /*!
     * \par Step 4: call emvSelectStatus(word * pbm)
     * Wrapper around the function amgEmvSelectStatus.
     *  This function will return the status of emvapplication selection
     *  after emvSelect().This function can be called if emvSelect is returning 
     *  the value less than or equal 0      
     */
    CHECK(ret >= -1, lblKO);
    if(ret <= 0) {
        emvSelectStatus(&isCardSta);    // to verify status of card and aapplication    
        if(isCardSta == 0x6283) {
            dspLS(2, "Application Block");
        } else if((isCardSta == 0x6A81)) {
            dspLS(2, "Card Block");
        }

    } else if(ret > 0) {
        dspLS(2, "Application selected");
    }

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(1, "KO!");
    ret = -1;
  lblEnd:
    //  if(stop) {
    /*!
     * \par Step 4: call emvStop()
     * This function is a wrapper around the functions amgEndPayment and amgEmvCloseContext.
     * This function should be called after finishing EMV processing and in case of exceptions
     */
    tmrPause(3);
    emvStop();
    prtStop();
    dspStop();
    // }
    trcS("tcnl0029 End\n");
    return ret;
}
