/** \file
 * Unitary test case tcma0020.
 * Function testing
 * \sa
 *  - BaseToDec()
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

#ifdef __TEST__




void tcma0020(void){
    int ret;
	char number[10]="";
    trcS("tcma0020 Beg\n");
	
	ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);
    
	memcpy(number,"80",2);
	ret= BaseToDec(number, 36);
	CHECK(ret==288,lblKO);

	ret= BaseToDec(number, 16);
	CHECK(ret==128,lblKO);

	dspLS(1,"Done");
	tmrPause(3);
	goto lblEnd;
lblKO:
	prtS("-tcma0020");
	dspLS(1,"KO");
	tmrPause(3);
lblEnd:
	prtStop();
    dspStop();
    trcS("tcma0020 End\n");
}

#endif

