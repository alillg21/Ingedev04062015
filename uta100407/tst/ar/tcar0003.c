
#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

// Unitary test case tcar0003 for testing :
// ecrProgramTax

void tcar0003(void) {
	int ret;
	char *fnc = "ProgramTax";
	char *ok = " OK";
	char *ko = " KO!";

	dspStart();
	dspLS(0, fnc);

	ret = prtStart();
	CHECK(ret >= 0, lblKO);

	ret = ecrStart();
    CHECK(ret == 0, lblKO);

	// Tax levels are from 1 to 9 (and not from 0 to 8)
	// Tax value has 2 decimals in integer representation 18,6 -> 1860
	ret = ecrProgramTax(3, "Tst 18,6%", 1860);
    CHECK(ret == 0, lblKO);
	// Reports error 1 : in fact this function is activated
	// for fiscal services only

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

