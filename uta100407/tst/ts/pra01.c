//
// Functional test case mrt01 - MERCHANDISE RETURN
//
// author:      Tamas DANI
// date:        18.08.2005
// description: ...
//

#include "tsterste.h"
#include "log.h"

#ifdef __TEST__

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

static int prepareData(void){
	return 1;
}

void pra01(void){
	int ret;

	// calling batch close before testing
	btc01();

	// prepairing data
	prepareData();

	ret= mapPutWord(traMnuItm,mnuPreAut);        CHK;
	pmtPayment();

	ret= mapPutWord(traMnuItm,mnuSubTotalBatch); CHK;
	logSubTotalBatch();

	ret= mapPutWord(traMnuItm,mnuPreAutCompl);   CHK;
	pmtPayment();

	ret= mapPutWord(traMnuItm,mnuSubTotalBatch); CHK;
	logSubTotalBatch();

	goto lblEnd;

lblKO:
	trcErr(ret);
	dspLS(0,"KO!");
lblEnd:
	tmrPause(3);
}

#endif // __TEST__
