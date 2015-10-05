#include <string.h>
#include "log.h"
#include "tst.h"

#ifdef __TEST__
static code const char *srcFile= __FILE__;

//Integration test case tctt0001.
//Building the request message for Daily download.

#define CHK CHECK(ret>=0,lblKO)
static int prepare01(void){
        return 1;    
}

static int printReqDnlDaily(const char *hdr,const char *req,word len){
    int ret;
    char tmp[prtW+1];
    
    ret= prtS(hdr); CHK;
    tmp[prtW]= 0;
    while(len){
       if(strlen(req)<=prtW){
            strcpy(tmp,req);
            len= 0;
       }else{
            memcpy(tmp,req,prtW);
            len-= prtW;
            req+= prtW;
       }
       ret= prtS(tmp); CHK;       
    }
    return 1;
lblKO:
    trcErr(ret);
        return -1;
}

void tctt0001(void){
    int ret;
    byte dReq[256];
    tBuffer bReq;

    ret= dspClear(); CHECK(ret>=0,lblKO);
    ret= dspLS(0,"prepare01..."); CHECK(ret>=0,lblKO);    

    ret= prepare01(); CHECK(ret>=0,lblKO);

    ret= dspLS(1,"build01..."); CHECK(ret>=0,lblKO);    
    bufInit(&bReq,dReq,256);
    ret= reqDnlDaily(&bReq); CHECK(ret>=0,lblKO);

    trcFS("req=[%s]\n",(char *)bufPtr(&bReq));
    ret= printReqDnlDaily("  request01",(char *)bufPtr(&bReq),bufLen(&bReq)); CHECK(ret>=0,lblKO);
    
    tmrPause(3);
    goto lblEnd;

lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
    tmrPause(3);
lblEnd:
    tmrStop(1);
}
#endif
