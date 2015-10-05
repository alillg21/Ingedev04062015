/** \file
 * Unitary test case tcda0004. To display 2 line message and to get user response.
 * Functions testing:
 * \sa
 *  - ppdDspKbdKey()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"



void tcda0004(void) {
    int ret;
   	char lngInd;
    char *msg= "SELECT LANGUAGE\n1:Eng 2:Ara \n";
   
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    cryStart('m');
    
    ret= ppdDspKbdKey(msg,&lngInd);  
    CHECK(ret>0 ,lblKO);
    switch (lngInd) {
		  case '1':
		   	prtS("English");   
		   	break;
		  case '2':
		  	prtS("Arabic");   	   
		  	break;
		 default:
		   	prtS("Other");  
		   	break; 
		  } 
	
    goto lblEnd;
  lblKO:
    trcErr(ret);
    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    dspLS(0, "KO!");
    tmrPause(3);
  lblEnd:
   cryStop('m');
   prtStop();
   dspStop();                  //close resources
   return;
}
