/** \file
 * Unitary test case tcab0014.
 * Functions testing:
 * \sa
 *  - emvContext()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0014.c $
 *
 * $Id: tcab0014.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#include "emv.h"

int tcab0014(int stop, byte * dVAK, byte * dTKA) {
#ifdef __EMV__
#define L_BUF 2048
    int ret;
    tBuffer bVAK;
    tQueue qVAK;
    tBuffer bTKA;
    tQueue qTKA;

    trcS("tcab0014 Beg\n");

    ret = tcab0013(0, dVAK, dTKA);  //Perform emv selection
    CHECK(ret >= 0, lblKO);

    bufInit(&bVAK, dVAK, L_BUF);
    queInit(&qVAK, &bVAK);

    bufInit(&bTKA, dTKA, L_BUF);
    queInit(&qTKA, &bTKA);

    dspClear();
    ret = dspLS(0, "emvContext...");
    CHECK(ret >= 0, lblKO);

    quePutTlv(&qVAK, tagTrnCurCod, 2, (byte *) "\x00\x56"); //Transaction Currency Code
    quePutTlv(&qVAK, tagTrnTyp, 1, (byte *) "\x00");    //Transaction Type
    quePutTlv(&qVAK, tagTrnDat, 3, (byte *) "\x04\x02\x03");    //M.Transaction Date
    quePutTlv(&qVAK, tagTrmAvn, 2, (byte *) "\x00\x8C");    //M.Application Version Number Terminal
    quePutTlv(&qVAK, tagAccCntCod, 2, (byte *) "\x00\x56"); //M.Card Acceptor Country Code
    quePutTlv(&qVAK, tagTrmFlrLim, 4, (byte *) "\x00\x00\x27\x10"); //M.Terminal Floor Limit
    quePutTlv(&qVAK, tagTrmCap, 3, (byte *) "\xE0\xF8\xC8");    //M.Terminal Capabilities
    quePutTlv(&qVAK, tagTrmTyp, 1, (byte *) "\x22");    //M.Terminal Type
    quePutTlv(&qVAK, tagThrVal, 4, (byte *) "\x00\x00\x00\x05");    //M.Threshold value
    quePutTlv(&qVAK, tagTarPer, 1, (byte *) "\x00");    //M.Target percentage
    quePutTlv(&qVAK, tagMaxTarPer, 1, (byte *) "\x00"); //M.Maximum target percentage
    quePutTlv(&qVAK, tagDftValDDOL, 11, (byte *) "\x9F\x37\x04\x9F\x47\x01\x8F\x01\x9F\x32\x01");   //M.Default value for DDOL
    quePutTlv(&qVAK, tagLstRspCod, 0, (byte *) "Y1Z1Y2Z2Y3Z300510104"); //M.List of possible response codes
    quePutTlv(&qVAK, tagAddTrmCap, 5, (byte *) "\xF1\x00\xF0\xA0\x01"); //O.Additional Terminal Capabilities
    quePutTlv(&qVAK, tagTrnTim, 3, (byte *) "\x09\x29\x12");    //O.Transaction Time
    quePutTlv(&qVAK, tagTrnCurExp, 1, (byte *) "\x02"); //O.Transaction Currency Exponent
    quePutTlv(&qVAK, tagTrnSeqCnt, 4, (byte *) "\x00\x00\x00\x02"); //O.Transaction Sequence Counter
    quePutTlv(&qVAK, tagMrcCatCod, 2, (byte *) "\x53\x11"); //O.Merchant Category Code
    quePutTlv(&qVAK, tagTrmId, 0, (byte *) "12345678"); //O.Terminal Identification
    quePutTlv(&qVAK, tagDftValTDOL, 3, (byte *) "\x9F\x08\x02");    //Default value for TDOL
    quePutTlv(&qVAK, tagAcqId, 5, (byte *) "\x00\x00\x47\x61\x73"); //O.Acquirer Identifier (BIN)
    quePutTlv(&qVAK, tagTrnRefCur, 2, (byte *) "\x00\x56"); //O.Transaction Reference Currency
    quePutTlv(&qVAK, tagTrnRefExp, 1, (byte *) "\x02"); //O.Transaction Reference Currency Exponent
    quePutTlv(&qVAK, tagTACDen, 5, (byte *) "\x00\x00\x00\x00\x00");    //Terminal Action Code Denial
    quePutTlv(&qVAK, tagTACOnl, 5, (byte *) "\x00\x00\x00\x00\x00");    //Terminal Action Code Online
    quePutTlv(&qVAK, tagTACDft, 5, (byte *) "\x00\x00\x00\x00\x00");    //Terminal Action Code Default
    ret = emvContext(&qVAK, &qTKA);
    CHECK(ret >= 0, lblKO);

    dspLS(3, "OK");
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
    trcS("tcab0014 End\n");
    return ret;
#else
    return -1;
#endif
}
