#include <stdio.h>
#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tctt0003.
//Parsing response of Parameter Download (Operator Information)

#define CHK CHECK(ret>=0,lblKO)
static int prepareBlock1(tBuffer *rsp){
    int ret;

        ret= bufApp(rsp,(byte *)"00",0); CHK; //BlockNo
        ret= bufApp(rsp,(byte *)"416D6972202020202020",0); CHK; //OperName1
        ret= bufApp(rsp,(byte *)"343536202020",0); CHK; //OperPass1
        ret= bufApp(rsp,(byte *)"05000100",0); CHK; //OperPer1
        
        ret= bufApp(rsp,(byte *)"496D72616E2020202020",0); CHK; //OperName2
        ret= bufApp(rsp,(byte *)"313233202020",0); CHK; //OperPass2
        ret= bufApp(rsp,(byte *)"83000000",0); CHK; //OperPer2
        ret= bufApp(rsp,(byte *)"0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",0); CHK; //FillZeroField
        
    return bufLen(rsp);
        
lblKO:
    trcErr(ret);
        return -1;

}


static int printDnl07(void){
    int ret;    
    char buf[256];
        card crd;

    ret= prtS("OperName:"); CHK;
    ret= mapGet(operName,buf,256); CHK; 
    ret= prtS(buf); CHK;
  
        ret= prtS("OperPassword:"); CHK;
    ret= mapGet(operPassword,buf,256); CHK;     
    ret= prtS(buf); CHK;

        ret= prtS("OperPermissions:"); CHK;
        ret= mapGetCard(operPermissions,crd); CHK;
        num2hex(buf,crd,2*sizeof(card));
        ret= prtS(buf); CHK;
        
        return 1;
lblKO:
    trcErr(ret);
        return -1;      
}


void tctt0003(void){
    int ret;
    byte dRsp[256];
    tBuffer bRsp;
        byte buf[256+1];

    ret= dspClear(); CHECK(ret>=0,lblKO);
    
        ret= dspLS(0,"prepareBlock1..."); CHECK(ret>=0,lblKO);    
    bufInit(&bRsp,dRsp,256);
        ret= prepareBlock1(&bRsp); CHECK(ret>=0,lblKO);     

        ret= hex2bin(buf,(char *)bufPtr(&bRsp),(byte)bufLen(&bRsp));  

    ret= dspLS(1,"parse01..."); CHECK(ret>=0,lblKO);    
        ret= rspDnl07(buf,256); CHECK(ret>=0,lblKO);

    ret= printDnl07(); CHECK(ret>=0,lblKO);

    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrStop(1);
   // tmrPause(3);
}
#endif

