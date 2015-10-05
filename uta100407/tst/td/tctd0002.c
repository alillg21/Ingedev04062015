
// Functional test case tctd0002.
//
// author:      Tamas Dani
// date:        11.07.2005
// description: test case for enterTxt

#include <stdio.h>
#include "log.h"

#ifdef __TEST__

#define CHK CHECK( ret >= 0, lblKO )

static code const char *srcFile = __FILE__;

void tctd0002(void){
    int ret;
    char buf[25+1];
    byte fmt='*';

    ret= dspStart();
    CHECK(ret>=0,lblKO);

    dspClear();
    ret= dspLS(0,"enterTxt");
    CHECK(ret>=0,lblKO);

    memset(buf,0,dspW+1); 
    ret= enterTxt(1,buf,16+1,fmt);
    prtS(buf);
    CHECK(ret>=0,lblKO);
    ret= dspLS(2,buf);
    CHECK(ret>=0,lblKO);    

    fmt=0xff;
    memset(buf,0,dspW+1);    
    dspClear();
    ret= dspLS(0,"enterTxt");
    ret= enterTxt(1,buf,9+1,fmt);
    prtS(buf);
    CHECK(ret>=0,lblKO);
    ret= dspLS(2,buf);
    CHECK(ret>=0,lblKO); 

    tmrPause(1);

    goto lblEnd;

lblKO:
    trcErr(ret);
    tmrPause(3);
lblEnd:
    trcFN("enterTxt: ret=%d\n",ret);
    dspStop();
}


#endif
