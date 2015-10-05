/** \file
 * Unitary test case tcab0017.
 * Functions testing:
 * \sa
 *  - emvCVMstart()
 *  - emvCVMoff()
 *  - cryEmvPin()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0017.c $
 *
 * $Id: tcab0017.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __EMV__
#include "emv.h"

int tcab0017(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
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
    word res;
    word len;

    trcS("tcab0017 Beg\n");

    ret = tcab0016(0, dVAK, dTKA, dVKA, dTAK);  //Go through previous stages of EMV transaction
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
    dspLS(0, "emvCVMstart...");

    quePutTag(&qTAK, tagCVMOutRes); //CVMOUT Result
    quePutTag(&qTAK, 0x95);     //Terminal Verification Result
    quePutTag(&qTAK, 0x9B);     //Transaction Status Information
    quePutTag(&qTAK, 0x9F34);   //CVM Result
    quePutTlv(&qVAK, 0x81, 4, (byte *) "\x00\x00\x00\x7B"); //Amount, Authorised (Binary), =123
    ret = emvCVMstart(&qVAK, &qTAK, &qVKA); //start CVM processing
    CHECK(ret >= 0, lblKO);

    ret = queLen(&qVKA);
    VERIFY(ret > 0);

    ret = queGetTlv(&qVKA, &tag, &len, (byte *) & res);
    VERIFY(ret == len);
    VERIFY(tag == tagCVMOutRes);    //the first tag is 1F38
    //VERIFY(len == 1);         //it is 1 in EMV Kernel 3 and 2 in EMV Kernel 4
    CHECK(res & 0x08, lblKO);   //OFFLINE PIN - this test is designed for offline pin only

    dspLS(1, "OK");
    tmrPause(1);

    dspStop();                  //stop HMI before starting SSA

    ret = cryStart('m');        //start cryptomodule
    CHECK(ret >= 0, lblKO);
    //ret = cryEmvPin("111111\nENTER PIN:\n\n");  //8881 for EndToEnd Pack
    ret = cryEmvPin("PLEASE ENTER\nPIN:\n\n");  //8881 for EndToEnd Pack    
    CHECK(ret == kbdVAL, lblKO);
    cryStop('m');               //stop cryptomodule
    dspStart();                 //now HMI can be started again

    dspClear();
    dspLS(0, "emvCVMoff...");
    queReset(&qTAK);
    queReset(&qVKA);
    quePutTag(&qTAK, tagCVMOutRes); //CVMOUT Result
    quePutTag(&qTAK, 0x95);     //Terminal Verification Result
    quePutTag(&qTAK, 0x9B);     //Transaction Status Information
    quePutTag(&qTAK, 0x9F34);   //CVM Result
    ret = emvCVMoff(&qTAK, &qVKA);  //offline pin entered
    CHECK(ret >= 0, lblKO);
    dspLS(2, "OK");

    ret = queLen(&qVKA);
    VERIFY(ret > 0);

    ret = queGetTlv(&qVKA, &tag, &len, (byte *) & res);
    VERIFY(ret == len);
    VERIFY(tag == tagCVMOutRes);    //the first tag is 1F38
    //VERIFY(len == 1);
    //CHECK(res & 0x01, lblKO);   //ending

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
    trcS("tcab0017 End\n");
    return ret;
}
#else
int tcab0017(int stop, byte * dVAK, byte * dTKA, byte * dVKA, byte * dTAK) {
    return -1;
}
#endif
