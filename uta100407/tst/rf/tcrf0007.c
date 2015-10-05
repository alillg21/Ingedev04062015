#include <string.h>
#include "tst.h"
#include "log.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcrf0007.
//Parsing response.

#define CHK CHECK(ret>=0,lblKO)
static int prepare01(tBuffer *rsp){
    int ret;
    byte fs;
    fs= cFS; 
    
    ret= bufApp(rsp,(byte *)"1",0); CHK; //Dial Indicator
    ret= bufApp(rsp,(byte *)"12345678",0); CHK; //Terminal identity
    ret= bufApp(rsp,(byte *)"0004",0); CHK; //Message number
    ret= bufApp(rsp,(byte *)"12",0); CHK; //Message type identification
    ret= bufApp(rsp,(byte *)"00",0); CHK; //Acquirer response code
    ret= bufApp(rsp,(byte *)"0",0); CHK; //Confirmation request
    ret= bufApp(rsp,(byte *)"002423",0); CHK; //Authorisation Code
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,(byte *)"1000",0); CHK; //Amount
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,(byte *)"APPROVED 002423",0); CHK; //Message
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,&fs,1); CHK; //FS
    ret= bufApp(rsp,&fs,1); CHK; //FS

    return bufLen(rsp);
lblKO:
    trcErr(ret);
	return -1;
}

static int printRsp(void){
    int ret;
    char buf[256];

    ret= prtS("RspCod:"); CHK;
    ret= mapGet(traRspCod,buf,256); CHK;
    ret= prtS(buf); CHK;

    /*ret= prtS("CfmReq:"); CHK;
    ret= mapGet(traCfmReq,buf,256); CHK;
    ret= prtS(buf); CHK;

    ret= prtS("AutCod:"); CHK;
    ret= mapGet(traAutCod,buf,256); CHK;
    ret= prtS(buf); CHK;

    ret= prtS("SrvMsg:"); CHK;
    ret= mapGet(traSrvMsg,buf,256); CHK;
    ret= prtS(buf); CHK;*/

    return 1;
lblKO:
    trcErr(ret);
	return -1;
}

void tcrf0007(void){
    int ret;
    byte dRsp[256];
    tBuffer bRsp;

    ret= dspClear(); CHECK(ret>=0,lblKO);
    ret= dspLS(0,"prepare01..."); CHECK(ret>=0,lblKO);    
    bufInit(&bRsp,dRsp,256);
    ret= prepare01(&bRsp); CHECK(ret>=0,lblKO);
    
    ret= dspLS(1,"parse01..."); CHECK(ret>=0,lblKO);    
    ret= rspAut(bufPtr(&bRsp),bufLen(&bRsp)); CHECK(ret>=0,lblKO);

    ret= printRsp(); CHECK(ret>=0,lblKO);

    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrStop(1);
    tmrPause(3);
}
#endif

