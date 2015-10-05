/** \file
 * Unitary test case tcab0016.
 * Functions testing:
 * \sa
 *  - emvAuthenticate()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0016.c $
 *
 * $Id: tcab0016.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#include "emv.h"
#ifdef __EMV__
static byte mod99[] = {
    0xAB, 0x79, 0xFC, 0xC9, 0x52, 0x08, 0x96, 0x96,
    0x7E, 0x77, 0x6E, 0x64, 0x44, 0x4E, 0x5D, 0xCD,
    0xD6, 0xE1, 0x36, 0x11, 0x87, 0x4F, 0x39, 0x85,
    0x72, 0x25, 0x20, 0x42, 0x52, 0x95, 0xEE, 0xA4,
    0xBD, 0x0C, 0x27, 0x81, 0xDE, 0x7F, 0x31, 0xCD,
    0x3D, 0x04, 0x1F, 0x56, 0x5F, 0x74, 0x73, 0x06,
    0xEE, 0xD6, 0x29, 0x54, 0xB1, 0x7E, 0xDA, 0xBA,
    0x3A, 0x6C, 0x5B, 0x85, 0xA1, 0xDE, 0x1B, 0xEB,
    0x9A, 0x34, 0x14, 0x1A, 0xF3, 0x8F, 0xCF, 0x82,
    0x79, 0xC9, 0xDE, 0xA0, 0xD5, 0xA6, 0x71, 0x0D,
    0x08, 0xDB, 0x41, 0x24, 0xF0, 0x41, 0x94, 0x55,
    0x87, 0xE2, 0x03, 0x59, 0xBA, 0xB4, 0x7B, 0x75,
    0x75, 0xAD, 0x94, 0x26, 0x2D, 0x4B, 0x25, 0xF2,
    0x64, 0xAF, 0x33, 0xDE, 0xDC, 0xF2, 0x8E, 0x09,
    0x61, 0x5E, 0x93, 0x7D, 0xE3, 0x2E, 0xDC, 0x03,
    0xC5, 0x44, 0x45, 0xFE, 0x7E, 0x38, 0x27, 0x77
};
#endif

int tcab0016(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
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

    trcS("tcab0016 Beg\n");

    ret = tcab0015(0, dVAK, dTKA, dVKA, dTAK);  //Go through previous stages of EMV transaction
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
    dspLS(0, "emvAuthenticate...");

    quePutTag(&qTAK, tagTVR);   //Terminal Verification Result
    quePutTag(&qTAK, tagTSI);   //Transaction Status Information
    quePutTlv(&qVAK, tagRSAKey, 0x80, mod99);   //RSA key
    quePutTlv(&qVAK, tagRSAKeyExp, 0x01, (byte *) "\x03");  //Public Key Exponent

    quePutTlv(&qVAK, tagODA, 0x01, (byte *) "\x01");    //TAG_ODA_PROCESSING
    quePutTlv(&qVAK, tagDetFaitPriTAA, 0x01, (byte *) "\x01");  //TAG_DETECT_FAIL_PRIOR_TAA
    quePutTlv(&qVAK, tagCDANevReqARQ1, 0x01, (byte *) "\x00");  //TAG_CDA_NEVER_REQUEST_ARQC_1GENAC
    quePutTlv(&qVAK, tagCDANevReqONL2, 0x01, (byte *) "\x00");  //TAG_CDA_NEVER_REQUEST_ONLINE_2GENAC
    quePutTlv(&qVAK, tagTRMOverpassAIP, 0x01, (byte *) "\x00"); //Boolean TRM to be done without testing AIP if TRUE (0x01)

    ret = emvAuthenticate(&qVAK, &qTAK, &qVKA, &qTKA);
    CHECK(ret >= 0, lblKO);

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
    trcS("tcab0016 End\n");
    return ret;
#else
    return -1;
#endif
}
