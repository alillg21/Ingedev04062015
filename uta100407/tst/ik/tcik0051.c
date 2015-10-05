#include <string.h>
#include "sys.h"
#include "tst.h"

//Unitary test case tcik0051.
//Functions testing of GPRS connection:
//  comGetIMSI
//  comGetIMEI
#ifdef __CHN_GPRS__
#define __GPRS__
#endif

#ifdef __GPRS__

void tcik0051(void) {
    int ret;
    byte idx, idxOK, idxErr;

    //char *ptr;
    char buf[256];

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    do {
        idx++;
        sprintf(buf, "iteration =%d", idx);
        prtS(buf);
        ret = comStart(chnGprs);
        CHECK(ret >= 0, lblKOIter);
        ret = dspLS(0, "comStart OK");
        CHECK(ret >= 0, lblKO);

        ret = dspLS(2, "comGetIMEI...");
        CHECK(ret >= 0, lblKO);

        ret = comGetIMEI(buf);
        prtS("IMEI :");
        prtS(buf);
        CHECK(ret >= 0, lblKOIter);

        ret = dspLS(2, "comGetIMEI OK");
        CHECK(ret >= 0, lblKO);
        //tmrPause(1);

        ret = dspLS(1, "comGetIMSI...");
        CHECK(ret >= 0, lblKO);

        ret = comGetIMSI(buf, "0000");  //second parameter is PIN of GSM SIM card
        prtS("IMSI :");
        prtS(buf);
        CHECK(ret >= 0, lblKOIter);

        ret = dspLS(1, "comGetIMSI OK");
        CHECK(ret >= 0, lblKO);
        idxOK++;
        goto lblEndIter;
      lblKOIter:
        idxErr++;
        comStop();
        dspLS(1, "comStop OK");
      lblEndIter:
        prtS("________________________");
    } while(idx < 20);
    prtS("UTA test: IMSI IMSI");
    sprintf(buf, "success  =%d", idxOK);
    prtS(buf);
    sprintf(buf, "Fail =%d", idxErr);
    prtS(buf);
    //tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
    dspLS(0, "KO!");
  lblEnd:
    prtStop();
    dspStop();
}
#endif
