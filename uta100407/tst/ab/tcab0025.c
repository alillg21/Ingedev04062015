/** \file
 * Unitary test case tcab0025.
 * Functions testing:
 * \sa
 *  - mnuSelect()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0025.c $
 *
 * $Id: tcab0025.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

void tcab0025(void) {
    int ret;
    char buf[dspW + 1];
    char *mnu[] = {
        "   MENU0",
        "MENU1",
        "MENU2",
        "MENU3",
        "MENU4",
        "MENU5",
        "MENU6",
        "MENU7",
        "MENU8",
        "MENU9",
        "MENUA",
        "MENUB",
        "MENUC",
        "MENUD",
        "MENUE",
        "MENUF",
        "MENUG",
        0
    };

    trcS("tcab0025 Beg\n");
    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    ret = mnuSelect((Pchar *) mnu, 1 * MNUMAX + 1, 60); //play with menu
    CHECK(ret >= 0, lblKO);

    dspClear();
    strcpy(buf, "STOP");        //if nothing selected, STOP will be displayed
    if(ret)
        strcpy(buf, mnu[ret % MNUMAX]); //otherwise the menu item is displayed
    ret = dspLS(1, buf);
    CHECK(ret >= 0, lblKO);

    tmrPause(3);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    dspStop();                  //close resource
    trcS("tcab0025 End\n");
}
