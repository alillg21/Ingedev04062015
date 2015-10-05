#include <string.h>
#include <stdio.h>
#include "pri.h"
#include "tc.h"

// Unitary test case tcjd0006
// Functions tested in a loop :
//  ecrOnLineSale
//  ecrFinishSale

void tcjd0006(void) {
	int ret;
	word plu = 0;
	char *fnc = "OnLineSale loop";
	card oldDspMode = 0;
	
	tcDspTitle(fnc, "");

	ret = ecrStart();
    CHECK(ret == 1, lblKO);

	//try several dspStart, prtStart
			
	oldDspMode = tcDspModeSet(-TRC_2_HMI);
	// Starting with plu = 0 provokes a
	// NACK(8) : "Price is out of range"
	for (plu = 1 ; plu < 100 ; plu++)
	{
		ret = ecrOnLineSale(plu, 1);  // quantity = 1
		if (ret == -1 && ecrGetAck() == 105)
			// (maximum count of PLU/lines in a sale (51) has been reached)
			break;
		
		CHECK(ret == 1, lblKO);
	
		// ecrPause(400);
		// Between the end of a sale and the beginning of a new sale,
		// there should be a 4s delay for the bill printing.
		// but it works with no pause at all...
	}

	// Provokes the impression of a single ticket with totals
	ret = ecrFinishSale();
	CHECK(ret == 1, lblKO);
	tcDspFN("ecrOnLineSale with %d PLU done", plu);
    goto lblEnd;
	 
  lblKO:
	tcPrt(fnc);
	tcDspF2N("KO! (NACK = %d, PLUs = %d)", ecrGetAck(), plu - 1);
	tcPrt(0);
	
  lblEnd:
	tcDspModeSet(oldDspMode);
  	ecrStop();
}

