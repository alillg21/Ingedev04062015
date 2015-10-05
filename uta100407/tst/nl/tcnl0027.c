#include <string.h>
#include "sys.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcnl0027.
//Functions testing:
//  dayOfWeek

void tcnl0027(void) {
    int ret;
    card DayOfWeek;

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, "dayOfWeek");
    CHECK(ret >= 0, lblKO);

    //Sunday is zero
    ret = dayOfWeek("20070924", &DayOfWeek);
    CHECK(ret >= 0, lblKO);
    CHECK(DayOfWeek == 1, lblKO);

    ret = dayOfWeek("20090101", &DayOfWeek);
    CHECK(ret >= 0, lblKO);
    CHECK(DayOfWeek == 4, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();                  //close resource
}
