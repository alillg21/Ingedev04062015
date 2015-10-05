#include <string.h>
#include "pri.h"
#include "tc.h"

// Unitary test case tcjd0009.
// Tested function :
//   ecrFinishSale

int tcjd0009(int pI1, int pI2, char *pS1, char *pS2) {
	int ret;
	char *fnc = "FinishSale";

	tcDspS(fnc);
	tcDspTitle(fnc, "");

	ret = ecrStart();
	CHECK(ret == 1, lblKO);
    
	// simulation of TOTAL key pressing
	ret = ecrFinishSale();
	CHECK(ret == 1, lblKO);
	
	tcDspS(" OK");
	goto lblEnd;
	 
  lblKO:
	// This test always return NACK error :
	// 16 : "Not started sale! Command not allowed!!"
	// or 3 : "A sale is in progress on ECR".
	// The only way to finish an ongoing sale is by a manual operation
	prtS(fnc);
	tcDspFN("KO! (ACK = %d)", ecrGetAck()); tcPrt((char *) 0);

  lblEnd:
  	ecrStop();
}
