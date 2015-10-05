//
// unitary test case tctd0008
//
// author:      Tamas Dani
// date:        01.09.2005
// description: test for combo Lrp and Cnt
//
// history:
// 05.11.2005   testing centering as well (testCnt)
//              using oprPad instead of oprCnt and oprCat instead of oprLrp

#include <string.h>
#include "pri.h"

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

static int testLrp(void);
static int testCnt(void);

void tctd0008(void){
    int ret;

    ret= dspStart(); CHK;
    ret= prtStart(); CHK;

    ret= testLrp();  CHK;
    ret= testCnt();  CHK;

    tmrPause(2);
    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(2);
    dspStop();
    prtStop();
}

static int testLrp(void){
    int ret;
    tTable tab;
    tContainer cnt;
    tCombo cmb;
    tLocator loc[3];
    char buf[prtW+1];
    char volDba[prtW*4];

    memset(volDba,0,prtW*4);
    dbaSetPtr(volDba);

    ret= tabInit(&tab,0xFF,0,prtW,2);
    VERIFY(ret==tabSize(&tab));
    ret= cntInit(&cnt,'t',&tab); CHECK(ret>0,lblKO); 

    loc[0].cnt= &cnt;
    loc[0].key= 0;
    loc[1].cnt= &cnt;
    loc[1].key= 1;
    loc[2].cnt= 0;
    loc[2].key= ' ';

    // testing in printer
    ret= cmbInit(&cmb,oprCat,loc,prtW+1); CHECK(ret>0,lblKO); 

    ret= tabPutStr(&tab,0,"left side");
    CHECK(ret>0,lblKO); 

    ret= tabPutStr(&tab,1,"right side");
    CHECK(ret>0,lblKO); 

    ret= cmbGet(&cmb,0,buf,prtW);
    CHECK(ret>0,lblKO); 

    prtS(buf);
    CHECK(strcmp(buf,"left side     right side")==0,lblKO); 
    prtS("Test OK");

    // testing in display
    ret= cmbInit(&cmb,oprCat,loc,dspW+1); CHECK(ret>0,lblKO); 

    ret= tabPutStr(&tab,0,"left");
    CHECK(ret>0,lblKO); 

    ret= tabPutStr(&tab,1,"right");
    CHECK(ret>0,lblKO); 

    ret= cmbGet(&cmb,0,buf,dspW);
    CHECK(ret>0,lblKO); 

    dspLS(0,buf);
    CHECK(strcmp(buf,"left       right")==0,lblKO);
    dspLS(1,"Test OK");

    ret= 1;
    goto lblEnd;
lblKO:
    trcErr(ret);
    ret= -1;
lblEnd:
    tmrPause(2);
    return ret;
}

static int testCnt(void){
    int ret;
    tTable tab;
    tContainer cnt;
    tCombo cmb;
    tLocator loc[3];
    char buf[prtW+1];
    char volDba[prtW*4];

    memset(volDba,0,prtW*4);
    dbaSetPtr(volDba);

    ret= tabInit(&tab,0xFF,0,prtW,2);
    VERIFY(ret==tabSize(&tab));
    ret= cntInit(&cnt,'t',&tab); CHECK(ret>0,lblKO); 

    loc[0].cnt= &cnt;
    loc[0].key= 0;
    loc[1].cnt= 0;
    loc[1].key= 'C';
    loc[2].cnt= &cnt;
    loc[2].key= 1;

    // testing in printer
    ret= cmbInit(&cmb,oprPad,loc,prtW+1); CHECK(ret>0,lblKO); 

    ret= tabPutStr(&tab,0,"centered");
    CHECK(ret>0,lblKO); 

    ret= tabPutStr(&tab,1,"                        ");
    CHECK(ret>0,lblKO); 

    ret= cmbGet(&cmb,0,buf,prtW);
    CHECK(ret>0,lblKO); 

    prtS(buf);
    CHECK(strcmp(buf,"        centered        ")==0,lblKO); 
    prtS("Test OK");

    // testing in display
    ret= cmbInit(&cmb,oprPad,loc,dspW+1); CHECK(ret>0,lblKO); 

    ret= tabPutStr(&tab,0,"centered");
    CHECK(ret>0,lblKO); 

    ret= tabPutStr(&tab,1,"                ");
    CHECK(ret>0,lblKO); 

    ret= cmbGet(&cmb,0,buf,dspW);
    CHECK(ret>0,lblKO); 

    dspLS(2,buf);
    CHECK(strcmp(buf,"    centered    ")==0,lblKO);
    dspLS(3,"Test OK");

    ret= 1;
    goto lblEnd;
lblKO:
    trcErr(ret);
    ret= -1;
lblEnd:
    tmrPause(2);
    return ret;
}
