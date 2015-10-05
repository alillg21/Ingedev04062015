#include <string.h>
#include "log.h"
#include "tst.h"

static code const char *srcFile= __FILE__;


// Verify  valid response Mac and updating the key register if the Reponse
// Mac matches the calculated Mac for   APACS 40 Pinpad message
//Verify ResponseMac Message coming to the pinpad
//x02S80083444FFFFFFFFFFFB37AB341x78x78x78x78180000766000113000002759x1C00000013320x1CAUTHx20COD
//Ex3D002759x1Cx1Cx1Cx1Cx03x44


#define CHK CHECK(ret>=0,lblKO)
static int prepareData(tBuffer *req){
    int ret;
        byte index;

        index='8';
        ret = mapPutByte(traStartKeyIndex,index); CHK;
    ret = mapPut(traResMAC,"B37AB341",lenResMAC+1); CHK;

        ret= bufApp(req,(byte *)"180000766000113000002759",0);CHK;
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)"00000013320",0); CHK;
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)"AUTH",0); CHK;
        ret= bufApp(req,(byte *)"\x20",1); CHK;
        ret= bufApp(req,(byte *)"CODE",0); CHK; 
        ret= bufApp(req,(byte *)"\x3D",1); CHK;
        ret= bufApp(req,(byte *)"002759",0); CHK; 
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret= bufApp(req,(byte *)"\x1C",1); CHK;
        ret=mapPut(traRspPacket,req->ptr,lenRspPacket+1 );
        //ret = mapPut(traRspPacket,"180000766000113000002759x1C00000013320x1CAUTHx20CODEx3D002759x1Cx1Cx1Cx1C",lenRspPacket+1); CHK;
        ret = mapPut(traAmtBlk,"83444FFFFFFFFFFF",lenAmtBlk+1);CHK;

        ret = mapPut(traReqMacResidue,"\x0C\x48\xDE\x50",lenReqMacResidue+1); CHK;
        ret= mapPutByte(traMsgType,'S'); CHK;
   

    return 1;    
lblKO:
    trcErr(ret);
 return -1;
}

void tctt0014(void){
    int ret;
    byte crt[4]="";
    char tmp[prtW+1]="";
        byte bChksum[5]="";
        tBuffer req;
        byte dReq[256];

        byte WorkingKey[(lenMK/2)+1] ="";
        byte WKLoc;
        byte ResMacResidue[lenResMacResidue+1]="";
        byte CResMacResidue[lenResMacResidue+1]="";

        char ChkSum[5];

        tctt0013();

    memset(tmp,0,prtW+1);
    memset(crt,0,4);

        ret= dspLS(0,"prepareData...");
        bufInit(&req,dReq,256);
    ret= prepareData(&req); CHECK(ret>=0,lblKO); 
    tmrPause(1);

        
    
        ret = processMsg(); CHECK(ret>=0,lblKO);
    
    CHECK(ret>=0,lblKO); 
        

        memcpy(CResMacResidue,"\xC2\x45\x21\x12",4);
    ret= mapGet(traResMacResidue,ResMacResidue,lenResMacResidue);CHK; 
        CHECK(memcmp(ResMacResidue,CResMacResidue,4)==0,lblKO);  

        cryStart();
        
        WKLoc =0x36;
    ret= cryVerify(WKLoc,crt); CHECK(ret>=0,lblKO);

    
        memcpy(WorkingKey,"\xC8\x8C\x54\xFB\x57\x2A\x8C\xF2",8);
        memcpy(WorkingKey+8,"\xC8\x8C\x54\xFB\x57\x2A\x8C\xF2",8);
        getKCV(WorkingKey,ChkSum);

        ret= hex2bin(bChksum,ChkSum,2); CHECK(ret==2,lblKO); 
        CHECK(memcmp(crt,bChksum,2)==0,lblKO);  
        ret= dspLS(1,"KeyReg Updated");

        
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
