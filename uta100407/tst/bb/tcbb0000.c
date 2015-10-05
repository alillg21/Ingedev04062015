/** \file
 * Unitary test case 
 * \sa
 *  - enterTxt2()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/bb/tcbb0000.c $
 *
 * $Id: tcbb0000.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

void tcbb0000(void){
    int ret;
	byte row, len, fmt;
    char txt[dspW*dspH+1]= "Budapest";
	char rw2[dspW*dspH+1]= "12345678901234561234567890123456";
	char rw3[dspW*dspH+1]= "123456789012345612345678901234561234567890123456";
	char rw4[dspW*dspH+1]= "1234567890123456123456789012345612345678901234561234567890123456";
	char rw0[dspW*dspH];

	ret= dspStart(); CHECK(ret>=0,lblKO);

	//The length of str is 8 characters, the maximum length is 12, clear text
	ret= dspClear(); CHECK(ret>=0,lblKO);
	row= 0; len= 12; fmt= 0xFF;
	ret= enterTxt2(row,txt,len,fmt); CHECK(ret>=0,lblKO);

	//Maximum 12 characters, for password input
	ret= dspClear(); CHECK(ret>=0,lblKO);
	row= 0; len= 12; fmt= '*';
	ret= enterTxt2(row,txt,len,fmt); CHECK(ret>=0,lblKO);

	//Two full rows in the last two lines of display
	ret= dspClear(); CHECK(ret>=0,lblKO);
	row= 2; len= 32; fmt= 0xFF;
	ret= enterTxt2(row,rw2,len,fmt); CHECK(ret>=0,lblKO);
	
	//Maximum 48 character on the top of display
	ret= dspClear(); CHECK(ret>=0,lblKO);
	row= 0; len= 48; fmt= 0xFF;
	ret= enterTxt2(row,rw2,len,fmt); CHECK(ret>=0,lblKO);

	//Three full rows in the last three lines of display
	ret= dspClear(); CHECK(ret>=0,lblKO);
	row= 1; len= 48; fmt= 0xFF;
	ret= enterTxt2(row,rw3,len,fmt); CHECK(ret>=0,lblKO);

	//Full display
	ret= dspClear(); CHECK(ret>=0,lblKO);
	row= 0; len= 64; fmt= 0xFF;
	ret= enterTxt2(row,rw4,len,fmt); CHECK(ret>=0,lblKO);

	//The str is an empty string, maximum length is 48 (full display)
	memset(rw0,0,1);	
	ret= dspClear(); CHECK(ret>=0,lblKO);
	row= 0; len= 64; fmt= 0xFF;
	ret= enterTxt2(row,rw0,len,fmt); CHECK(ret>=0,lblKO);

	//The str is an empty string, maximum length is 4, for password input
	memset(rw0,0,1);	
	ret= dspClear(); CHECK(ret>=0,lblKO);
	row= 3; len= 4;  fmt= '*';
	ret= enterTxt2(row,rw0,len,fmt); CHECK(ret>=0,lblKO);

	goto lblEnd;
    
lblKO:
    trcErr(ret);
    dspLS(0,"KO");
lblEnd:
	tmrPause(3);
	dspStop();
}
