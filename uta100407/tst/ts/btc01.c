
// Integration test case btc01
//
// author:      Tamas Dani
// date:        16.08.2005
// description: Perform the batch close transaction. In this way server totals 
//              and terminal totals are cleared. This operation should be done
//              at the beginning of each testing session.
//

#include "def.h"
#include "sys.h"
#include "str.h"
#include "log.h"

#ifdef __TEST__

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

void btc01(void){
	int  ret;
	word wCheck;
	word LogIdx;

	trcS("btc01: Beg\n");

	// getting the number of stored transactions
	ret= mapGetWord(regLogIdx,LogIdx); CHK;

	// close batch only if transaction log is not empty 
	if(LogIdx){
		ret= mapPutWord(traMnuItm,mnuCloseBatch); CHK;
		logCloseBatch();
		ret= mapGetWord(regLogIdx,LogIdx); CHK;
	}

	// verifying that transaction log is empty
	CHECK(LogIdx==0,lblKO);

	ret= mapGetWord(totNdb,wCheck); CHK; CHECK(wCheck==0,lbTotErr); // number of debits
	ret= mapGetWord(totTdb,wCheck); CHK; CHECK(wCheck==0,lbTotErr); // total of debits
	ret= mapGetWord(totNcr,wCheck); CHK; CHECK(wCheck==0,lbTotErr); // number of credits
	ret= mapGetWord(totTcr,wCheck); CHK; CHECK(wCheck==0,lbTotErr); // total of credits
	ret= mapGetWord(totNad,wCheck); CHK; CHECK(wCheck==0,lbTotErr); // number of adjustments
	ret= mapGetWord(totTad,wCheck); CHK; CHECK(wCheck==0,lbTotErr); // total of adjustments

	trcS("btc01: End\n");
	goto lblEnd;

lbTotErr:
	goto lblEnd;
lblKO:
	trcErr(LogIdx);
	dspLS(0,"KO!");
lblEnd:
	tmrPause(3);
}

#endif // __TEST__
