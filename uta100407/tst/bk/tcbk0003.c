/** \file
 * Unitary test case tcbk0003.
 * Functions testing:
 * \sa
 *  - cry3DESDukptInit()
 *  - cry3DESDukptPinInput()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __DUKPT__

void tcbk0003(void){
    int ret;
    byte crt[4];
    char tmp[prtW+1];
    byte acc[8];
    byte blk[8];
    byte loc;
    byte InitSNKey[10];     // InitSNKey = KET SET ID (FFFF) + TRMS ID (9876543210) + TRANSACTION COUNTER (E00000)
    byte SNKey[10];         // SNKey return at pin = KET SET ID + TRMS ID + TRANSACTION COUNTER

    memset(tmp,0,prtW+1);
    memset(crt,0,4);

    tcbk0002(); //test and working keys downloading
    
    //open ressources
    ret= prtStart(); CHECK(ret>=0,lblKO);
    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= cryStart('m'); CHECK(ret>=0,lblKO);

    //verify that the working key 6AC292FAA1315B4D 858AB3A3D7D5933A is downloaded at array 2 slot 4
    loc= 0x00; //0x24;
    ret= cryVerify(loc,crt);
    CHECK(ret>=0,lblKO);
    CHECK(memcmp(crt,"\xAF\x8C\x07\x4A",4)==0,lblKO);  //it is the certificate of the key 6AC292FAA1315B4D 858AB3A3D7D5933A

    //start testing

    //*** Initialize DUKPT process ***
    memcpy(InitSNKey, "\xFF\xFF\x98\x76\x54\x32\x10\xE0\x00\x00", 10);
    ret= cry3DESDukptInit(loc,InitSNKey);
    CHECK(ret>=0,lblKO);

    ret= dspStop(); //close channel to give cryptomodule access to HMI
    CHECK(ret>=0,lblKO);

    //*** First Pin entry and get pinblock ***        
    memcpy(acc,"\x00\x00\x40\x12\x34\x56\x78\x90",8);    
    ret= cry3DESDukptGetPin("AMOUNT= 1000 USD","CODE:",acc,60,SNKey,blk); //enter 1234 to obtain a known result
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

//    CHECK(memcmp(blk,"\x10\xA0\x1C\x8D\x02\xC6\x91\x07",8)==0,lblKO); //it is the pinblock if the pin is 1234

    ret= dspStop(); //close channel to give cryptomodule access to HMI
    CHECK(ret>=0,lblKO);

    //*** Second Pin entry and get pinblock ***
    ret= cry3DESDukptGetPin("AMOUNT= 1000 USD","CODE:",acc,60,SNKey,blk); //enter 1234 to obtain a known result
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

//    CHECK(memcmp(blk,"\x18\xDC\x07\xB9\x47\x97\xB4\x66",8)==0,lblKO);  //it is the pinblock if the pin is 1234

    goto lblEnd;
        
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop('m'); prtStop(); dspStop(); //close resources
}
#else
void tcbk0003(void) {}
#endif

