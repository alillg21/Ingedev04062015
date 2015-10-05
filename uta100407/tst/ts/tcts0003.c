#include "pri.h"
#include "tst.h"
#include <string.h>

//Unitary test case tcts0003.
//Functions testing:
//  enterStr2
//  enterPwd2

void tcts0003(void){
    int ret;
    char buf[20+1];
    
    ret= dspStart();
    CHECK(ret>=0,lblKO);

    ret= dspLS(0,"enterStr2");
    CHECK(ret>=0,lblKO);

    memset(buf,0,20+1);
	strcpy(buf,"Enter Id:");
    ret= enterStr2(1,buf,20+1);
    CHECK(ret>=0,lblKO);
    
    ret= dspLS(2,buf+strlen("Enter Id:"));
    CHECK(ret>=0,lblKO);    
    
    tmrPause(3);

    dspClear();
    ret= dspLS(0,"enterPwd2");
    CHECK(ret>=0,lblKO);

    memset(buf,0,dspW+1);    
	strcpy(buf,"PWD: ");
    ret= enterPwd2(1,buf,dspW+1);
    CHECK(ret>=0,lblKO);
    ret= dspLS(2,buf+strlen("PWD: "));
    CHECK(ret>=0,lblKO);    

    tmrPause(3);

    goto lblEnd;
    
lblKO:
    trcErr(ret);
    dspLS(0,"KO");
    tmrPause(3);
lblEnd:
    dspStop(); //close resource
}
