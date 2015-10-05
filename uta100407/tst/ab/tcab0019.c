/** \file
 * Unitary test case tcab0019.
 * Functions testing:
 * \sa
 *  - emvAnalyse()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0019.c $
 *
 * $Id: tcab0019.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __EMV__
#include "emv.h"

int tcab0019(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
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
    char tmp[dspW + 1];
    char *ptr = 0;
    card tag;
    byte cry;
    word len;

    trcS("tcab0019 Beg\n");

    ret = tcab0018(0, dVAK, dTKA, dVKA, dTAK);  //Go through previous stages of EMV transaction
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
    dspLS(0, "emvAnalyse...");

    quePutTag(&qTAK, tagTVR);   //Terminal Verification Result
    quePutTag(&qTAK, tagTSI);   //Transaction Status Information
    quePutTag(&qTAK, tagIssTrnCrt); //Issuer Transaction Certificate
    quePutTag(&qTAK, tagCID);   //Cryptogram Information Data (CID)
    quePutTag(&qTAK, tagUnpNum);    //Unpredictable Number
    quePutTag(&qTAK, tagCVMRes);    //Cardholder Verification Method (CVM) Results
    quePutTag(&qTAK, tagIAD);   //Issuer Application Data
    quePutTag(&qTAK, tagATC);   //Application Transaction Counter (ATC)
    quePutTag(&qTAK, tagPAN);   //Application Primary Account Number (PAN)
    quePutTag(&qTAK, tagAplUsgCtl); //Application Usage Control

    quePutTag(&qTAK, tagTrmDc1);    //Terminal decision for Generate AC (TC,AAC,ARQC)
    quePutTag(&qTAK, tagTrmDc2);    //Terminal decision after Generate AC 0=Declined, 1= Approved

    quePutTlv(&qVAK, tagAmtOthNum, 6, (byte *) "\x00\x00\x00\x00\x00\x00"); //Amount, Other (Numeric)
    ret = emvAnalyse(&qVAK, &qTAK, &qVKA, &qTKA);
    CHECK(ret > 0, lblKO);

    while(queLen(&qVKA)) {      //retrieve the cryptogram type
        ret = queGetTlv(&qVKA, &tag, &len, buf);
        VERIFY(ret == len);
        VERIFY(len <= 256);

        if(tag != tagCID)
            continue;           //9F27 is Cryptogram Information Data (CID)

        cry = *buf & 0xF0;
        ptr = 0;
        switch (cry) {
          case 0x00:
              ptr = "AAC";
              break;
          case 0x40:
              ptr = "TC";
              break;
          case 0x80:
              ptr = "ARQC";
              break;
          default:
              VERIFY(cry == 0xC0);
              ptr = "AAR";
              break;
        }
    }
    VERIFY(ptr);
    strcpy(tmp, "emvAnalyse ");
    strcat(tmp, ptr);
    dspLS(1, tmp);

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
    trcS("tcab0019 End\n");
    return ret;
}
#else
int tcab0019(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
    return -1;
}
#endif
