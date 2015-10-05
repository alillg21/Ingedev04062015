/** \file
 * Unitary test case tcab0020.
 * Functions testing:
 * \sa
 *  - emvComplete()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0020.c $
 *
 * $Id: tcab0020.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __EMV__
#include "emv.h"

int tcab0020(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
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
    byte buf[256];
    char *ptr;
    card tag;
    byte cry;
    word len;

    trcS("tcab0020 Beg\n");

    ret = tcab0019(0, dVAK, dTKA, dVKA, dTAK);  //Go through previous stages of EMV transaction
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
    dspLS(0, "emvComplete...");

    quePutTag(&qTAK, tagTVR);   //Terminal Verification Result
    quePutTag(&qTAK, tagTSI);   //Transaction Status Information
    quePutTag(&qTAK, tagCVMRes);    //Cardholder Verification Method (CVM) Results
    quePutTag(&qTAK, tagCID);   //Cryptogram Information Data (CID)
    quePutTag(&qTAK, tagIssTrnCrt); //Issuer Transaction Certificate

    quePutTag(&qTAK, tagTrmDc2);    //Terminal decision after Generate AC 0=Declined, 1= Approved
    quePutTag(&qTAK, tagScrRes);    //Result of execution of the scripts  
    quePutTlv(&qVAK, tagOnlSta, 1, (byte *) "\x02");    //Online process status: 0= offline, 1=completed, 2= failed  
    quePutTlv(&qVAK, tagLstRspCod, 0, (byte *) "Y1Z1Y2Z2Y3Z300510104"); //M.List of possible response codes  

    quePutTag(&qTAK, tagIAD);   //Issuer Application Data
    quePutTag(&qTAK, tagATC);   //Application Transaction Counter (ATC)
    quePutTag(&qTAK, tagScrRes);    //Result of execution of the scripts

    quePutTlv(&qVAK, tagAuthRes, 1, (byte *) "\x01");   //Online process status: 0 declined, 1 accepted
    quePutTlv(&qVAK, tagRspCod, 2, (byte *) "00");  // Transaction response code
    quePutTlv(&qVAK, tagCommOk, 2, (byte *) "\x00\x01");    // Transaction response code

    cry = 0x40;
    ret = emvComplete(cry, &qVAK, &qTAK, &qVKA);
    CHECK(ret > 0, lblKO);

    ptr = 0;
    while(queLen(&qVKA)) {
        ret = queGetTlv(&qVKA, &tag, &len, buf);
        VERIFY(ret == len);
        VERIFY(len <= 256);

        if(tag != tagCID)
            continue;           //9F27 is Cryptogram Information Data (CID)

        cry = *buf & 0xF0;
        if(cry == 0x40)
            ptr = "APPROVED";
        else
            ptr = "DECLINED";
    }

    dspLS(1, ptr);

    dspLS(2, "OK!");
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
    trcS("tcab0020 End\n");
    return ret;
}
#else
int tcab0020(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
    return -1;
}
#endif
