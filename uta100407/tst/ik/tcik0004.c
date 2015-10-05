#include "string.h"
#include <unicapt.h>
#include <message.h>
#include "sys.h"
#include "tst.h"
#include "pri.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcab0004. Bar code reader
//Functions testing:
// bcrStart()
// bcrStop()
// bcrGet()

void tcik0004(void) {
#ifdef __BCR__
    int ret, i;
    byte dat[255];
    char Nbr[3];
    tBuffer buf;
    char DateTimeBeg[12 + 1], DateTimeEnd[12 + 1], DateTimeResult[12 + 1];  //YYMMDDhhmmss

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = dspHgt(8);
    ret = dspWdt(30);
    ret = dspLS(0, "Test of 10 bar code reading");
    CHECK(ret >= 0, lblKO);
    bufInit(&buf, dat, 255);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    //Original time
    ret = getDateTime(DateTimeBeg);
    CHECK(ret >= 0, lblKO);

    for (i = 0; i < 10; i++) {
        ret = dspClear();
        ret = dspLS(0, "Test of 10 bar code reading");
        CHECK(ret >= 0, lblKO);
        ret = dspLS(1, "Activating...");
        CHECK(ret >= 0, lblKO);
        ret = bcrStart();
        CHECK(ret >= 0, lblKO);
        ret = dspLS(1, Nbr);
        CHECK(ret >= 0, lblKO);

        ret = dspLS(1, "Ready.");
        CHECK(ret >= 0, lblKO);
        ret = bcrGet(&buf, 10);
        CHECK(ret > 0, lblKO);
        strcpy(Nbr, " ");
        Beep();
        ret = dspLS(2, "Done, bar code:");
        CHECK(ret >= 0, lblKO);
        //ret= prtS((char *)bufPtr(&buf)); CHECK(ret >= 0, lblKO);
        ret = dspLS(3, (char *) bufPtr(&buf));
        CHECK(ret >= 0, lblKO);
        ret = dspLS(4, "enter Nbr (3 digits) :");
        CHECK(ret >= 0, lblKO);
        ret = enterStr(5, Nbr, 4);
        CHECK(ret >= 0, lblKO);
        //ret= prtS(Nbr); CHECK(ret >= 0, lblKO);
        ret = dspLS(4, "Nbr = ");
        CHECK(ret >= 0, lblKO);
        ret = dspLS(5, Nbr);
        CHECK(ret >= 0, lblKO);
        tmrPause(2);
        bcrStop();
    }
    //End time
    ret = getDateTime(DateTimeEnd);
    CHECK(ret >= 0, lblKO);
    ret = dspLS(2, "End of 10 reading");
    CHECK(ret >= 0, lblKO);
    subStr(DateTimeResult, DateTimeEnd, DateTimeBeg);
    ret = prtS(DateTimeResult);
    CHECK(ret >= 0, lblKO);     //just for test

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
