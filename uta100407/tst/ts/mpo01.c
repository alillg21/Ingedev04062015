//
// Functional test case mpo01 - MAIL/PHONE ORDER
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

void mpo01(void){
	int ret;
	prepareData();
	// calling batch close before testing
	btc01();

	ret= mapPutWord(traMnuItm,mnuMailPhoneOrder); CHK;
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
