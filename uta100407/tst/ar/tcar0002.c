
#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcar0002.
//Functions testing:
//  ecrOnLineSale
//  ecrCloseOnLine

void tcar0002(void) {
	int ret;
	char *fnc = "CloseOnLine";
	char *ok = " OK";
	char *ko = " KO!";
	char *info = "Finish sale on ECR";
	
	dspStart();
	dspLS(0, fnc);

	prtStart();
	ret = ecrStart();
    CHECK(ret >= 0, lblKO);

	ret = ecrOnLineSale(9, 1);
	CHECK(ret == 0, lblKO);
 
	tmrPause(1);
	
	ret = ecrCloseOnLine();
	CHECK(ret == 0, lblKO);
	
	prtS(info);
	dspLS(1, ok);
	
    goto lblEnd;
	 
  lblKO:
	dspLS(1, ko);
	prtS(fnc); prtS(ko);
	
  lblEnd:
	dspLS(2, info);
  	ecrStop();
  	prtStop();
	dspStop();
}
