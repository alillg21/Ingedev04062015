#include <string.h>
#include "pri.h"
#include "tst.h"

#ifdef __OWF__
// Generating APACS 40 keys
// EncryptedPinBlock Message coming to the pinpad
// x02I800000020050000000279320050000000271200000000019F00x03x05

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

void tctt0016(void){
    int ret;
    byte crt[4]="";
    char tmp[prtW+1];
        byte bChksum[5];
        byte PinKeyLoc;
        byte MacKeyLoc;
        byte WKLoc;
        char ChkSum[5];
        byte *MacKey= (byte*) "\x3B\x7C\x13\xA4\x61\x52\x19\xA1\x3B\x7C\x13\xA4\x61\x52\x19\xA1";
        byte *ABCD= (byte*) "\x79\x32\x00\x50\x00\x00\x00\x27\x12\x00\x00\x00\x00\x01\x9F\x00";
        byte bPanBlk[16/2+1];
        byte *PinKey= (byte*) "\xA7\xEF\x62\x46\xE0\x0B\x2F\x43\xA7\xEF\x62\x46\xE0\x0B\x2F\x43";
        memset(tmp,0,prtW+1);
        memset(crt,0,4);

        tctt0015();//inject Master and Working Key(APACS 40)

        WKLoc=0x36;
        // Location for generate keys
        PinKeyLoc=0x40; // => Double Pin Key         
        MacKeyLoc=0x42; // => Double Mac Key 
        memcpy(bPanBlk,"\x00\x00\x20\x05\x00\x00\x00\x02",8); 

        ret= cryStart('m');
        ret= a40GenA40Keys(ABCD,PinKeyLoc,MacKeyLoc,WKLoc); CHECK(ret>=0,lblKO);  // Change by bkassovic

        ret= cryVerify(PinKeyLoc,crt); CHECK(ret>=0,lblKO);
        getKCV(PinKey,ChkSum);
        ret= hex2bin(bChksum,ChkSum,2); CHECK(ret==2,lblKO); 
        CHECK(memcmp(crt,bChksum,2)==0,lblKO);  
        ret= dspLS(1,"PinKey loaded");

        ret= cryVerify(MacKeyLoc,crt); CHECK(ret>=0,lblKO);     
        getKCV(MacKey,ChkSum);
        ret= hex2bin(bChksum,ChkSum,2); CHECK(ret==2,lblKO); 
        CHECK(memcmp(crt,bChksum,2)==0,lblKO);  
        ret= dspLS(2,"MacKey loaded");

    
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
void tctt0016(void) {}
#endif
