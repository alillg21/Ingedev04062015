#include <string.h>
#include "log.h"
#include "tst.h"

static code const char *srcFile= __FILE__;


// Obtaining Pin block for  APACS 40 Pinpad message
//EncryptedPinBlock Message coming to the pinpad
//x02I800000020050000000279320050000000271200000000019F00x03x05

#define CHK CHECK(ret>=0,lblKO)
static int prepareData(void){
    int ret;
        byte index;

        index='8';
        ret= mapPutByte(traStartKeyIndex,index); CHK;
        ret= mapPut(traFieldA,"79320050",lenPanBlk+1); CHK;
    ret= mapPut(traFieldB,"00000027",lenField+1); CHK;
        ret= mapPut(traFieldC,"12000000",lenField+1); CHK;
        ret= mapPut(traFieldD,"00019F00",lenField+1); CHK;
        ret= mapPut(traPanBlk,"0000200500000002",lenPanBlk+1); CHK;
        ret= mapPutByte(traMsgType,'I'); CHK;
   

    return 1;    
lblKO:
    trcErr(ret);
 return -1;
}

void tctt0012(void){
    int ret;
    byte crt[4]="";
    char tmp[prtW+1]="";
        byte bChksum[5]="";

        byte PinKey[(lenMK/2)+1] ="";
        byte PinLoc;
        byte PinBlk[lenPinBlk];
        byte cPinBlk[lenPinBlk];
        char ChkSum[5];

        tctt0011();

    memset(tmp,0,prtW+1);
    memset(crt,0,4);

        ret= dspLS(0,"prepareData...");
    ret= prepareData();CHECK(ret>=0,lblKO);  
    tmrPause(1);

        
    
        ret = processMsg(); CHECK(ret>=0,lblKO);
    
    CHECK(ret>=0,lblKO); 
        
        cryStart();

        
        PinLoc =0x40;
    ret= cryVerify(PinLoc,crt); CHECK(ret>=0,lblKO);

    
        memcpy(PinKey,"\xA7\xEF\x62\x46\xE0\x0B\x2F\x43",8);
        memcpy(PinKey+8,"\xA7\xEF\x62\x46\xE0\x0B\x2F\x43",8);
        getKCV(PinKey,ChkSum);

        ret= hex2bin(bChksum,ChkSum,2); CHECK(ret==2,lblKO); 
        CHECK(memcmp(crt,bChksum,2)==0,lblKO);  
        ret= dspLS(1,"PinKey loaded");


        memcpy(cPinBlk,"\xF4\x85\xC7\xD4\x19\xF3\xA3\x4D",8);
        ret= mapGet(traPinBlk,PinBlk,lenPinBlk); CHECK(ret>=0,lblKO);
        CHECK(memcmp(PinBlk,cPinBlk,8)==0,lblKO);  
        ret= dspLS(2,"PinBlock Fine");


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
