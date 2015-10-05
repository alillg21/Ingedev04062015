/** \file
 * Unitary test case tcbk0001.
 * Functions testing:
 * \sa
 *  - cryDukptInit()
 *  - cryDukptPinInput()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __DUKPT__

void tcbk0001(void){
    int ret;
    byte crt[4];
    char tmp[prtW+1];
    byte acc[8];
    byte blk[8];
    byte loc;
    byte InitSNKey[20];     // InitSNKey = KET SET ID (FFFF) + TRMS ID (9876543210) + TRANSACTION COUNTER (E00000)
    byte SNKey[10];         // SNKey return at pin = KET SET ID + TRMS ID + TRANSACTION COUNTER

    memset(tmp,0,prtW+1);
    memset(crt,0,4);

    tcbk0000(); //test and working keys downloading
    
    //open ressources
    ret= prtStart(); CHECK(ret>=0,lblKO);
    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= cryStart('m'); CHECK(ret>=0,lblKO);

    //verify that the working key 6AC292FAA1315B4D is downloaded at array 2 slot 4
    loc= 0x24;
    ret= cryVerify(loc,crt);
    CHECK(ret>=0,lblKO);
    CHECK(memcmp(crt,"\xAB\x37\xCA\xB6",4)==0,lblKO);  //it is the certificate of the key 6AC292FAA1315B4D

    //start testing

    //*** Initialize DUKPT process ***
    memcpy(InitSNKey, "\x3F\x3F\x3F\x3F\x39\x38\x37\x36\x35\x34\x33\x32\x31\x30\x3E\x30\x30\x30\x30\x30", 20); // Padding with 0x30
    ret= cryDukptInit(loc,InitSNKey);
    CHECK(ret>=0,lblKO);

    ret= dspStop(); //close channel to give cryptomodule access to HMI
    CHECK(ret>=0,lblKO);

    //*** First Pin entry and get pinblock ***
    memcpy(acc,"\x00\x00\x40\x12\x34\x56\x78\x90",8);    
    ret= cryDukptGetPin("AMOUNT= 1000 USD","CODE:",acc,60,SNKey,blk); //enter 1234 to obtain a known result
    CHECK(ret>=0,lblKO);

    ret= dspStart(); //now we can open HMI again
    CHECK(ret>=0,lblKO);

    //present results
    ret= bin2hex(tmp,SNKey,10); CHECK(ret==20,lblKO);  //SNKey
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
    memset(tmp,0,prtW+1);
    ret= bin2hex(tmp,blk,8); CHECK(ret==16,lblKO);     //Pinblock
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
    ret= dspLS(0,tmp); CHECK(ret>=0,lblKO);
    tmrPause(1);

    CHECK(memcmp(blk,"\xB7\x69\x97\xF8\x3C\x14\x79\xDB",8)==0,lblKO); //it is the pinblock if the pin is 1234


    ret= dspStop(); //close channel to give cryptomodule access to HMI
    CHECK(ret>=0,lblKO);

    //*** Second Pin entry and get pinblock ***
    ret= cryDukptGetPin("AMOUNT= 1000 USD","CODE:",acc,60,SNKey,blk); //enter 1234 to obtain a known result
    CHECK(ret>=0,lblKO);

    ret= dspStart(); //now we can open HMI again
    CHECK(ret>=0,lblKO);

    //present results
    ret= bin2hex(tmp,SNKey,10); CHECK(ret==20,lblKO); //SNKey
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
    memset(tmp,0,prtW+1);
    ret= bin2hex(tmp,blk,8); CHECK(ret==16,lblKO);    //Pinblock
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
    ret= dspLS(0,tmp); CHECK(ret>=0,lblKO);
    tmrPause(1);

    CHECK(memcmp(blk,"\x92\x5B\xC2\xA3\x96\x52\xCF\x75",8)==0,lblKO);  //it is the pinblock if the pin is 1234

    goto lblEnd;
        
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop('m'); prtStop(); dspStop(); //close resources
}
#else
void tcbk0001(void) {}
#endif


