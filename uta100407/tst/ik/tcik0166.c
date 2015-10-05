/// \file tcik0166.c

#include <string.h>
#include "sys.h"
#include "tst.h"

/** Unitary UTA test case : mifare card implecit selection
 * \sa
 * - mfcStart()
 * - mfcStop()
 * - mfcDetect()
 * - clsBip()
 * - clsText()
 */

void tcik0166(void) {
#ifdef __MFC__
    int ret;
    byte dly;
    byte KEY_A[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    dspLS(0, "tcik0166");

    ret = prtStart();           //Does not work with CLESS in real mode
    // CHECK(ret >= 0, lblKO);

    trcS("Cless detection :tcik0166\n");
    prtS("--------");
    prtS("tcik0166");

    ret = mfcStart();
    CHECK(ret >= 0, lblKO);

    ret = clsText(1, "tcik0166");
    CHECK(ret >= 0, lblKO);

    dly = 0;
    ret = clsBip(dly);
    CHECK(ret >= 0, lblKO);

    ret = clsClearDsp();
    CHECK(ret >= 0, lblKO);

    dspLS(1, "Card detected");
    prtS("Card detected");
    ret = clsText(1, "Card detected");
    CHECK(ret >= 0, lblKO);

    ret = clsClearDsp();
    CHECK(ret >= 0, lblKO);

    dspLS(1, "If you whant to pay");
    dspLS(2, "Return your card !!!");
    prtS("Return your card !!!");
    ret = clsText(1, "If you whant");
    CHECK(ret >= 0, lblKO);
    ret = clsText(2, "TO PAY");
    CHECK(ret >= 0, lblKO);
    ret = clsText(3, "RETURN YOUR CARD !!!");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = mfcDetect(10);
    CHECK(ret >= 0, lblKO);

    ret = clsClearDsp();
    CHECK(ret >= 0, lblKO);

    dspLS(1, "Detect card for payment");
    prtS("Detect card for payment");
    ret = clsText(1, "Detect card for payment");
    CHECK(ret >= 0, lblKO);

    dspLS(2, "Authentication");
    prtS("Authentication");
    ret = clsText(2, "Authentication");
    CHECK(ret >= 0, lblKO);

    ret = mfcAut(KEY_A, 0xA8);
    CHECK(ret >= 0, lblKO);

    ret = clsBip(dly);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = clsText(4, "end treatment");
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    dspLS(1, "Test Fail");
    clsText(3, "Test Fail");
    prtS("Test Fail");
    goto lblEnd;
  lblEnd:
    prtS("--------");
    dspLS(1, "Remove your card");
    clsText(1, "Remove your card");
    tmrPause(1);
    clsClearDsp();
    mfcStop();
    prtStop();
    dspStop();
    tmrPause(3);
#endif
}
