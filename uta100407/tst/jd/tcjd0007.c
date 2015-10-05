#include <string.h>
#include "pri.h"
#include "tc.h"

// Unitary test case tcjd0007.
// Tested function :
//   ecrHandShake

void tcjd0007(void) {
	int ret;
	char *fnc = "HandShake";
	
	tcDspTitle(fnc, "");
	
	ret = ecrStart();
	CHECK(ret == 1, lblKO);
    
	ret = ecrHandShake();
	CHECK(ret >= 0, lblKO);

	tcDspS(" OK");
	goto lblEnd;
	 
  lblKO:
	tcDspS(fnc); tcPrt(0);
	tcDspFN("KO! (NACK = %d)", ecrGetAck()); tcPrt(0);
	tcSetRet(ecrGetAck());
	
	// Example of error management to be used at application level
	switch (ecrGetAck())
	{
		case  1 :
			// ECR is not ready and responded NACK(1)
			// Ask the merchant to reset the ECR (press CLEAR + TOTAL)
			// or reload ECR with paper rolls if necessary
			break;

		case  3 :
			// ECR is busy because a manual sale is not closed yet
			// Ask the merchant to finish the ongoing sale
			break;

		case -2 :
			// Timeout : the terminal is not set on its cradle
			// or the ECR is not connected or switched on.
			// Ask the merchant to put the terminal on the cradle
			break;
	}
	
  lblEnd:
  	ecrStop();
}

