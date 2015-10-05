/** \file
 * Unitary test case tcbk0002.
 * Functions testing: Working key loaded for starting 3DES DUKPT process
 * \sa
 *  - cryLoadDKuDK()
 *  - cryVerify()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcbk0002(void){
    int ret;
    byte crt[4];
    char tmp[prtW+1];
    byte key[16];
    byte loc;
    //WK, working key is 6AC292FAA1315B4D 858AB3A3D7D5933A
    //TK, transport key is the test key 1111111111111111 1111111111111111
    //WC, working key encrypted by transport key is FCAC1A1B0B12913D C77CB65C1DD45C20 
    const byte *wrkh= (byte *)"\xFC\xAC\x1A\x1B\x0B\x12\x91\x3D";   //  =>  EDE
    const byte *wrkl= (byte *)"\xC7\x7C\xB6\x5C\x1D\xD4\x5C\x20";   //  =>  EDE

    
    memset(tmp,0,prtW+1);
    memset(crt,0,4);

    tcab0021(); //test key downloading
    
    //open ressources
    ret= prtStart(); CHECK(ret>=0,lblKO);
    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= cryStart('m'); CHECK(ret>=0,lblKO);

    //verify that the transport key 1111111111111111 is downloaded at array 1 slot 0
    ret= cryVerify(0x10,crt);
    CHECK(ret>=0,lblKO);
    CHECK(memcmp(crt,"\x82\xE1\x36\x65",4)==0,lblKO); //it is the certificate of the key 1111111111111111

    //start testing
    ret= dspLS(0,"cryLoadDKuDK..."); CHECK(ret>=0,lblKO);

    memcpy(key,wrkh,8);
    memcpy(key+8,wrkl,8); //DED using double key
    
    loc= 0x00; //0x24;
    ret= cryLoadDKuDK(loc,0x10,key,'d'); //download dukpt key (DUKPT_INITIAL_KEY) into loc using transport key located at 0x10
    CHECK(ret>=0,lblKO);
    
    ret= cryVerify(loc,crt); CHECK(ret>=0,lblKO);

    //present results
    ret= bin2hex(tmp,&loc,1); CHECK(ret==2,lblKO);    
    strcat(tmp,": "); VERIFY(strlen(tmp)==4);    
    ret= bin2hex(tmp+4,crt,4); CHECK(ret==8,lblKO);    
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
    ret= dspLS(1,tmp); CHECK(ret>=0,lblKO);
    CHECK(memcmp(crt,"\xAF\x8C\x07\x4A",4)==0,lblKO);  //it is the certificate of the key 
                                                       //6AC292FAA1315B4D 858AB3A3D7D5933A
    tmrPause(1);
    goto lblEnd;
        
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop('m'); prtStop(); dspStop(); //close resources
}
