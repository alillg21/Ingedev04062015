#include "sys.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcab0141. Bar code reader.Obsolete
//Functions testing:
// bcrStart()
// bcrStop()
// bcrGet()

void tcab0141(void) {
#ifdef __BCR__
    int ret;
    byte dat[255];
    tBuffer buf;

    bufInit(&buf, dat, 255);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "Activating...");
    CHECK(ret >= 0, lblKO);
    ret = bcrStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "Ready.");
    CHECK(ret >= 0, lblKO);
    ret = bcrGet(&buf, 10);
    CHECK(ret > 0, lblKO);

    Beep();
    ret = dspLS(2, "Done");
    CHECK(ret >= 0, lblKO);
    ret = prtS((char *) bufPtr(&buf));
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(3, "KO");
  lblEnd:
    tmrPause(3);
    bcrStop();
    prtStop();
    dspStop();
#endif
}
