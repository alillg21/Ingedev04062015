#include <string.h>
#include "tst.h"
//#include "pri.h"

static code const char *srcFile= __FILE__;

//Unitary test case tcrf0004.
//Calculating LRC checksum

void tcrf0004(void){

	int ret;
    char msgstr[200];
	int msglngth;
	char msglrc;
	
    ret= prtStart(); //open the associated channel
    CHECK(ret>=0,lblKO);
	
	strcpy(msgstr,"\x02"); // 1
	strcat(msgstr,"9\x2E"); // 2
	strcat(msgstr,"0012345000              050323161310AO90000000\x1C"); // 47
	strcat(msgstr,"V001000000000000\x03"); // 17
	//strcat(msgstr,"j"); // last character is the LRC - the correct checksum is "j"
	
	msglngth= strlen(msgstr); // must be 1+2+47+17 = 67
	msglrc= stdLrc(&msgstr[1], msglngth-1); // from the string's second char
	ret= prtS(msgstr);
	
	sprintf(msgstr, "length: %d LRC: %c", msglngth, msglrc);
	ret= prtS(msgstr);
    CHECK(ret>=0,lblKO);
	
    goto lblEnd;
    
lblKO:
    trcErr(ret);
lblEnd:
    prtStop(); //close resource


}
