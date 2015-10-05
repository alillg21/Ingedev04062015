/** \file
 * Unitary test case tcbk0004.
 * Functions testing: Working key loaded for starting OWF process
 * \sa
 *  - cryLoadDKuDK()
 *  - cryVerify()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

void tcbk0004(void){
    int ret;
    byte crt[4];
    char tmp[prtW+1];
    byte key[16];
    byte loc;
    //WK, working key is 2A3E4C6B7CCB2F6B
    //TK, transport key is the test key 1111111111111111
    //WC, working key encrypted by transport key is 24FCF3F9429E8E6F  
    const byte *wrk= (byte *)"\x24\xFC\xF3\xF9\x42\x9E\x8E\x6F";   //  =>  EDE 

    memset(tmp,0,prtW+1);
    memset(crt,0,4);

    tcab0021(); //test key downloading
    
    //open ressources
    ret= prtStart(); CHECK(ret>=0,lblKO);
    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= cryStart('m'); CHECK(ret>=0,lblKO);

    //verify that the transport key 1111111111111111 is downloaded at array 1 slot 0
    ret= cryVerify(0x10,crt); CHECK(ret>=0,lblKO);
    CHECK(memcmp(crt,"\x82\xE1\x36\x65",4)==0,lblKO); //it is the certificate of the key 1111111111111111

    //start testing
    ret= dspLS(0,"cryLoadDKuDK..."); CHECK(ret>=0,lblKO);

    memcpy(key,wrk,8);   // First part : Key encrypted   
    memcpy(key+8,wrk,8); // Second part : Key encrypted
    
    loc= 0x24;
    ret= cryLoadDKuDK(loc,0x10,key,'g'); //download working key = register key (GEN_KEY) into loc using transport key located at 0x10
    CHECK(ret>=0,lblKO);
    
    ret= cryVerify(loc,crt); CHECK(ret>=0,lblKO);

    //present results
    ret= bin2hex(tmp,&loc,1); CHECK(ret==2,lblKO);    
    strcat(tmp,": "); VERIFY(strlen(tmp)==4);    
    ret= bin2hex(tmp+4,crt,4); CHECK(ret==8,lblKO);    
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
    ret= dspLS(1,tmp); CHECK(ret>=0,lblKO);
    CHECK(memcmp(crt,"\x1E\x92\x65\xE5",4)==0,lblKO);  //it is the certificate of the register key 2A3E4C6B7CCB2F6B

    goto lblEnd;
        
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop('m'); prtStop(); dspStop(); //close resources
}
