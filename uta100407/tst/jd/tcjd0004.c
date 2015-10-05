#include <stdio.h>
#include <string.h>
#include "pri.h"
#include "tc.h"

// Unitary test case tcjd0004 for testing :
// ecrProgramTax


void tcjd0004(void) {
	int ret;
	char *fnc = "ProgramTax";

	tcDspTitle(fnc, "");

	ret = ecrStart();
    CHECK(ret == 1, lblKO);

	// Tax levels are from 1 to 9 (and not from 0 to 8)
	// Tax value has 2 decimals in integer representation 18,6 -> 1860
	ret = ecrProgramTax(3, "Tst 18,6%", 1860);
    CHECK(ret == 1, lblKO);
	// Reports error 1 : in fact this function is activated
	// for fiscal services only

	tcDspS(" OK");
    goto lblEnd;
	 
  lblKO:
	prtS(fnc);
	tcDspFN("KO! (NACK = %d)", ecrGetAck());
	tcDspS("(setting TAX levels is prohibited in Serbia)");
	tcPrt(0);
	
  lblEnd:
  	ecrStop();
}

