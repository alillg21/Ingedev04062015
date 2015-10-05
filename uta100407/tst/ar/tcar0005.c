#include <string.h>
#include <stdio.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

// Unitary test case tcar0005
// Functions tested in a loop :
//  ecrOnLineSale
//  ecrFinishSale

void tcar0005(void) {
	int ret;
	word plu;
	char tmp[100];
	char *fnc = "OnLineSale";
	char *ok = " OK";
	char *ko = " KO!";
	
	dspStart();
	dspLS(0, fnc);

	ret = prtStart();
	CHECK(ret >= 0, lblKO);
	ret= ecrStart();
    CHECK(ret == 0, lblKO);

	// Starting with plu = 0 provokes a
	// NACK(8) : "Price is out of range"
	for(plu = 1; plu < 4/*10*/ ; plu++){
		ret = ecrOnLineSale(plu, 1);  // quantity = 1
		CHECK(ret == 0, lblKO);
	
		ret = ecrFinishSale();
		CHECK(ret == 0, lblKO);

		// Between the end of a sale and the beginning of a new sale,
		// there should be a 4s delay for the bill printing.
		// ecrPause(400);
		// but it works without ... ?!
	}
	
	dspLS(1, ok);
    goto lblEnd;
	 
  lblKO:
	sprintf(tmp, "OnLineSale NAK=%d", ecrAckVal());
	prtS(tmp);
	dspLS(1, ko);
	dspLS(0, tmp);
	
  lblEnd:
	dspStop();
  	ecrStop();
  	prtStop();
}
