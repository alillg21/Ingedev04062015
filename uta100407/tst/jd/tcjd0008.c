#include <string.h>
#include "pri.h"
#include "tc.h"

//Unitary test case tcjd0008
//Functions testing :
//  ecrOnLineSale
//  ecrOnlineVoid
//  ecrFinishSale

void tcjd0008(void) {
	int ret;
	char *fnc = "OnLineVoid";
	
	tcDspTitle(fnc, "");

	ret = ecrStart();
	CHECK(ret == 1, lblKO);
    
	ret = ecrOnLineSale(7, 10); // ecrPLUnumber, Quantity
	CHECK(ret == 1, lblKO);

	// Void the last sale (0)
	ret = ecrOnLineVoid(0);
	CHECK(ret == 1, lblKO);

	// simulate TOTAL key pressing
	ret = ecrFinishSale();
	CHECK(ret == 1, lblKO);
	
	tcDspS(" OK");
    goto lblEnd;
	 
  lblKO:
	tcPrt(fnc);
	tcDspFN("KO! (NACK = %d)", ecrGetAck()); tcPrt((char *) 0);
	
  lblEnd:
  	ecrStop();
}

