/** \file 
 * Unitary test case for sys component(tcik0159.c): Mifare Cards TeliumPass+ reader 
 * to i5100 and using PCD library
 * or to Teluim terminal
 * \sa
 *  - mfcStart()
 *  - mfcStop()
 *  - mfcLoad()
 *  - mfcSave()
 *  - mfcDebit()
 *  - mfcCredit()
 */

#include <string.h>
#include <stdio.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcik0159
//Mifare Cards manipulation with TeliumPass+, value operations
//connected to i5100 and using PCD library or connected to Telium terminal

void tcik0159(void) {
#ifdef __MFC__
    int ret;
    byte KEY_A[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    byte recharge[4] = { 0x00, 0x00, 0x00, 0x08 };
    byte amount[4] = { 0x00, 0x00, 0x00, 0x01 };
    byte buf[16];
    char print[24];

    ret = dspStart();
    // CHECK(ret>=0,lblKO);
    ret = prtStart();
    //CHECK(ret>=0,lblKO);

    trcS("Mifare Cards test :tcik0159\n");
    dspLS(0, "tcik0159");
    prtS("--------");
    prtS("tcik0159");
    prtS("--------");

    ret = mfcStart();
    CHECK(ret >= 0, lblKO);

    prtS("Detect card");
    ret = mfcDetect(60);
    CHECK(ret >= 0, lblKO);

    prtS("Authentication");
    ret = mfcAut(KEY_A, 0xA8);
    CHECK(ret >= 0, lblKO);

    prtS("Recharge");
    ret = mfcSave(recharge, 0x80, 'V');
    CHECK(ret >= 0, lblKO);

    prtS("Load data:");
    memset(buf, 0, sizeof(buf));
    ret = mfcLoad(buf, 0x80, 'V');
    CHECK(ret >= 0, lblKO);
    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X", buf[0], buf[1], buf[2], buf[3]);
    prtS(print);

    prtS("Credit");
    ret = mfcCredit(amount, 0x80);
    CHECK(ret >= 0, lblKO);

    prtS("Load data:");
    memset(buf, 0, sizeof(buf));
    ret = mfcLoad(buf, 0x80, 'V');
    CHECK(ret >= 0, lblKO);
    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X", buf[0], buf[1], buf[2], buf[3]);
    prtS(print);

    prtS("Debit");
    ret = mfcDebit(amount, 0x80);
    CHECK(ret >= 0, lblKO);

    prtS("Debit");
    ret = mfcDebit(amount, 0x80);
    CHECK(ret >= 0, lblKO);

    prtS("Load data:");
    memset(buf, 0, sizeof(buf));
    ret = mfcLoad(buf, 0x80, 'V');
    CHECK(ret >= 0, lblKO);
    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X", buf[0], buf[1], buf[2], buf[3]);
    prtS(print);

    prtS("Auth sector9 keyA");
    ret = mfcAut(KEY_A, 0xA9);
    CHECK(ret >= 0, lblKO);

    prtS("Trailer data:");
    memset(buf, 0, sizeof(buf));
    ret = mfcLoad(buf, 0x93, 'D');
    CHECK(ret >= 0, lblKO);
    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    prtS(print);
    memset(print, 0, sizeof(print));
    sprintf(print, "%02X %02X %02X %02X %02X %02X %02X %02X",
            buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14],
            buf[15]);
    prtS(print);

    prtS("tcik0159 OK :-)");
    goto lblEnd;
  lblKO:
    trcErr(ret);
    prtS("tcik0159 KO :-(");
  lblEnd:
    prtS("--------");
    mfcStop();
    prtStop();
    dspStop();
    tmrPause(3);
#endif
}
