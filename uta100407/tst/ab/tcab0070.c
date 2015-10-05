#include "sys.h"
#include "tst.h"

//Unitary test case tcab0070.
//Testing local fonts.
//To perform this test the variable __FONT__ in the file country.h 
// should be set to a proper value.
//The SSA code should correspond to the value of __FONT__

void tcab0070(void) {
    int ret;
    static const char *str = "\xe1\xe9\xed";

    ret = cryStart('M');        //open SSA to be able to select the proper font
    CHECK(ret >= 0, lblKO);

    ret = dspStart();           //at this moment the font is selected
    CHECK(ret >= 0, lblKO);

    ret = dspLS(0, str);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(INV(1), str);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(BLD(2), str);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = dspLS(INV(BLD(3)), str);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = prtS(str);
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;
  lblKO:
    trcErr(ret);
    tmrPause(3);
  lblEnd:
    dspStop();
    prtStop();
    cryStop('M');
}
