/** \file
 * Unitary test case tcab0015.
 * \sa
 * Functions testing:
 *  - emvPrepare()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0015.c $
 *
 * $Id: tcab0015.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#include "emv.h"

int tcab0015(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
#ifdef __EMV__
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
    card tag;
    byte buf[256];
    char tmp[256];
    byte idx;
    word len;

    trcS("tcab0015 Beg\n");

    ret = tcab0014(0, dVAK, dTKA);  //Go through previous stages of EMV transaction
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
    dspLS(0, "emvPrepare...");

    quePutTag(&qTAK, tagChdNam);    //Cardholder Name
    quePutTag(&qTAK, tagCapkIdx);   //Certificate Authority Public Key Index ICC
    quePutTag(&qTAK, tagAIP);   //Application Interchange Profile
    quePutTag(&qTAK, tagPANSeq);    //Application Primary Account Number (PAN) Sequence Number
    quePutTag(&qTAK, tagTrk2);  //Track 2 Equivalent Data
    quePutTag(&qTAK, tagPAN);   //Application Primary Account Number (PAN)
    quePutTag(&qTAK, tagExpDat);    //Application Expiration Date
    quePutTag(&qTAK, tagEffDat);    //Application Effective Date
    quePutTag(&qTAK, tagIssCntCod); //Issuer Country Code
    quePutTag(&qTAK, tagTSI);   //Transaction Status Information
    quePutTag(&qTAK, tagIACDft);    //Issuer Action Code Default
    quePutTag(&qTAK, tagIACDnl);    //Issuer Action Code Denial
    quePutTag(&qTAK, tagIACOnl);    //Issuer Action Code Online
    quePutTag(&qTAK, tagCVM);   //Cardholder Verification Method (CVM) List
    quePutTag(&qTAK, tagDFNam); //Dedicated File (DF) Name
    quePutTag(&qTAK, tagAppLbl);    //Application Label
    quePutTag(&qTAK, tagAppPrfNam); //Application Preferred Name

    ret = emvPrepare(&qVAK, &qTAK, &qVKA, &qTKA);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "OK");
    CHECK(ret >= 0, lblKO);

    while(queLen(&qVKA)) {
        memset(buf, 0, 256);
        memset(tmp, 0, 256);

        ret = queGetTlv(&qVKA, &tag, &len, buf);
        VERIFY(ret == len);
        VERIFY(len <= 256);

        switch (tag) {
          case tagChdNam:
              prtS("5F20, Cardholder Name:");
              prtS((char *) (buf + (strlen((char *) buf) - prtW)));
              break;
          case tagTrk2:
              prtS("57, Track 2:");
              VERIFY(ret <= 128);
              ret = bin2hex(tmp, buf, (byte) ret);
              idx = 0;
              while(tmp[idx]) {
                  if(tmp[idx] == 'D')
                      tmp[idx] = '=';
                  idx++;
              }
              prtS(tmp);
              break;
          case tagPAN:
              prtS("5A, PAN:");
              VERIFY(ret <= 128);
              ret = bin2hex(tmp, buf, (byte) ret);
              prtS(tmp);
              break;
          case tagAppLbl:
              prtS("50, Label:");
              VERIFY(ret <= 128);
              prtS((char *) buf);
              break;
          case tagAppPrfNam:
              prtS("9F12, Preferred Name:");
              VERIFY(ret <= 128);
              prtS((char *) buf);
              break;
          case tagExpDat:
              prtS("5F24, Expiration Date:");
              VERIFY(ret <= 128);
              ret = bin2hex(tmp, buf, (byte) ret);
              prtS(tmp);
              break;
          case tagEffDat:
              prtS("5F25, Effective Date:");
              VERIFY(ret <= 128);
              ret = bin2hex(tmp, buf, (byte) ret);
              prtS(tmp);
              break;
          case tagIssCntCod:
              prtS("5F28 Issuer Country Code:");
              VERIFY(ret <= 128);
              ret = bin2hex(tmp, buf, (byte) ret);
              prtS(tmp);
              break;
          default:
              break;
        }
    }

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
    trcS("tcab0015 End\n");
    return ret;
#else
    return -1;
#endif
}
