#include "sys.h"
#include "tst.h"

//Unitary test case tcab0131.
//printing in arabic font

//Iso8859_35 is bold only
//Iso8859_35_b prints garbage
//Iso8859_35 makes english in bold
//Iso8859_35_b makes english small and bold
//Iso8859_35: no normal, no inverted (display and printed)
//Iso8859_35_b: no bold, no inverted (display and printed)

//Needed: free switching normal-bold-inverted in the same way as it works in english

void tcab0131(void) {
    int ret;
    char *ptr;

    ret = cryStart('M');
    CHECK(ret >= 0, lblKO);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    //ptr= "³À.² º¾¼Ã½¸º°Æ¸Ï"; //5 cyrillic
    //ptr= "  ÚJ ALKALMAZÁS "; //2 hungarian
    ptr = "         \xE2\xFE\xEC\xF8\x97\xF3\x90";
    ret = prtS(ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(BLD(2), ptr);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(INV(3), ptr);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    trcErr(ret);
  lblEnd:
    prtStop();
    dspStop();
    //cryStop('M'); 
    tmrPause(3);
}
