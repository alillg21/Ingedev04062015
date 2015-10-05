/** \file
 * Unitary test case tcbk0006.
 * Functions testing:
 * \sa
 *  - cryOneWayFunctionExternal()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __OWF__

void tcbk0006(void){
    int ret;
    byte crt[4];
    char tmp[prtW+1];
    byte dataLeft[8], dataRight[8];
    byte resultLeft[8], resultRight[8];
    byte locIn;

    memset(tmp,0,prtW+1);
    memset(crt,0,4);

    tcbk0004(); //test and working key = register key downloading
    
    //open ressources
    ret= prtStart(); CHECK(ret>=0,lblKO);
    ret= dspStart(); CHECK(ret>=0,lblKO);    
    ret= cryStart('m'); CHECK(ret>=0,lblKO);

    //verify that the register key 0123456789ABCDEF is downloaded at array 2 slot 4
    locIn= 0x24;
    ret= cryVerify(locIn,crt);
    CHECK(ret>=0,lblKO);
    CHECK(memcmp(crt,"\x1E\x92\x65\xE5",4)==0,lblKO);  //it is the certificate of the key 2A3E4C6B7CCB2F6B

    //*** OWF External process ***
    memcpy(dataLeft,  "\x00\x00\x00\x27\x79\x32\x00\x50", 8); 
    memcpy(dataRight, "\x00\x00\x00\x27\x79\x32\x00\x50", 8);
    
    ret= cryOwfExt(dataLeft, dataRight, locIn, resultLeft, resultRight);   
    CHECK(ret>=0,lblKO);

    //present results
    ret= bin2hex(tmp,resultLeft,8); CHECK(ret==16,lblKO);    
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
	ret= dspLS(0, "OWF res left :");
	ret= dspLS(1,tmp); CHECK(ret>=0,lblKO);                   //result left 3B7C12A4615219A0
    ret= bin2hex(tmp,resultRight,8); CHECK(ret==16,lblKO);    
    ret= prtS(tmp); CHECK(ret>=0,lblKO);
	ret= dspLS(2, "OWF res right :");
    ret= dspLS(3,tmp); CHECK(ret>=0,lblKO);                   //result right 3B7C12A4615219A0

    goto lblEnd;
        
lblKO:
    trcErr(ret);
    dspLS(0,"KO!");
lblEnd:
    tmrPause(3);
    cryStop('m'); prtStop(); dspStop(); //close resources
}
#else
void tcbk0006(void) {}
#endif
