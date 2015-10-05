/** \file
 * Unitary test case tcab0018.
 * Functions testing:
 * \sa
 *  - emvValidate()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0018.c $
 *
 * $Id: tcab0018.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __EMV__
#include "emv.h"

int tcab0018(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
#define L_BUF 2048
    int ret;
    tBuffer bVAK;
    tQueue qVAK;
    tBuffer bTKA;
    tQueue qTKA;
    tBuffer bVKA;
    tQueue qVKA;
    tBuffer bTAK;
    tQueue qTAK;

    trcS("tcab0018 Beg\n");

    ret = tcab0017(0, dVAK, dTKA, dVKA, dTAK);  //Go through previous stages of EMV transaction
    CHECK(ret >= 0, lblKO);

    bufInit(&bVAK, dVAK, L_BUF);
    queInit(&qVAK, &bVAK);

    bufInit(&bTKA, dTKA, L_BUF);
    queInit(&qTKA, &bTKA);

    bufInit(&bVKA, dVKA, L_BUF);
    queInit(&qVKA, &bVKA);

    bufInit(&bTAK, dTAK, L_BUF);
    queInit(&qTAK, &bTAK);

    dspClear();
    dspLS(0, "emvValidate...");

    quePutTag(&qTAK, tagTVR);   //Terminal Verification Result
    quePutTag(&qTAK, tagTSI);   //Transaction Status Information
    quePutTlv(&qVAK, tagAmtBin, 4, (byte *) "\x00\x00\x00\x7B");    //Amount, Authorised (Binary), =123
    quePutTlv(&qVAK, tagAmtNum, 6, (byte *) "\x00\x00\x00\x00\x01\x23");    //Amount, Authorised (Numeric)
    quePutTlv(&qVAK, tagAmtOthBin, 4, (byte *) "\x00\x00\x00\x00"); //Amount, Other (Binary)
    quePutTlv(&qVAK, tagAmtPrv, 4, (byte *) "\x00\x00\x00\x00");    //Amount found in the transaction file, in a previous transaction performed with the same card

    quePutTlv(&qVAK, tagAmtPrv, 4, (byte *) "\x00\x00\x00\x00");    //Amount found in the transaction file, in a previous transaction performed with the same card  
    quePutTlv(&qVAK, tagFrcOnl, 1, (byte *) "\x00");    //Flag transaction forced online  
    quePutTlv(&qVAK, tagHotLst, 1, (byte *) "\x00");    //Flag set to TRUE if the card is in BlackList

    ret = emvValidate(&qVAK, &qTAK, &qVKA, &qTKA);
    CHECK(ret > 0, lblKO);

    dspLS(2, "OK");
    tmrPause(1);

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(1, "KO!");
    Beep();
    tmrPause(3);
    ret = -1;
  lblEnd:
    if(stop) {
        emvStop();
        prtStop();
        dspStop();
    }
    trcS("tcab0018 End\n");
    return ret;
}
#else
int tcab0018(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
    return -1;
}
#endif
