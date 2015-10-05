#include <string.h>
#include "log.h"
#include "tst.h"

static code const char *srcFile= __FILE__;


// Injection of  Working key int he message into specified loc derived from index
//coming in the message
//for Apacs 40 pinpad message
//EncryptedStartKey Injection Message coming to the pinpad
//x025A8092EA00B4F3CE9C3Dx03x78


#define CHK CHECK(ret>=0,lblKO)
static int prepareData(void){
    int ret;
        byte index;
        byte MKloc;   
        byte WK[(lenMK/2)+1]="";      
        

        MKloc=0x12;
        index='8';
        ret= mapPutByte(traStartKeyIndex,index); CHK;
    ret= mapPutByte(appMKLoc,MKloc); CHK;
        memcpy(WK,"\x92\xEA\x00\xB4\xF3\xCE\x9C\x3D\x92\xEA\x00\xB4\xF3\xCE\x9C\x3D",16);
        ret= mapPut(traWK,WK,(lenWK/2)+1); CHK; 
        ret= mapPutByte(traMsgType,'5'); CHK;
        ret= mapPutByte(traSubMsgType,'A'); CHK;
   

    return 1;    
lblKO:
    trcErr(ret);
 return -1;
}

void tctt0011(void){
    int ret;
    byte crt[4]="";
    char tmp[prtW+1]="";
        byte bChksum[5]="";

        byte WorkingKey[(lenMK/2)+1] ="";
        byte WKLoc;
    byte index;
        char ChkSum[5];

        tctt0010();

    memset(tmp,0,prtW+1);
    memset(crt,0,4);
        
        
        ret= dspLS(0,"prepareData...");
    ret= prepareData();CHECK(ret>=0,lblKO); 
    tmrPause(1);

        
    
        ret = processMsg(); CHECK(ret>=0,lblKO);
    
    CHECK(ret>=0,lblKO); 
        
        cryStart();

        ret= mapGetByte(traStartKeyIndex,index); CHECK(ret>=0,lblKO);
    ret= getLocationFromStartKeyIndex(index,&WKLoc);

    ret= cryVerify(WKLoc,crt); CHECK(ret>=0,lblKO);

    //present results
    ret= bin2hex(tmp,&WKLoc,1); CHECK(ret==2,lblKO);        
    strcat(tmp,": "); VERIFY(strlen(tmp)==4);    
    ret= bin2hex(tmp+4,crt,4); CHECK(ret==8,lblKO);        
    ret= dspLS(1,tmp); CHECK(ret>=0,lblKO);

        memcpy(WorkingKey,"\x2A\x3E\x4C\x6B\x7C\x3B\x2F\x6B",8);
        memcpy(WorkingKey+8,"\x2A\x3E\x4C\x6B\x7C\x3B\x2F\x6B",8);
        getKCV(WorkingKey,ChkSum);

        ret= hex2bin(bChksum,ChkSum,2); CHECK(ret==2,lblKO); 
        CHECK(memcmp(crt,bChksum,2)==0,lblKO);  //it is the certificate of the key 2A3E4C6B7C3B2F6B
        ret= dspLS(1,"WK loaded");
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
