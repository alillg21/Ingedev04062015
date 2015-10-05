// Functional test case rvr01 - USER REVERSAL
//
// author:      Pal Suranyi
// date:        16.08.2005
// description: After performing tests BTC01, PRC01 and MRT01 choose reversal 
//              menu item. Verify that the return code is good and the receipt 
//              corresponds to the format required.
//              Perform the batch subtotals transaction. Verify that the server 
//              debit total amount is not changed and it is the same as terminal 
//              debit total. Verify that the server and terminal credit total 
//              amounts are the zero.
//              Choose reversal once more. The transaction should be refused.
//              Print transaction log. There should be two transactions, 
//              the last one being cancelled.
//
#include <string.h>
#include "log.h"

#ifdef __TEST__

#include "tsterste.h"

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

void rvr01(void)
{
	int ret;

	// calling batch close before testing
	btc01();
	
	// calling purchase
	prc01();

	// calling merchandise return
	mrt01();

	// calling reversal
	ret= mapPutWord(traMnuItm,mnuReversal); CHK;
	pmtPayment();

	// calling subtotal batch
//	ret= mapPutWord(traMnuItm,mnuSubTotalBatch); CHK;
//	pmtPayment();

	// calling reversal once again (should be declined)
	ret= mapPutWord(traMnuItm,mnuReversal); CHK;
	pmtPayment();

	// print trxlog
	ret= mapPutWord(traMnuItm,mnuTranLog); CHK;
	rptReceipt();

	goto lblEnd;

lblKO:
	trcErr(ret);
	goto lblEnd;
lblEnd:
	return;
}

#endif // __TEST__
