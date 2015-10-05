#include <stdio.h>
#include <string.h>
#include "pri.h"
#include "tc.h"

// Unitary test case tcjd0002.
// Tested functions :
//  ecrOnLineSale
//  ecrFinishSale

void tcjd0002(void)
{
	int ret;
	// word plu;
	char *fnc = "OnLineSale";

	tcDspTitle(fnc, "");

	ret = ecrStart();
    CHECK(ret == 1, lblKO);
    
	ret = ecrOnLineSale(7, 700); // 7 = "064 elektronsk", quantity
	CHECK(ret == 1, lblKO);
	
	ret = ecrOnLineSale(55, 550); // 55 = "Star  013", quantity
	CHECK(ret == 1, lblKO);

	ret = ecrOnLineSale(57, 570); // 57 = "Vesti", quantity
	CHECK(ret == 1, lblKO);

	ret = ecrFinishSale();
	CHECK(ret == 1, lblKO);
		
	tcDspS(" OK");
	goto lblEnd;
	 
  lblKO:
	tcPrt(fnc); 
	tcDspFN("KO! (NACK = %d)", ecrGetAck());
	tcPrt(0);
	
  lblEnd:
  	ecrStop();
}
