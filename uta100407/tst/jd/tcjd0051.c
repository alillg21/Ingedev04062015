#include "sys.h"
#include "tc.h"

static code const char *srcFile = __FILE__;

//Unitary test case tcjd0051.
//Functions testing of GPRS connection:
//  comGetIMSI
//  comGetIMEI
#ifdef __CHN_GPRS__
#define __GPRS__
#endif

#ifdef __GPRS__
int tcjd0051(int pI1, int pI2, char *pS1, char *pS2)
{
    int ret;
    char buf[256];

    ret = comStart(chnGprs);
    CHECK(ret >= 0, lblKO);
    ret = tcDsp(" comStart OK");
    CHECK(ret >= 0, lblKO);

    ret = tcDsp(" comGetIMEI...");
    CHECK(ret >= 0, lblKO);

    ret = comGetIMEI(buf);
    prtS("IMEI :");
    prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = tcDspPrev(" comGetIMEI OK");
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

	tcDsp(" comGetIMSI...");
    ret = comGetIMSI(buf,"0000"); //second parameter is PIN of GSM SIM card
    prtS("IMSI :");
    prtS(buf);
    CHECK(ret >= 0, lblKO);

    tcDspPrev(" comGetIMSI OK");
    goto lblEnd;

  lblKO:
    trcErr(ret);
    tcDsp(" KO!");
  lblEnd:
    comStop();
    tcDsp(" comStop OK");
}
#endif

