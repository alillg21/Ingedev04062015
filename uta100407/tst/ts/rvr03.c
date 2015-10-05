// Functional test case rvr03 - MAC REVERSAL
//
// author:      Tamas DANI
// date:        16.08.2005
// description: To simulate MAC reversal transaction modify the MAC FID 
//              after receiving the response. It is done temporarily only
//              for this test case.
//              Start normal purchase transaction. An error message should be 
//              displayed. The terminal should send MAC reversal request 
//             (without breaking the connection?).
//             Verify that the reversal is finished properly and the receipt 
//             is printed.
//             Perform the batch subtotals transaction. Verify that the server 
//             debit total amount is not changed and it is the same as terminal 
//             debit total.
//             Print transaction log. The last transaction is printed and 
//             marked as cancelled.
//

#include <string.h>
#include "tsterste.h"
#include "log.h"

#ifdef __TEST__

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

void rvr03(void)
{
	int i;
	int ret;
	char ReqMac[lenReqMac+1];

	// calling batch close before testing
	btc01();

	// getting the MAC from the database
	ret= mapGet(spdhReqMac,ReqMac,lenReqMac+1); CHK;
	VERIFY(strlen(ReqMac)<=lenReqMac);

	// changing one byte of the MAC to 'X', starting at the end
	for(i=lenReqMac-1; i>=0; i--)
	{
		if(ReqMac[i]!='X')
		{
			ReqMac[i]='X';
			break;
		}
	}

	// putting the wrong MAC into the database
	ret= mapPutStr(spdhReqMac,ReqMac); CHECK(ret>=0,lblKO); 

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
