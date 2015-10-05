#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcar0007
//Functions testing :
//  ecrOnLineSale
//  ecrOnlineVoid
//  ecrFinishSale

void tcar0007(void) {
	int ret;
	char *fnc = "OnLineVoid";
	char *ok = " OK";
	char *ko = " KO!";
	
	dspStart();
	dspLS(0, fnc);

	ret = prtStart();
	CHECK(ret >= 0, lblKO);
	ret = ecrStart();
	CHECK(ret == 0, lblKO);
    
	ret = ecrOnLineSale(7, 10); // ecrPLUnumber, Quantity
	CHECK(ret == 0, lblKO);

	// Void the last sale (0)
	ret = ecrOnLineVoid(0);
	CHECK(ret == 0, lblKO);

	// simulate TOTAL key pressing
	ret = ecrFinishSale();
	CHECK(ret == 0, lblKO);
	
	dspLS(1, ok);
    goto lblEnd;
	 
  lblKO:
	prtS(fnc); prtS(ko);
	dspLS(1, ko);
	
  lblEnd:
  	ecrStop();
  	prtStop();
	dspStop();
}

