/** \file 
 * Unitary test case for tcab0208.c: contactless Mifare card processing
 * \sa
 *  - mfcStart()
 *  - mfcStop()
 *  - mfcDetect()
 *  - mfcAut()
 *  - mfcLoad()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0208.c $
 *
 * $Id: tcab0208.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcab0208(void) {
#ifdef __MFC__
    int ret = 0;
    byte buf[16];
    char tmp[24];

    //byte recharge[4] = { 0x00,0x00,0x00,0x08 };

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "mfcStart...");
    CHECK(ret >= 0, lblKO);
    ret = mfcStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "mfcDetect...");
    CHECK(ret >= 0, lblKO);
    ret = mfcDetect(60);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(2, "mfcAut...");
    CHECK(ret >= 0, lblKO);
    ret = mfcAut(0, 0xA0);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(3, "mfcLoad...");
    CHECK(ret >= 0, lblKO);
    ret = mfcLoad(buf, 0x00, 'D');
    CHECK(ret >= 0, lblKO);

    bin2hex(tmp, buf, 8);
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);

    bin2hex(tmp, buf + 8, 8);
    ret = prtS(tmp);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(3);
    mfcStop();
    prtStop();
    dspStop();
#endif
}
