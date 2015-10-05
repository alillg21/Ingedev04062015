// Functional test case dnl01 - DOWNLOAD
//
// author:      Pal Suranyi
// date:        16.08.2005
// description: Perform the download transaction. Verify the following:
//              - the return code is correct
//              - the working MAC and PIN keys are downloaded successfully
//                (verify the return code of cryLoadDKuD function)
//              - all the information received is printed properly
//                (except key values) No other information is printed.
//
#include "log.h"

#ifdef __TEST__

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

void dnl01(void)
{
	int ret;

	trcS("dnl01: Beg\n");

	ret= datLogCur(); CHK;
	ret= mapPutWord(traMnuItm,mnuMasterKey);CHK;
	ret= valOpr(); CHK; //validate operator   

	// Set hardwired masterkey 0123456789ABCDEF
	// the return code of cryLoadDKuD function is verified inside
	admMasterKey();

	traReset();
	ret= datLogCur(); CHK;
	ret= mapPutWord(traMnuItm,mnuDownload); CHK;
	ret= valOpr(); CHK; //validate operator

	// Perform the download transaction.
	admDownload();

	goto lblEnd;
lblKO:
	trcErr(ret);
	goto lblEnd;
lblEnd:
	return;
	trcS("dnl01: End\n");
}

#endif // __TEST__


