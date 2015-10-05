#if 0
//is not compilable: include missing, etc
#include <string.h>
#include <unicapt.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcjl0007.
//Functions testing:
//  

static int testPrn(uint32 hPrn, const char *hdr, const char *nrm,
                   const char *bld) {
    int ret;

    ret = prnPrint(hPrn, (char *) hdr);
    if(ret != RET_OK)
        return -1;
    ret = prnPrint(hPrn, (char *) nrm);
    if(ret != RET_OK)
        return -1;
    ret = prnPrint(hPrn, (char *) bld);
    if(ret != RET_OK)
        return -1;
    return 1;
}

static int testDsp(uint32 hHmi, const ssaFontDescr_t * dsc, const char *hdr,
                   const char *nrm, const char *bld) {
    int ret;

    ret = hmiADClearLine(hHmi, 0);
    if(ret != RET_OK)
        return -1;
    ret = hmiADDisplayText(hHmi, 0, 0, (char *) hdr);
    if(ret != RET_OK)
        return -1;

    ret = hmiADClearLine(hHmi, 1);
    if(ret != RET_OK)
        return -1;
    ret = hmiADDisplayText(hHmi, 1, 0, (char *) nrm);
    if(ret != RET_OK)
        return -1;

    ret = hmiADClearLine(hHmi, 2);
    if(ret != RET_OK)
        return -1;
    ret = hmiADDisplayText(hHmi, 2, 0, (char *) bld);
    if(ret != RET_OK)
        return -1;
    return 1;
}

//non-uta
void tcjl0007(void) {
    int ret;                    //*< integer value for return code */
	uint32 hSsa, hPrn, hHmi;
	ssaFontDescr_t dsc;

    ret = ssaSecOpen(SSA_MERCHANT, &hSsa);
    if(ret != RET_OK)
        goto lblEnd;

    ret = hmiOpen("DEFAULT", &hHmi);
    if(ret != RET_OK)
        goto lblEnd;

    ret = prnOpen(PRN_DEFAULT_CHANNEL, &hPrn, PRN_IMMEDIATE);
    if(ret != RET_OK)
        goto lblEnd;

    ret = ssaSecLafWriteFont(hSsa, TERM, Iso8859_35, &dsc);
    if(ret != RET_OK)
        goto lblEnd;

    ret = testPrn(hPrn, "Hebrew test:",
                  "NORM \xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef",
                  "\x1B\x1A BOLD \xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef");
    if(ret < 0)
        goto lblEnd;

    ret = testDsp(hHmi, &dsc, "Hebrew test:",
                  "NORM \xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef",
                  "BOLD \xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef");
    if(ret < 0)
        goto lblEnd;
	tmrPause(5);
	
  lblKO:
    goto lblEnd;
  lblEnd:
    return;
}
#else
void tcjl0007(void) {}
#endif
