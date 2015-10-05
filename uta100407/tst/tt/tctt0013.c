#include <string.h>
#include "log.h"
#include "tst.h"

static code const char *srcFile= __FILE__;


// Calculating RequestMAC and RequestMACResidue for  APACS 40 Pinpad message
//Generate RequestMac Message coming to the pinpad
//x02Q800554960098659466510100000939000001800007660001322211150240010x1C;
//5549600986594665x3D03121010000093900000?x1C444x1CPOS223-SEQ000001x1C00x1Cx1Cx03x7E


#define CHK CHECK(ret>=0,lblKO)
static int prepareData(tBuffer *req){
    int ret;
        byte index;
        

        index='8';
        ret= mapPutByte(traStartKeyIndex,index); CHK;
        ret= mapPut(traFieldA,"55496009",lenPanBlk+1); CHK;
    ret= mapPut(traFieldB,"86594665",lenField+1); CHK;
        ret= mapPut(traFieldC,"10100000",lenField+1); CHK;
        ret= mapPut(traFieldD,"93900000",lenField+1); CHK;
        ret= mapPutByte(traMsgType,'Q'); CHK;

        ret= bufApp(req,(byte *)"1800007660001322211150240010",0);CHK;
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)";5549600986594665",0); CHK;
        ret= bufApp(req,(byte *)"\x3D",1); CHK;
        ret= bufApp(req,(byte *)"03121010000093900000?",0); CHK;
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)"444",0); CHK; 
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)"POS223-SEQ000001",0); CHK; 
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)"00",0);CHK;
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)"\x1C",1); CHK;

//      ret= mapPut(traReqPacket,"1800007660001322211150240010x1C;5549600986594665x3D03121010000093900000?x1C444x1CPOS223-SEQ000001x1C00x1Cx1C",lenReqPacket+1); CHK;
        
   ret=mapPut(traReqPacket,req->ptr,lenReqPacket+1);

    return 1;    
lblKO:
    trcErr(ret);
 return -1;
}

void tctt0013(void){
    int ret;
    byte crt[4]="";
    char tmp[prtW+1]="";
        byte bChksum[5]="";
        tBuffer req;
        byte dReq[256];

        byte MacKey[(lenMK/2)+1] ="";
        byte MacLoc;
        byte ReqMacResidue[lenReqMacResidue+1]="";
        byte ReqMAC[lenReqMAC+1]="";
        byte CReqMAC[lenReqMAC+1]="";
        byte CReqMacResidue[lenReqMacResidue+1]="";

        char ChkSum[5];
        
        tctt0011();

    memset(tmp,0,prtW+1);
    memset(crt,0,4);

        ret= dspLS(0,"prepareData...");
        bufInit(&req,dReq,256);
    ret= prepareData(&req);CHECK(ret>=0,lblKO); 
    tmrPause(1);

        
    
        ret = processMsg(); CHECK(ret>=0,lblKO);
    
    CHECK(ret>=0,lblKO); 
        
        cryStart();

        
        MacLoc =0x42;
    ret= cryVerify(MacLoc,crt); CHECK(ret>=0,lblKO);

    
        memcpy(MacKey,"\xC8\xC1\xE5\x1F\xA4\xCE\xA7\x7C",8);
        memcpy(MacKey+8,"\xC8\xC1\xE5\x1F\xA4\xCE\xA7\x7C",8);
        getKCV(MacKey,ChkSum);

        ret= hex2bin(bChksum,ChkSum,2); CHECK(ret==2,lblKO); 
        CHECK(memcmp(crt,bChksum,2)==0,lblKO);  
        ret= dspLS(1,"MacKey loaded");


        memcpy(CReqMAC,"AC6F9C36",8);
        memcpy(CReqMacResidue,"\x0C\x48\xDE\x50",4);
        ret= mapGet(traReqMAC,ReqMAC,lenReqMAC);   CHECK(ret>=0,lblKO);
    ret= mapGet(traReqMacResidue,ReqMacResidue,lenReqMacResidue);   CHECK(ret>=0,lblKO);
        CHECK(memcmp(ReqMAC,CReqMAC,8)==0,lblKO);  
        CHECK(memcmp(ReqMacResidue,CReqMacResidue,4)==0,lblKO);  
        ret= dspLS(2,"MacGen fine");
    goto lblEnd;
        
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop();
    prtStop();
    dspStop();
}
