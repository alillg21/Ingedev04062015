#include <string.h>
#include "pri.h"
#include "tst.h"

//Unitary test case tcab0024.
//  fmtAmt


//to do: testing the limits and invalid data


static int testAmt(void){
    int ret;
    char buf[prtW+1];

    ret= dspClear(); CHECK(ret>=0,lblKO);
    ret= dspLS(0,"fmtAmt"); CHECK(ret>=0,lblKO);

    ret= fmtAmt(buf,"-123",2,0); //This works fine
    CHECK(strcmp(buf,"-1.23")==0,lblKO);
    ret= dspLS(1,buf); CHECK(ret>=0,lblKO);
    tmrPause(1);

    ret= fmtAmt(buf,"-1",2,0); // This has problem 0.-1
    CHECK(strcmp(buf,"-0.01")==0,lblKO);
    ret= dspLS(2,buf); CHECK(ret>=0,lblKO);
    tmrPause(1);

    ret= 1;
    goto lblEnd;
lblKO:
    trcErr(ret);
    dspLS(0,"KO");
    ret= -1;
lblEnd:
    tmrPause(1);
    return ret;
}


void tcmt0024(void){
    int ret;

    ret= dspStart(); CHECK(ret>=0,lblKO);
    ret= testAmt(); CHECK(ret>=0,lblKO);

    ret= 1;
    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO");
lblEnd:
    tmrPause(3);
    dspStop(); //close resource
}
