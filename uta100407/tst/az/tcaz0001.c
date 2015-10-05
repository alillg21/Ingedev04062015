/** \file
 * Unitary test case tcaz0001.
 * Testing combo.
 * \sa
 *  - cmbInit()
 *  - cmbGet()
 */

#include <string.h>
#include "pri.h"
#include "tst.h"


static int testPad(void) {
    int ret;
    tTable tab;
    tContainer cnt;
    tCombo cmb;
    tLocator loc[3];
    char buf[prtW + 1];
    char volDba[prtW * 2];
    trcS("tcaz0001 Beg\n");

    memset(volDba, 0, prtW * 2);
    
    dbaSetPtr(volDba);

    ret = dspLS(1, "tabInit...");
    CHECK(ret >= 0, lblKO);

    ret = tabInit(&tab, 0xFF, 0, prtW, 2, 0); 
    VERIFY(ret == (int) tabSize(&tab));

    tmrPause(1);

    ret = dspLS(2, "cntInit...");
    CHECK(ret >= 0, lblKO);
    ret = cntInit(&cnt, 't', &tab);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    ret = dspLS(3, "cmbInit...");
    CHECK(ret >= 0, lblKO);

    loc[0].cnt = &cnt;
    loc[0].key = 0;
    loc[1].cnt = 0;             //padding type is transferred by value
    loc[1].key = 'S';
    loc[2].cnt = &cnt;
    loc[2].key = 1;

    ret = cmbInit(&cmb, oprPad, loc, prtW + 1);
    CHECK(ret > 0, lblKO);

    tmrPause(1);

    ret = tabPutStr(&tab, 0, "Password");  //original string
    CHECK(ret > 0, lblKO);

    ret = tabPutStr(&tab, 1, "****************");   			   //padding character control string
    CHECK(ret > 0, lblKO);

    ret = cmbGet(&cmb, 0, buf, prtW + 1);   	   
    CHECK(ret > 0, lblKO);

    ret = prtS(buf);
    CHECK(ret > 0, lblKO);
    CHECK(strcmp(buf, "********") == 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = -1;
  lblEnd:
    return ret;
}

void tcaz0001(void) {
    int ret;

    nvmStart();                 //initialise non-volatile memory
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspClear();
    CHECK(ret >= 0, lblKO);
    ret = dspLS(1, "Pad Subst...");
    CHECK(ret >= 0, lblKO);
    ret = testPad();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

     goto lblEnd;

  lblKO:
    dspLS(0, "KO");
  lblEnd:
    tmrPause(3);
    dspStop();
    prtStop();                  //close resource
    trcS("tcaz0001 End\n");
}
