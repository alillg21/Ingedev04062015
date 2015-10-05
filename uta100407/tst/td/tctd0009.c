//
// unitary test case tctd0009
//
// author:      Tamas Dani
// date:        01.09.2005
// description: test for ...
//

#include <string.h>
#include "log.h"

#ifdef __TEST__

static code const char *srcFile= __FILE__;
#define CHK CHECK(ret>=0,lblKO)

static int prepareData(void)
{
	return 1;

//lblKO:
//	trcErr(ret); 
//	return -1;
}

void tctd0009(void)
{
	int ret;

	ret= dspStart(); CHK;
	ret= dspClear(); CHK;

	ret= prepareData(); CHK;
	tmrPause(1);

	goto lblEnd;

lblKO:
	trcErr(ret);
	dspLS(0,"KO!");
lblEnd:
	tmrPause(3);
	dspStop();
}
#endif
