// Functional test case rvr02 - TIMEOUT REVERSAL
//
// author:      Tamas DANI
// date:        16.08.2005
// description: Start normal purchase transaction. After sending request break 
//              the connection. An error message should be displayed.
//              Restore the connection during error message display. The terminal 
//              should start dialing automatically.
//              Verify that the reversal is finished properly and the receipt 
//              is printed.
//              Perform the batch subtotals transaction. Verify that the server 
//              debit total amount is not changed and it is the same as
//              terminal debit total.
//              Print transaction log. The last transaction is printed and marked 
//              as cancelled.
//

#include "tsterste.h"
#include "log.h"

#ifdef __TEST__

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

void rvr02(void)
{
	int ret;

	// calling batch close before testing
	btc01();

	ret= mapPutWord(traMnuItm,mnuNormalPurchase); CHK;
	pmtPayment();

//	ret= mapPutWord(traMnuItm,mnuSubTotalBatch); CHK;
//	pmtPayment();

	ret= mapPutWord(traMnuItm,mnuDuplicata); CHK;
	pmtPayment();

	goto lblEnd;

lblKO:
	trcErr(ret);
	dspLS(0,"KO!");
lblEnd:
	tmrPause(3);
}

#endif // __TEST__
