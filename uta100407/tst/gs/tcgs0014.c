// tcgs0011
//
// author:      Gábor Sárospataki
// date:        01.02.2006
// description: oprSpr

#include "pri.h"
#include <string.h>
#define CHK CHECK(ret>=0,lblKO)

int tcgs0014(void) {
    enum { dim = 8 };
    int ret;
    tTable arg1;
    tTable arg2;
    tContainer cntArg1, cntArg2, cntCmb;
    tLocator arg[3];
    tCombo cmbSpr1, cmbSpr2;
    char bgd[prtW + 1];
    char str[prtW + 1];
    byte sec;
    word ofs;
    word len;
    char dst[prtW + 1];

    sec = 0x0;
    ofs = 0;

    //initialize the peripherials
    ret = prtStart();
    CHK;
    ret = dspStart();
    nvmStart();

    //initialize the fields
    len = prtW;
    ret = tabInit(&arg1, sec, ofs, len, 1);
    CHK;
    ofs += ret;
    ret = tabInit(&arg2, sec, ofs, len, 1);
    CHK;
    ofs += ret;

    //initialize the containers
    ret = cntInit(&cntArg1, 't', &arg1);
    CHK;
    ret = cntInit(&cntArg2, 't', &arg2);
    CHK;
    ret = cntInit(&cntCmb, 'x', &cmbSpr1);
    CHK;

    //initialize the locators's pointer for cmbSpr1
    arg[0].cnt = &cntArg1;
    arg[1].cnt = &cntArg2;
    arg[2].cnt = 0;

    arg[0].key = 0;
    arg[1].key = 0;
    arg[2].key = 0;

    ret = cmbInit(&cmbSpr1, oprSpr, arg, prtW);
    CHK;

    //initialize the locators's pointer for cmbSpr2
    arg[0].cnt = &cntArg1;
    arg[1].cnt = &cntCmb;
    arg[2].cnt = 0;

    arg[0].key = 0;
    arg[1].key = 0;
    arg[2].key = 0;

    ret = cmbInit(&cmbSpr2, oprSpr, arg, prtW);
    CHK;

    strcpy(bgd, "%s");
    ret = tabPut(&arg1, 0, bgd, prtW);
    CHK;
    strcpy(str, "");
    ret = tabPut(&arg2, 0, str, prtW);
    CHK;

    ret = prtS("The first combo.");
    memset(dst, 0, sizeof(dst));
    ret = cmbGet(&cmbSpr1, 0, dst, prtW);
    CHK;
    ret = prtS(dst);
    CHK;
    CHECK(strcmp(dst, "") == 0, lblKO); //empty sting
    tmrPause(1);

    ret = prtS("The second combo.");
    memset(dst, 0, sizeof(dst));
    ret = cmbGet(&cmbSpr2, 0, dst, prtW);
    CHK;
    ret = prtS(dst);
    CHK;
    CHECK(strcmp(dst, "") == 0, lblKO);

    ret = 1;
    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
    tmrPause(2);
    ret = -1;
  lblEnd:
    dspStop();
    prtStop();
    return ret;
}
