#include <string.h>
#include "pri.h"
#include "tst.h"

#ifdef __OWF__
// Calculating RequestMAC and RequestMACResidue for  APACS 40 Pinpad message
//Generate RequestMac Message coming to the pinpad
//x02Q800554960098659466510100000939000001800007660001322211150240010x1C;
//5549600986594665x3D03121010000093900000?x1C444x1CPOS223-SEQ000001x1C00x1Cx1Cx03x7E

static void getKCV(byte *key,char *chk){ //calculate checksum
        byte dst[8];
        byte src[8];
        char buf[16+1]="";
        
        memset(chk,0,8+1);
        memset(src,0,8);
        stdEDE(dst,src,key);
        bin2hex((char *)buf,dst,8);
        memcpy(chk,buf,8);
}

void tctt0018(void){
    int ret;
    byte crt[4];
    byte bChksum[5];
    byte PinKeyLoc;
    byte MacKeyLoc;
    byte WKLoc;
    byte *MacKey= (byte*) "\xC8\xC1\xE5\x1F\xA4\xCE\xA7\x7C\xC8\xC1\xE5\x1F\xA4\xCE\xA7\x7C";
    byte *ABCD= (byte*) "\x55\x49\x60\x09\x86\x59\x46\x65\x10\x10\x00\x00\x93\x90\x00\x00";
    byte ReqMacResidue[4];
    char ChkSum[8+1];
    char ReqMAC[8+1];
    char tmp[prtW+1];
    tBuffer req;
    byte dReq[256];
    byte buf[8];

    memset(tmp,0,prtW+1);
    memset(crt,0,4);
    tctt0015();
    bufInit(&req,dReq,256);
    WKLoc=0x36;
    PinKeyLoc=0x40;
    MacKeyLoc=0x42;

    ret= bufApp(&req,(byte *)"1800007660001322211150240010",0);CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)";5549600986594665",0); CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)"\x3D",1); CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)"03121010000093900000?",0); CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)"444",0); CHECK(ret>=0,lblKO); 
    ret= bufApp(&req,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)"POS223-SEQ000001",0); CHECK(ret>=0,lblKO); 
    ret= bufApp(&req,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)"00",0);CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);
    ret= bufApp(&req,(byte *)"\x1C",1); CHECK(ret>=0,lblKO);

    //Generate Apacs 40Keys
    //not used testcase tctt0016 for generating 40 keys cause the abcd is different
    //therefore generating it locally here
    ret= cryStart('m');
    ret= a40GenA40Keys(ABCD,PinKeyLoc,MacKeyLoc,WKLoc); CHECK(ret>=0,lblKO); // Change by bkassovic
        //cryGenerateA40Keys(ABCD,ABCD+4,ABCD+8,ABCD+12,PinKeyLoc,MacKeyLoc,WKLoc);
    ret= cryVerify(MacKeyLoc,crt); CHECK(ret>=0,lblKO);
    getKCV(MacKey,ChkSum);

    ret= hex2bin(bChksum,ChkSum,2); CHECK(ret==2,lblKO); 
    CHECK(memcmp(crt,bChksum,2)==0,lblKO);  
    ret= dspLS(1,"MacKey loaded");

    //Generate MAC
    while(bufLen(&req)%8){
        ret=bufApp(&req,(byte *)"\x00",1);
    } 
    ret= cryMac(&req,MacKeyLoc,buf); CHECK(ret>=0,lblKO);
        memcpy(ReqMacResidue,buf+4,4);
        bin2hex(ReqMAC,buf,4);        
    CHECK(memcmp(ReqMAC,"AC6F9C36",8)==0,lblKO);  
    CHECK(memcmp(ReqMacResidue,"\x0C\x48\xDE\x50",4)==0,lblKO);  
    ret= dspLS(2,"MacGen fine");   
        
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
#else
void tctt0018(void) {}
#endif
