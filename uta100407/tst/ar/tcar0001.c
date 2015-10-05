
#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcar0001.
//Functions testing:
//  ecrOnLineSale
//  ecrFinishSale

void tcar0001(void) {
	int ret;
	char *fnc = "OnLineSale";
	char *ok = " OK";
	char *ko = " KO!";

	dspStart();
	dspLS(0, fnc);

	ret = prtStart();
	CHECK(ret >= 0, lblKO);  	
	ret = ecrStart();
    CHECK(ret == 0, lblKO);
    
	ret = ecrOnLineSale(7, 300); // ecrPLUnumber, Quantity
	CHECK(ret == 0, lblKO);
	
	tmrPause(1);

	ret = ecrFinishSale();
	CHECK(ret == 0, lblKO);
	
	dspLS(1, ok);
	goto lblEnd;
	 
  lblKO:
	dspLS(1, ko);
	prtS(fnc); prtS(ko);
	
  lblEnd:
  	ecrStop();
  	prtStop();
	dspStop();
}
