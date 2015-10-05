#include <string.h>
#include "tst.h"
#include "log.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

// Test case tcrf0006
// Download and reading downladed data using UTA database

#define CHK CHECK(ret>=0,lblKO)
static int prepareData(void){
    int ret;
	//ret= mapPutWord(traMnuItm,mnuSale); CHK;
	ret= mapPut(traEntMod,"m",1); CHK;
	ret= mapPutStr(traTrk2,"5499990000000256=07129011244256600000"); CHK;
	ret= mapPutStr(traPan,"5499990000000256"); CHK;
	ret= mapPut(traExpDat,"0712",lenExpDat); CHK;
	ret= mapPutStr(traAmt,"234"); CHK;
	ret= mapPutWord(regMsgNum,1); CHK;
	//ret= mapPut(traDiaInd,"1",1); CHK;
	//ret= mapMove(issBeg+1,0); CHK;
	return 1;    
lblKO:
	trcErr(ret);
 return -1;
}

void tcrf0006(void){
    int ret;
    char *chn= "7E11200";
    char *srv= "0-00-97165739374";

    ret= prepareData(); CHECK(ret>=0,lblKO);

    ret= dspClear();
    CHECK(ret>=0,lblKO);

    ret= dspLS(0,"comConnect...");
    CHECK(ret>=0,lblKO);
    
    ret= comConnect(0,chn,srv);
    CHECK(ret>=0,lblKO);

    ret= dspLS(1,"Tle...");
    CHECK(ret>=0,lblKO);
    
    ret= tleAut(); //perform authorisation request and get response
    CHECK(ret>=0,lblKO);

    ret= dspLS(2,"Disconnect...");
    CHECK(ret>=0,lblKO);
    
    ret= comDisconnect(0,chn,srv);
    CHECK(ret>=0,lblKO); 

    tmrPause(3);
    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
    ret= comDisconnect(0,chn,srv);
lblEnd:
    tmrStop(1);
}

#endif
