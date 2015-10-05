#include "ctx.h"
#ifdef __TEST__

#include "smfComm.h"
/*****
#include <string.h>
#include "log.h"
#include "tst.h"
*************/

//Integration test case tcjd0020.
//Application TOP5VRS
//Maintenance operation.

//#define CHK if(ret<0) return ret

void tcjd0020(void) {
	smfCommConfigScheduleMnt_t smf;
	int ret;

	ret = smfCommConfigConnection(SMF_SERIAL);
	CHECK(ret == 0, lblKO);

	// December, 24th 2008
	smf.date = (2008 << 16) | (12 << 8) | 24;
	// 23h40m0s
	smf.time = (23 << 16) | (40 << 8) | 0;

	smf.action = 1;
	
	tcDspInit(0);
	tcDspTitle("tcjd0020", "SMF test");

	ret = smfCommConfigScheduleMnt(&smf);
	CHECK(ret == 0, lblKO);
	
	ret = smfCommStart(SMF_INDIRECT);
	CHECK(ret == 0, lblKO);

	dspLS(0, "OK");
	goto lblEnd;

lblKO:
	dspLS(0, "KO!");
lblEnd:
	tmrPause(3);
}
#endif

