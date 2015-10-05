#include <string.h>
#include "tst.h"
#include "log.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcrf0009.
//Connection.
//Building the request.
//Launching transport layer engine.
//Parsing response.
//Disconnection.

#define CHK CHECK(ret>=0,lblKO)

static int prepareData(void){
    int ret;
    ret= mapPutWord(traMnuItm,mnuDownload); CHK;
    ret= mapPut(traEntMod,"m",1); CHK;
	//ret= mapPutStr(traTrk2,"5499990000000256=07129011244256600000"); CHK;
	//ret= mapPutStr(traPan,"5499990000000256"); CHK;
	//ret= mapPut(traExpDat,"0712",lenExpDat); CHK;
	//ret= mapPutStr(traAmt,"234"); CHK;
	
	ret= mapPutWord(regMsgNum,1); CHK;
	return 1;    
lblKO:
    trcErr(ret);
 return -1;
}

static int padDialog(void){
	int ret;
	char *ptr;
    tBuffer bPat;
    byte dPat[16];
    tBuffer bRsp;
    byte dRsp[256];
    byte trm[]= {'*',0};
    
	//The constant string parameters should be retrieved from database, not hardcoded

	//wait for *
	ret= comRecvRsp(0,trm,60);
    CHECK(ret>0,lblKO); 

	//send Nui+X25
	ptr= "N123456789123456789-12121212306\x0D\x0A";
    ret= comSendReq((byte *)ptr,(word)strlen(ptr));
    CHECK(ret>0,lblKO); 

	//wait for COM
    bufInit(&bRsp,dRsp,256);
    bufInit(&bPat,dPat,16);
    bufApp(&bPat,(byte *)"COM\x0D\x0A",5);

    ret= comRecvPat(&bRsp,&bPat,5); //wait for pattern
    CHECK(ret>0,lblKO); //0 means timeout, it is also failure

	ret= 1;
	goto lblEnd;
lblKO:
    ret= -1;
    goto lblEnd;
lblEnd:
    return ret;
}

void tcrf0009(void){
	int ret;
	char *chn= "7E11200";
	char *srv= "0-4212700";

	ret= prepareData(); CHECK(ret>=0,lblKO);

	ret= dspClear();
	CHECK(ret>=0,lblKO);

	ret= dspLS(0,"comConnect...");
	CHECK(ret>=0,lblKO);
    
	ret= comConnect(0,chn,srv);
	CHECK(ret>=0,lblKO);

	ret= padDialog();
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
