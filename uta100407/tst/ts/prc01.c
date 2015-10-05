//
// Functional test case prc01 - NORMAL PURCHASE
//
// author:      Pal Suranyi
// date:        16.08.2005
//
// description: Perform the normal purchase transaction with the amount 111.
//              Verify the following:
//              - the return code is correct
//              - the receipt corresponds to the format required.
//              Perform the batch subtotals transaction. Verify that the server
//              debit total amount is 111 and it is the same as terminal debit total.
//              Credit and adjustment totals should be zero.
//
//.hystory:     getting terminal debit total (19.08.2005 TD)
//              new labels: lblDBA, lblAMT   (19.08.2005 TD)
//

#include <stdlib.h>
#include "log.h"

#ifdef __TEST__

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

void prc01(void)
{
	int ret;
	char tot[lenTot+1];

	trcS("prc01: Beg\n");

	traReset();
	ret= datLogCur(); CHK;
	ret= mapPutWord(traMnuItm,mnuNormalPurchase); CHK;
	ret= valOpr(); CHK; //validate operator

	// Perform payment.
	pmtPayment();

	traReset();
	ret= datLogCur(); CHK;
	ret= mapPutWord(traMnuItm,mnuSubTotalBatch); CHECK(ret>=0,lblDBA);
	ret= valOpr(); CHK; //validate operator

	// Perform batch subtotal transaction
	logSubTotalBatch();

	ret= mapGet(totTdb,tot,lenTot+1); CHECK(ret>=0,lblDBA);
	CHECK(atoi(tot)==111,lblAMT);

	goto lblEnd;

lblDBA: //data access error occured
	trcErr(ret);
	usrInfo(infDataAccessError);
	ret= 0;
	goto lblEnd;
lblAMT:
	dspClear();
	dspLS(0,"debit total ");
	dspLS(0,"check failed");
	tmrPause(2);
	dspClear();
	goto lblEnd;
lblKO:
	trcErr(ret);
	goto lblEnd;
lblEnd:
	return;
	trcS("prc01: End\n");
}

#endif // __TEST__
