#include <string.h>
#include "pri.h"
#include "tst.h"

// Obtaining Pin block for  APACS 40 Pinpad message
//EncryptedPinBlock Message coming to the pinpad
//x02I800000020050000000279320050000000271200000000019F00x03x05

void tctt0017(void){
    int ret;
    byte crt[4]="";
    char tmp[prtW+1];
        byte PinKeyLoc;
        byte WKLoc;
        byte bPanBlk[16/2+1];
        byte PinBlk[8+1]="";
        byte cPinBlk[8+1];

        memset(tmp,0,prtW+1);
    memset(crt,0,4);

        tctt0016();//GenerateAPACS 40 keys

        WKLoc=0x36;
        PinKeyLoc=0x40;

        memcpy(bPanBlk,"\x00\x00\x20\x05\x00\x00\x00\x02",8); 
    ret= cryStart('m');
        //Generate Pin block
        dspStop(); 
        ret= cryGetPin("\n PIN:\n\n",bPanBlk,PinKeyLoc,PinBlk); 
        ret= dspStart();
    
        
        memcpy(cPinBlk,"\xF4\x85\xC7\xD4\x19\xF3\xA3\x4D",8);
        CHECK(memcmp(PinBlk,cPinBlk,8)==0,lblKO);  
        ret= dspLS(2,"PinBlock Fine");
 

    
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
