/** \file
 * Unitary test case tcab0029. Structure sTable methods testing
 * Functions testing:
 * \sa
 *  - dbaSetPtr()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0029.c $
 *
 * $Id: tcab0029.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

static int testTab(tTable * tab) {
    int ret;
    char tmp[dspW + 1];

    VERIFY(tabSize(tab) == dspW * 3);
    VERIFY(tabDim(tab) == 3);
    VERIFY(tabRowFmt(tab) == 'S');

    ret = dspLS(2, "tabReset...");
    ret = prtS("tabReset...");

    ret = tabReset(tab);        //fill all memory reserved by zeroes
    VERIFY(ret == (int) tabSize(tab));

    ret = tabFlush(tab);
    CHECK(ret == 0 || ret == (int) tabSize(tab), lblKO);

    tmrPause(1);

    ret = dspLS(3, "tabPutStr...");
    ret = prtS("tabPutStr...");

    ret = tabPutStr(tab, 0, "Row 0");   //save the string with ending zero into row 0
    CHECK(ret == (int) strlen("Row 0") + 1, lblKO);
    ret = prtS("\"Row 0\"");

    ret = tabPutStr(tab, 1, "Row 1");   //save the string with ending zero into row 1
    CHECK(ret == (int) strlen("Row 1") + 1, lblKO);
    ret = prtS("\"Row 1\"");

    ret = tabPutStr(tab, 2, "Row 2");   //save the string with ending zero into row 2
    CHECK(ret == (int) strlen("Row 2") + 1, lblKO);
    ret = prtS("\"Row 2\"");

    tmrPause(1);

    ret = dspLS(0, "tabGetStr...");
    ret = prtS("tabGetStr...");
    ret = tabGetStr(tab, 0, tmp);   //retrieve row 0
    CHECK(ret == dspW + 1, lblKO);  //ending zero is added

    ret = dspLS(1, tmp);
    CHECK(strcmp(tmp, "Row 0") == 0, lblKO);
    ret = prtS("\"Row 0\"");

    tmrPause(1);

    ret = tabGetStr(tab, 1, tmp);   //retrieve row 1
    CHECK(ret == dspW + 1, lblKO);

    ret = dspLS(2, tmp);
    CHECK(strcmp(tmp, "Row 1") == 0, lblKO);
    ret = prtS("\"Row 1\"");

    tmrPause(1);

    ret = tabGetStr(tab, 2, tmp);   //retrieve row 2
    CHECK(ret == dspW + 1, lblKO);

    ret = dspLS(3, tmp);
    CHECK(strcmp(tmp, "Row 2") == 0, lblKO);
    ret = prtS("\"Row 2\"");

    ret = tabFlush(tab);
    CHECK(ret == 0 || ret == (int) tabSize(tab), lblKO);

    tmrPause(1);

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
    ret = -1;
  lblEnd:
    return ret;
}

void tcab0029(void) {
    int ret;
    byte volDba[128];
    byte cch[128];
    tTable tab;

    trcS("tcab0029 Beg\n");
    nvmStart();                 //initialise non-volatile memory

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    ret = prtS("**** Vol. mem. RAM ****");
    ret = dspLS(0, "dbaSetPtr...");
    ret = prtS("dbaSetPtr...");
    memset(volDba, 0, 128);
    dbaSetPtr(volDba);          //the buffer volDba will be used for volatile section of data base

    tmrPause(1);

    ret = dspLS(1, "Vol tabInit...");
    ret = prtS("Vol tabInit...3 Rows 16");
    ret = tabInit(&tab, 0xFF, 0, dspW, 3, 'S'); //initiate a table in volatile memory of three rows of width dspW
    VERIFY(ret == (int) tabSize(&tab));

    ret = tabDim(&tab);         //get the number of rows in the table
    CHECK(ret == 3, lblKO);

    ret = tabWdt(&tab);         //get the length of a row in the table
    CHECK(ret == dspW, lblKO);

    ret = tabSize(&tab);        //get the size of memory reserved for the table
    CHECK(ret == dspW * 3, lblKO);

    tmrPause(1);

    ret = testTab(&tab);        //test volatile memory table
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = prtS("** Non Vol. mem. DFS **");
    ret = dspLS(1, "Nvm tabInit...");
    ret = prtS("Nvm tabInit...3 Rows 16");
    CHECK(ret >= 0, lblKO);

    ret = tabInit(&tab, 0, 0, dspW, 3, 'S');    //initiate a table in non-volatile memory of three rows of width dspW
    VERIFY(ret == (int) tabSize(&tab));

    tmrPause(1);

    ret = testTab(&tab);        //test non-volatile memory table
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);

    ret = prtS("*** Cache mem. ****");
    ret = dspLS(1, "Cache...");
    ret = prtS("Cache... DFS to RAM");
    CHECK(ret >= 0, lblKO);

    ret = tabCache(&tab, cch, 128);
    CHECK(ret >= 0, lblKO);

    tmrPause(1);

    ret = testTab(&tab);        //test a table with cache
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    dspStop();                  //close resource
    prtStop();
    trcS("tcab0029 End\n");
}
