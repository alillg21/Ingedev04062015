#include <string.h>
#include "log.h"
#include "tst.h"

static code const char *srcFile= __FILE__;


// Injection of  MasterKey 0123456789ABDCDEF into loc 12
//for Apacs 40 pinpad message
//MasterKey Injection Message coming to the pinpad
//x025M000123456789ABCDEFx03x7d


#define CHK CHECK(ret>=0,lblKO)
static int prepareData(void){
    int ret;

        byte loc;
        loc=0x12;
        ret= mapPut(traMK,"\x01\x23\x45\x67\x89\xAB\xCD\xEF\x01\x23\x45\x67\x89\xAB\xCD\xEF",16); CHK;
        ret= mapPutByte(appMKLoc,loc); CHK;
        ret= mapPutByte(traMsgType,'5'); CHK;
        ret= mapPutByte(traSubMsgType,'M'); CHK;

    return 1;    
lblKO:
    trcErr(ret);
 return -1;
}

void tctt0010(void){
    int ret;
    byte crt[4]="";
    char tmp[prtW+1];
        byte bChksum[5];

        byte MasterKey[(lenMK/2)+1] ="";
        byte MKloc;
        char ChkSum[5];

    memset(tmp,0,prtW+1);
    memset(crt,0,4);

        ret= dspLS(0,"prepareData...");
    ret= prepareData();CHECK(ret>=0,lblKO); 
    tmrPause(1);

        
    
        ret = processMsg(); CHECK(ret>=0,lblKO);
    
    CHECK(ret>=0,lblKO); 
        ret= mapGetByte(appMKLoc,MKloc); CHK;
        cryStart('m');
    ret= cryVerify(MKloc,crt); CHECK(ret>=0,lblKO);

    //present results
    ret= bin2hex(tmp,&MKloc,1); CHECK(ret==2,lblKO);        
    strcat(tmp,": "); VERIFY(strlen(tmp)==4);    
    ret= bin2hex(tmp+4,crt,4); CHECK(ret==8,lblKO);        
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
    ret= dspLS(1,tmp); CHECK(ret>=0,lblKO);

        mapGet(traMK,MasterKey,16); 
        getKCV(MasterKey,ChkSum);

        ret= hex2bin(bChksum,ChkSum,2); CHECK(ret==2,lblKO); 
        CHECK(memcmp(crt,bChksum,2)==0,lblKO);  //it is the certificate of the key 0123456789ABCDEF
        ret= dspLS(1,"MK loaded");
    goto lblEnd;
        
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop('m');
    prtStop();
    dspStop();
}
