//
// Functional test case mrt01 - MERCHANDISE RETURN
//
// author:      Tamas DANI
// date:        16.08.2005
// description: Perform the merchandise return transaction with the amount 222. 
//              Verify that the return code is good and the receipt
//              corresponds to the format required.
//              Perform the batch subtotals transaction. Verify that the server 
//              debit total amount is not changed and it is the same as
//              terminal debit total. Verify that the server and terminal credit 
//              total amounts are the equal to 222.
//

#include "tsterste.h"
#include "log.h"

#ifdef __TEST__

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

static int prepareData(void){
	return 1;
}

void mrt01(void){
	int ret;
	prepareData();
	trcS("mrt01: Beg\n");

	// calling batch close before testing
	btc01();

	ret= mapPutWord(traMnuItm,mnuMerchandiseReturn); CHECK(ret>=0,lblDBA);
	pmtPayment();

	ret= mapPutWord(traMnuItm,mnuSubTotalBatch); CHECK(ret>=0,lblDBA);
	logSubTotalBatch();

	trcS("mrt01: End\n");
	goto lblEnd;

lblDBA: //data access error occured
	trcErr(ret);
	usrInfo(infDataAccessError);
	ret= 0;
	goto lblEnd;
//lblKO:
//	trcErr(ret);
//	dspLS(0,"KO!");
lblEnd:
	tmrPause(2);
}

#endif // __TEST__
