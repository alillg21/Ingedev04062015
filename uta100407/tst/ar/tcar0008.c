#include <string.h>
#include "pri.h"
#include "tst.h"

static code const char *srcFile = __FILE__;

// Unitary test case tcar0006.
// Tested function :
//   ecrHandShake

void tcar0008(void) {
	int ret;
	char *fnc = "FinishSale";
	char *ok = " OK";
	char *ko = " KO!";
	char tmp[20];

	dspStart();
	dspLS(0, fnc);

	ret = prtStart();
	CHECK(ret >= 0, lblKO);
	
	ret = ecrStart();
	CHECK(ret == 0,lblKO);
    
	// simulation of TOTAL key pressing
	ret = ecrFinishSale();
	CHECK(ret == 0, lblKO);
	
	dspLS(1, ok);
	goto lblEnd;
	 
  lblKO:
	dspLS(1, ko);
	// When run alone without any sale in progress on ECR
	// this test should return the error code 16
	// "Not started sale! Command not allowed!!"
	sprintf(tmp, "%s ACK = %d", fnc, ecrAckVal());
	prtS(tmp);

  lblEnd:
  	ecrStop();
  	prtStop();
	dspStop();
}
