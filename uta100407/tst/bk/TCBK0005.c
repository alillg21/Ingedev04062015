/** \file
 * Unitary test case tcbk0005.
 * Functions testing:
 * \sa
 *  - cryOneWayFunctionInternal()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __OWF__

void tcbk0005(void){
    int ret;
    byte crt[4];
    char tmp[prtW+1];
    byte dataLeft[8];     
    byte dataRight[8];
    byte locIn, locOut;

    memset(tmp,0,prtW+1);
    memset(crt,0,4);

    tcbk0004(); //test and working key = register key downloading
    
    //open ressources
    ret= prtStart(); CHECK(ret>=0,lblKO);
    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= cryStart('m'); CHECK(ret>=0,lblKO);

    //verify that the register key 2A3E4C6B7C3B2F6B is downloaded at array 2 slot 4
    locIn= 0x24;
    ret= cryVerify(locIn,crt);
    CHECK(ret>=0,lblKO);
    CHECK(memcmp(crt,"\x1E\x92\x65\xE5",4)==0,lblKO);  //it is the certificate of the key 2A3E4C6B7CCB2F6B

    //*** OWF Internal process ***
	ret= dspLS(0,"cryOWFInternal.."); CHECK(ret>=0,lblKO);
    locOut= 0x26;
    memcpy(dataLeft,  "\x00\x00\x00\x27\x79\x32\x00\x50", 8); 
    memcpy(dataRight, "\x00\x00\x00\x27\x79\x32\x00\x50", 8);
    
    ret= cryOwfInt(dataLeft, dataRight, locIn, locOut, 'g');   
    CHECK(ret>=0,lblKO);

    //verify that the key result is downloaded at array 2 slot 6
    ret= cryVerify(locOut,crt); CHECK(ret>=0,lblKO);

    //present results
    ret= bin2hex(tmp,&locOut,1); CHECK(ret==2,lblKO);    
    strcat(tmp,": "); VERIFY(strlen(tmp)==4);    
    ret= bin2hex(tmp+4,crt,4); CHECK(ret==8,lblKO);    
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
    ret= dspLS(1,tmp); CHECK(ret>=0,lblKO);
    CHECK(memcmp(crt,"\x50\x15\x08\x6C",4)==0,lblKO);  //it is the certificate of the result key 3B7C12A4615219A0

    goto lblEnd;
        
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop('m'); prtStop(); dspStop(); //close resources
}
#else
void tcbk0005(void) {}
#endif
