#include <string.h>
#include "pri.h"
#include "tst.h"

#ifdef __CANVAS__
static code const char *srcFile = __FILE__;

//Unitary test case tcab0170.
//guiMenuSelect testing: discarded
// obsolete

#define RUN(F) ret= F(); CHECK(ret>=0,lblKO)
void tcab0170(void) {
    int ret;
    tMenu mnu;
    char *itm[] = {
        "UTA members",
        "Iran",
        "Cameroun",
        "Senegal",
        "Arab Emirates",
        "Hungary",
        "France",
        "Switzerland",
        "India",
        "Sri Lanka",
        "Bangladesh",
        "Lebanon",
        "Morocco",
        "Austria",
        "Slovenia",
        "Georgia",
        "Indonesia",
        "Algeria",
        "China",
        "Philippines",
        "Greece",
        "Ukraine",
        0
    };
    byte len, mar;
    const char *ptr;

    RUN(dspStart);
    RUN(prtStart);
    RUN(cnvStart);

    //build menu
    len = cnvH / MNUROW;
    if(len > 4)
        len -= 2;
    ret = drwMenuInit(&mnu, (Pchar *) itm, len, 0);
    CHECK(ret >= 0, lblKO);

    mar = 0;
    if(len > 4)
        mar = 8;

    RUN(cnvClear);
    ret = guiMenuSelect(&mnu, mar, mar, "kt", 0, 60);
    CHECK(ret >= 0, lblKO);

    if(ret)
        ptr = itm[LBYTE(ret)];
    else
        ptr = "cancelled";
    ret = prtS(ptr);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    cnvStop();
    prtStop();
    dspStop();
}
#else
void tcab0170(void) {
}
#endif
