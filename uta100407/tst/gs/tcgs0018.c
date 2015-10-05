// tcgs0020
//
// author:      Gábor Sárospataki
// date:        27.19.2006
// description: test case to cmbIf combo

#include "pri.h"
#include <string.h>

#define CHK CHECK(ret>=0,lblKO)

static int isTrueFalse(tContainer * cntCnd, int len) {
    int ret;
    tContainer cntTrue, cntFalse;
    tCombo cmbIf;
    tLocator arg[3];
    char buf[dspW + 1];

    ret = cntInit(&cntTrue, 's', "True");
    CHK;
    ret = cntInit(&cntFalse, 's', "False");
    CHK;

    memset(arg, 0, sizeof(arg));
    arg[0].cnt = cntCnd;
    arg[1].cnt = &cntTrue;
    arg[2].cnt = &cntFalse;

    ret = cmbInit(&cmbIf, oprIf, arg, (byte) len);
    CHK;
    ret = cmbGet(&cmbIf, 0, buf, dspW);
    CHK;
    CHECK(!strcmp(buf, "True") || !strcmp(buf, "False"), lblKO);
    ret = dspClear();
    CHK;
    ret = dspLS(0, "Status");
    CHK;
    ret = dspLS(1, buf);
    CHK;
    tmrPause(1);
    return strcmp(buf, "False");
  lblKO:
    trcErr(ret);
    return -1;
}

void tcgs0018(void) {
    int ret;
    tContainer cntCnd;
    tTable tbl;
    byte log1;

    nvmStart();
    ret = dspStart();
    CHK;
    //true string 
    ret = cntInit(&cntCnd, 's', "Blalala");
    CHK;
    ret = isTrueFalse(&cntCnd, 0);
    CHK;
    CHECK(ret, lblKO);
    //false string
    ret = cntInit(&cntCnd, 's', "");
    CHK;
    ret = isTrueFalse(&cntCnd, 0);
    CHK;
    CHECK(!ret, lblKO);
    //Init byte
    ret = tabInit(&tbl, 0, 0, 1, 1);
    CHK;
    ret = cntInit(&cntCnd, 't', &tbl);
    CHK;
    //true byte
    log1 = 67;
    ret = tabPut(&tbl, 0, &log1, 1);
    CHK;
    ret = isTrueFalse(&cntCnd, 1);
    CHK;
    CHECK(ret, lblKO);
    //false byte
    log1 = 0;
    ret = tabPut(&tbl, 0, &log1, 1);
    CHK;
    ret = isTrueFalse(&cntCnd, 1);
    CHK;
    CHECK(!ret, lblKO);
    goto lblEnd;

  lblKO:
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(1);
    dspStop();
}
