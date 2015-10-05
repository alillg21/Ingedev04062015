#include <string.h>
#include "tst.h"
#include "log.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcrf0010.
//Online call testing.

#define CHK CHECK(ret>=0,lblKO)
static int prepare01(void){
    int ret;
    ret= mapPutWord(traMnuItm,mnuDownload); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
    //ret= mapPutStr(traTrk2,"5425364009408223=05121011126947400000"); CHK;
    //ret= mapPutStr(traPan,"5425364009408223"); CHK;
    //ret= mapPut(traExpDat,"0512",lenExpDat); CHK;
    //ret= mapPutStr(traAmt,"234"); CHK;
    ret= mapPutWord(regMsgNum,1); CHK;
    //ret= mapPut(traDiaInd,"1",1); CHK;
    //ret= mapMove(issBeg+1,0); CHK;
    //Phone number 
    //ret= mapMove(acqBeg+1,1); 
    //ret= mapPutStr(acqPriPhone,"5739374");  
    //ret= mapPutStr(acqSecPhone,"5739374");  

    return 1;    
lblKO:
    trcErr(ret);
 return -1;
}

static int printData(void){
    int ret;
    char buf[256];
	
	ret= prtS("RspCod:"); CHK;
    ret= mapGet(traRspCod,buf,256); CHK;
    ret= prtS(buf); CHK;

	ret= mapGet(rptSpaces,buf,256); CHK;
    ret= prtS(buf); CHK;

	ret= prtS("=> TrmLoc:"); CHK;
    ret= mapGet(appTrmLoc,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRIName:"); CHK;
    ret= mapGet(appSRIName,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRIAddr:"); CHK;
    ret= mapGet(appSRIAddr,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRICity:"); CHK;
    ret= mapGet(appSRICity,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRIState:"); CHK;
    ret= mapGet(appSRIState,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRICntry:"); CHK;
    ret= mapGet(appSRICntry,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> SRIPhone:"); CHK;
    ret= mapGet(appSRIPhone,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> PinPadChar:"); CHK;
    ret= mapGet(appPinPadChar,buf,256); CHK;
    ret= prtS(buf); CHK;
	ret= prtS("=> RetID:"); CHK;
    ret= mapGet(appRetID,buf,256); CHK;
    ret= prtS(buf); CHK;

	ret= mapGet(rptSpaces,buf,256); CHK;
    ret= prtS(buf); CHK;

    return 1;    
lblKO:
    trcErr(ret);
    return -1;
}

void tcrf0010(void){
    int ret;

    ret= dspClear(); CHECK(ret>=0,lblKO);

    ret= dspLS(0,"prepare01..."); CHECK(ret>=0,lblKO);    
    ret= prepare01(); CHECK(ret>=0,lblKO);
    
    ret= dspLS(1,"onlAut..."); CHECK(ret>=0,lblKO);    
    ret= onlAut(); CHECK(ret>=0,lblKO);
    ret= printData(); CHECK(ret>=0,lblKO);

    goto lblEnd;
    
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
}
#endif