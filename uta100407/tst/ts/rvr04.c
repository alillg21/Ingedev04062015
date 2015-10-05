// Functional test case rvr04 - USER STORNO (REVERSAL) 2
//
// author:      Pal Suranyi
// date:        30.08.2005
// description: This functional test case performa the following steps: 
//				BTC01, PRC01, batch subtotal, PRC01, batch subtotal, 
//				reversal, batch subtotal, batch close.
//              Verify that the return codes are good and the receipts 
//              corresponds to the format required.
//              Verify that each batch subtotals' balance is correct.
//
#include <string.h>
#include "log.h"

#ifdef __TEST__

#include "tsterste.h"

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

void rvr04(void)
{
	int ret;

	trcS("rvr04: Beg\n");

	// calling batch close before testing
	btc01();
	
	// calling purchase (it calls batch subtotal also)
	prc01();

	// calling purchase again (it calls batch subtotal also)
	prc01();

	// calling user reversal
	ret= mapPutWord(traMnuItm, mnuReversal); CHECK(ret>=0,lblDBA);
	pmtPayment();

	ret= mapPutWord(traMnuItm,mnuSubTotalBatch); CHECK(ret>=0,lblDBA);
	logSubTotalBatch();


	// calling reversal
	ret= mapPutWord(traMnuItm,mnuReversal); CHK;
	pmtPayment();

	// calling subtotal batch
	ret= mapPutWord(traMnuItm,mnuSubTotalBatch); CHK;
	pmtPayment();

	// calling reversal once again (should be declined)
	ret= mapPutWord(traMnuItm,mnuReversal); CHK;
	pmtPayment();

	// print trxlog
	ret= mapPutWord(traMnuItm,mnuTranLog); CHK;
	rptReceipt();

	trcS("mrt01: End\n");

	goto lblEnd;

lblDBA:
	trcErr(ret);
	goto lblEnd;
lblKO:
	trcErr(ret);
	goto lblEnd;
lblEnd:
	return;
}

#endif // __TEST__
