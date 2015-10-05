#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

// Unitary test case tcar0006.
// Tested function :
//   ecrHandShake

void tcar0006(void) {
	int ret;
	char *fnc = "HandShake";
	char *ok = " OK";
	char *ko = " KO!";
	
	dspStart();
	dspLS(0, fnc);

	ret = prtStart();
	CHECK(ret >= 0, lblKO);
	
	ret = ecrStart();
	CHECK(ret == 0,lblKO);
    
	ret = ecrHandShake();
	CHECK(ret == 0, lblKO);

	ret = ecrOnLineSale(7, 100); // sell a recharge of 100 dinars
	CHECK(ret == 0, lblKO);

	// simulation of TOTAL key pressing
	ret = ecrFinishSale();
	CHECK(ret == 0, lblKO);
// -------------------
	
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
