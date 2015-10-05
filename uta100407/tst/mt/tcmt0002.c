#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tcmt0002.
//Parsing MDC response.

#define CHK CHECK(ret>=0,lblKO)
// Test for Message
static int prepare01(tBuffer *rsp){
    int ret;
    byte fs;
    fs= cFS; 
    
    ret= bufApp(rsp,(byte *)"5",0); CHK; //System Code
    ret= bufApp(rsp,(byte *)"A",0); CHK; //Record Type
    ret= bufApp(rsp,(byte *)"GB",0); CHK; //Batch Response Code
        ret= bufApp(rsp,(byte *)" SUCCESSFUL           # 0628103",30); //Message

    return bufLen(rsp);
lblKO:
    trcErr(ret);
        return -1;
}

// Test for Custom Message
/*
static int prepare02(tBuffer *rsp){
    int ret;
    byte fs;
    fs= cFS; 
    
    ret= bufApp(rsp,(byte *)"5",0); CHK; //System Code
    ret= bufApp(rsp,(byte *)"A",0); CHK; //Record Type
    ret= bufApp(rsp,(byte *)"GB",0); CHK; //Batch Response Code
    ret= bufApp(rsp,(byte *)"000",0); CHK; //Batch ID
        ret= bufApp(rsp,(byte *)"SUCCESSFUL       ",17); //Message
    ret= bufApp(rsp,(byte *)"99999999",0); CHK; //Terminal ID
        ret= bufApp(rsp,(byte *)"f",0); CHK; //Filler
        ret= bufApp(rsp,(byte *)"X",0); CHK; //RPDL Flag

    return bufLen(rsp);
lblKO:
    trcErr(ret);
        return -1;
}
*/
static int printRsp(void){
    int ret;
    char buf[256];

    ret= prtS("RspCod:"); CHK;
    ret= mapGet(traRspCod,buf,256); CHK;
    ret= prtS(buf); CHK;

    ret= prtS("Message:"); CHK;
    ret= mapGet(batMsg,buf,256); CHK;
    ret= prtS(buf); CHK;

   /*
        ret= prtS("RPDL Flag:"); CHK;
    ret= mapGet(batRPDLFlag,buf,256); CHK;
    ret= prtS(buf); CHK;
        */
    
        return 1;
lblKO:
    trcErr(ret);
        return -1;
}

void tcmt0002(void){
    int ret;
    byte dRsp[256];
    tBuffer bRsp;

    ret= dspClear(); CHECK(ret>=0,lblKO);
    ret= dspLS(0,"prepare01..."); CHECK(ret>=0,lblKO);    
    bufInit(&bRsp,dRsp,256);
    ret= prepare01(&bRsp); CHECK(ret>=0,lblKO);
    
    ret= dspLS(1,"parse01..."); CHECK(ret>=0,lblKO);    
    ret= rspMdc(bufPtr(&bRsp),bufLen(&bRsp)); CHECK(ret>=0,lblKO);

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

