#include "string.h"
#include <unicapt.h>
#include <message.h>
#include "sys.h"
#include "tst.h"
#include "pri.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcab0002. Bar code reader
//Functions testing:
// bcrStart()
// bcrStop()
// bcrGet()

void tcik0002(void) {
#ifdef __BCR__
    int ret;
    byte dat[255];
    char Nbr[3];
    tBuffer buf;

    bufInit(&buf, dat, 255);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    while(strcmp(Nbr, "0")) {
        ret = dspClear();
        ret = dspLS(1, Nbr);
        CHECK(ret >= 0, lblKO);
        ret = dspHgt(8);
        ret = dspWdt(24);
        ret = dspLS(0, "Activating...");
        CHECK(ret >= 0, lblKO);
        ret = bcrStart();
        CHECK(ret >= 0, lblKO);
        ret = dspLS(1, Nbr);
        CHECK(ret >= 0, lblKO);

        ret = dspLS(1, "Ready.");
        CHECK(ret >= 0, lblKO);
        ret = bcrGet(&buf, 10);
        CHECK(ret > 0, lblKO);
        strcpy(Nbr, "");
        Beep();
        ret = dspLS(2, "Done, bar code: ");
        CHECK(ret >= 0, lblKO);
        ret = prtS((char *) bufPtr(&buf));
        CHECK(ret >= 0, lblKO);
        ret = dspLS(3, (char *) bufPtr(&buf));
        CHECK(ret >= 0, lblKO);
        ret = dspLS(4, "enter Nbr (3 digits) :");
        CHECK(ret >= 0, lblKO);
        ret = enterStr(5, Nbr, 4);
        CHECK(ret >= 0, lblKO);
        ret = prtS(Nbr);
        CHECK(ret >= 0, lblKO);
        ret = dspLS(6, "Nbr = ");
        CHECK(ret >= 0, lblKO);
        ret = dspLS(7, Nbr);
        CHECK(ret >= 0, lblKO);
        tmrPause(3);
        bcrStop();
    }
    goto lblEnd;
  lblKO:
    dspLS(3, "KO");
    bcrStop();
  lblEnd:
    tmrPause(3);

    prtStop();
    dspStop();
#endif
}
