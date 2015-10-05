// tcgs0017
//
// author:      Gábor Sárospataki
// date:        27.09.2006
// description: test case to cmbEq combo

#include "pri.h"
#include <string.h>
#define CHK CHECK(ret>=0,lblKO)

static int isEqual(tContainer * cntVal1, tContainer * cntVal2, int len) {
    int ret;
    tContainer cntTrue, cntFalse, cntEq;
    tCombo cmbIf, cmbEq;
    tLocator argEq[3], argIf[3];
    char buf[dspW + 1];

    ret = cntInit(&cntTrue, 's', "     Equal");
    CHK;
    ret = cntInit(&cntFalse, 's', "  Not Equal");
    CHK;
    ret = cntInit(&cntEq, 'x', &cmbEq);
    CHK;

    memset(argEq, 0, sizeof(argEq));
    memset(argIf, 0, sizeof(argIf));
    argEq[0].cnt = cntVal1;
    argEq[1].cnt = cntVal2;
    argEq[2].key = len;
    argIf[0].cnt = &cntEq;
    argIf[1].cnt = &cntTrue;
    argIf[2].cnt = &cntFalse;

    ret = cmbInit(&cmbIf, oprIf, argIf, dspW);
    CHK;
    ret = cmbInit(&cmbEq, oprEq, argEq, 1);
    CHK;
    ret = cmbGet(&cmbIf, 0, buf, dspW);
    CHK;
    CHECK(!strcmp(buf, "     Equal") || !strcmp(buf, "  Not Equal"), lblKO);
    ret = dspClear();
    CHK;
    ret = dspLS(0, "Status:");
    CHK;
    ret = dspLS(1, buf);
    CHK;
    tmrPause(1);
    return strcmp(buf, "  Not Equal") ? 1 : 0;
  lblKO:
    trcErr(ret);
    return -1;
}

static int isEqual2(tContainer * cntVal1, word val2, int len) {
    int ret;
    tContainer cntTrue, cntFalse, cntEq;
    tCombo cmbIf, cmbEq;
    tLocator argEq[3], argIf[3];
    char buf[dspW + 1];

    ret = cntInit(&cntTrue, 's', "     Equal");
    CHK;
    ret = cntInit(&cntFalse, 's', "  Not Equal");
    CHK;
    ret = cntInit(&cntEq, 'x', &cmbEq);
    CHK;

    memset(argEq, 0, sizeof(argEq));
    memset(argIf, 0, sizeof(argIf));
    argEq[0].cnt = cntVal1;
    argEq[1].cnt = 0;
    argEq[1].key = val2;
    argEq[2].key = len;
    argIf[0].cnt = &cntEq;
    argIf[1].cnt = &cntTrue;
    argIf[2].cnt = &cntFalse;

    ret = cmbInit(&cmbIf, oprIf, argIf, dspW);
    CHK;
    ret = cmbInit(&cmbEq, oprEq, argEq, 1);
    CHK;
    ret = cmbGet(&cmbIf, 0, buf, dspW);
    CHK;
    CHECK(!strcmp(buf, "     Equal") || !strcmp(buf, "  Not Equal"), lblKO);
    ret = dspClear();
    CHK;
    ret = dspLS(0, "Status:");
    CHK;
    ret = dspLS(1, buf);
    CHK;
    tmrPause(1);
    return strcmp(buf, "  Not Equal") ? 1 : 0;
  lblKO:
    trcErr(ret);
    return -1;
}

void tcgs0019(void) {
    int ret;
    tContainer cntVal1, cntVal2;
    tTable tbl1, tbl2;
    byte log1;
    word log2;
    card log4;

    nvmStart();
    ret = dspStart();
    CHK;
    //true string 
    ret = cntInit(&cntVal1, 's', "Blalala");
    CHK;
    ret = cntInit(&cntVal2, 's', "Blalala");
    CHK;
    ret = isEqual(&cntVal1, &cntVal2, 0);
    CHK;
    CHECK(ret, lblKO);
    //false string
    ret = cntInit(&cntVal1, 's', "Blalala");
    CHK;
    ret = cntInit(&cntVal2, 's', "23354");
    CHK;
    ret = isEqual(&cntVal1, &cntVal2, 0);
    CHK;
    CHECK(!ret, lblKO);
    //Init byte
    ret = tabInit(&tbl1, 0, 0, 1, 1);
    CHK;
    ret = cntInit(&cntVal1, 't', &tbl1);
    CHK;
    ret = tabInit(&tbl2, 0, 1, 1, 1);
    CHK;
    ret = cntInit(&cntVal2, 't', &tbl2);
    CHK;
    //true byte
    log1 = 67;
    ret = tabPut(&tbl1, 0, &log1, 1);
    CHK;
    ret = tabPut(&tbl2, 0, &log1, 1);
    CHK;
    ret = isEqual(&cntVal1, &cntVal2, 1);
    CHK;
    CHECK(ret, lblKO);
    //false byte
    log1 = 0;
    ret = tabPut(&tbl1, 0, &log1, 1);
    CHK;
    log1 = 17;
    ret = tabPut(&tbl2, 0, &log1, 1);
    CHK;
    ret = isEqual(&cntVal1, &cntVal2, 1);
    CHK;
    CHECK(!ret, lblKO);
    //Init word
    ret = tabInit(&tbl1, 0, 0, 2, 1);
    CHK;
    ret = cntInit(&cntVal1, 't', &tbl1);
    CHK;
    ret = tabInit(&tbl2, 0, 2, 2, 1);
    CHK;
    ret = cntInit(&cntVal2, 't', &tbl2);
    CHK;
    //true word
    log2 = 0xff00;
    ret = tabPut(&tbl1, 0, &log2, 2);
    CHK;
    ret = tabPut(&tbl2, 0, &log2, 2);
    CHK;
    ret = isEqual(&cntVal1, &cntVal2, 2);
    CHK;
    CHECK(ret, lblKO);
    //false word
    log2 = 56;
    ret = tabPut(&tbl1, 0, &log2, 2);
    CHK;
    log2 = 0xff00;
    ret = tabPut(&tbl2, 0, &log2, 2);
    CHK;
    ret = isEqual(&cntVal1, &cntVal2, 2);
    CHK;
    CHECK(!ret, lblKO);
    //Init card
    ret = tabInit(&tbl1, 0, 0, 4, 1);
    CHK;
    ret = cntInit(&cntVal1, 't', &tbl1);
    CHK;
    ret = tabInit(&tbl2, 0, 4, 4, 1);
    CHK;
    ret = cntInit(&cntVal2, 't', &tbl2);
    CHK;
    //true card
    log4 = 0xff000000;
    ret = tabPut(&tbl1, 0, &log4, 4);
    CHK;
    ret = tabPut(&tbl2, 0, &log4, 4);
    CHK;
    ret = isEqual(&cntVal1, &cntVal2, 4);
    CHK;
    CHECK(ret, lblKO);
    //false card
    log4 = 0xffaa;
    ret = tabPut(&tbl1, 0, &log4, 4);
    CHK;
    log4 = 0xffab;
    ret = tabPut(&tbl2, 0, &log4, 4);
    CHK;
    ret = isEqual(&cntVal1, &cntVal2, 4);
    CHK;
    CHECK(!ret, lblKO);

    //Init byte
    ret = tabInit(&tbl1, 0, 0, 1, 1);
    CHK;
    ret = cntInit(&cntVal1, 't', &tbl1);
    CHK;
    //true: bytte with a constant value
    log1 = 0xfe;
    ret = tabPut(&tbl1, 0, &log1, 1);
    CHK;
    ret = isEqual2(&cntVal1, 0xfe, 1);
    CHK;
    CHECK(ret, lblKO);
    //false: byte with a constant value
    log2 = 56;
    ret = tabPut(&tbl1, 0, &log2, 1);
    CHK;
    ret = isEqual2(&cntVal1, 0xf, 1);
    CHK;
    CHECK(!ret, lblKO);

    //Init word
    ret = tabInit(&tbl1, 0, 0, 2, 1);
    CHK;
    ret = cntInit(&cntVal1, 't', &tbl1);
    CHK;
    //true: word with a constant value
    log2 = 0xabab;
    ret = tabPut(&tbl1, 0, &log2, 2);
    CHK;
    ret = isEqual2(&cntVal1, 0xabab, 2);
    CHK;
    CHECK(ret, lblKO);
    //false: word with a constant value
    log2 = 56;
    ret = tabPut(&tbl1, 0, &log2, 2);
    CHK;
    ret = isEqual2(&cntVal1, 11, 2);
    CHK;
    CHECK(!ret, lblKO);
    goto lblEnd;

  lblKO:
    dspLS(0, "KO!");
  lblEnd:
    tmrPause(1);
    dspStop();
}
